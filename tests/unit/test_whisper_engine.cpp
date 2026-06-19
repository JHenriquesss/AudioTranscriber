#include "FakeWhisperEngine.hpp"
#include "AudioTestHelpers.hpp"

#include "whisper_engine/WhisperEngine.hpp"
#include "whisper_engine/WhisperWavReader.hpp"

#include <catch2/catch_test_macros.hpp>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <vector>

namespace {

std::filesystem::path createWorkspace(const std::string &name) {
    const auto workspace = std::filesystem::temp_directory_path() / name;
    std::filesystem::remove_all(workspace);
    std::filesystem::create_directories(workspace);
    return workspace;
}

void writeFakeModel(const std::filesystem::path &path) {
    std::ofstream stream(path, std::ios::binary);
    stream << "not-a-real-whisper-model";
}

} // namespace

TEST_CASE("WhisperEngine rejects invalid model file", "[whisper][engine]") {
    const auto workspace = createWorkspace("phase08-whisper-invalid-model");
    const auto modelPath = workspace / "ggml-small.bin";
    writeFakeModel(modelPath);

    whisper_engine::WhisperEngine engine;
    shared::CancellationToken token;
    const auto loaded = engine.loadModel(modelPath, token);

    REQUIRE_FALSE(loaded.ok);
    REQUIRE(loaded.error.code == shared::ErrorCode::ModelLoadFailed);

    std::filesystem::remove_all(workspace);
}

TEST_CASE("WhisperWavReader loads 16 kHz mono PCM WAV", "[whisper][wav]") {
    const auto workspace = createWorkspace("phase08-whisper-wav-reader");
    const auto wavPath = workspace / "normalized.wav";
    audio_test::writePcmMonoWav(wavPath, 16000, std::vector<std::int16_t>{0, 16384, -16384, 0});

    const auto samples = whisper_engine::readMonoPcm16Wav(wavPath);
    REQUIRE(samples.ok);
    REQUIRE(samples.value.size() == 4);
    REQUIRE(samples.value[1] > 0.4F);

    std::filesystem::remove_all(workspace);
}

TEST_CASE("WhisperEngine integration transcribes real model when configured",
          "[whisper][integration][integration]") {
    const char *modelEnv = std::getenv("OFFLINE_TRANSCRIBER_TEST_MODEL");
    if (modelEnv == nullptr || modelEnv[0] == '\0') {
        SKIP("Set OFFLINE_TRANSCRIBER_TEST_MODEL to run the real whisper integration test.");
    }

    const std::filesystem::path modelPath = modelEnv;
    if (!std::filesystem::is_regular_file(modelPath)) {
        SKIP("OFFLINE_TRANSCRIBER_TEST_MODEL does not point to a readable model file.");
    }

    const auto workspace = createWorkspace("phase08-whisper-integration");
    const auto wavPath = workspace / "normalized.wav";
    audio_test::writePcmMonoWav(wavPath, 16000, std::vector<std::int16_t>(32000, 0));

    whisper_engine::WhisperEngine engine;
    shared::CancellationToken token;
    REQUIRE(engine.loadModel(modelPath, token).ok);

    whisper_engine::WhisperRuntimeOptions options;
    options.languageCode = "en";
    options.enableWordTimestamps = false;

    const auto transcribed = engine.transcribe(wavPath, options, token);
    REQUIRE(transcribed.ok);
    REQUIRE_FALSE(transcribed.value.empty());
    REQUIRE_FALSE(transcribed.value.front().text.empty());
    REQUIRE(transcribed.value.front().text != "stub transcription");

    std::filesystem::remove_all(workspace);
}

TEST_CASE("FakeWhisperEngine remains injectable for deterministic pipeline tests",
          "[whisper][engine]") {
    auto fakeEngine = std::make_unique<test_support::FakeWhisperEngine>();
    whisper_engine::WhisperEngine engine(std::move(fakeEngine));

    const auto workspace = createWorkspace("phase08-whisper-fake-engine");
    const auto modelPath = workspace / "ggml-small.bin";
    writeFakeModel(modelPath);
    const auto wavPath = workspace / "normalized.wav";
    audio_test::writePcmMonoWav(wavPath, 16000, std::vector<std::int16_t>(1600, 0));

    shared::CancellationToken token;
    REQUIRE(engine.loadModel(modelPath, token).ok);

    whisper_engine::WhisperRuntimeOptions options;
    options.languageCode = "en";
    const auto transcribed = engine.transcribe(wavPath, options, token);
    REQUIRE(transcribed.ok);
    REQUIRE(transcribed.value.front().text == "hello  world");

    std::filesystem::remove_all(workspace);
}
