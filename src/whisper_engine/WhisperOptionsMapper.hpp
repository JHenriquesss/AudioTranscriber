#pragma once

#include <string>

namespace whisper_engine {

enum class TranscriptionLanguageOption { Auto, Portuguese, English };

[[nodiscard]] TranscriptionLanguageOption parseLanguageOption(const std::string &language);

[[nodiscard]] std::string mapLanguageToWhisperCode(TranscriptionLanguageOption language);

[[nodiscard]] std::string mapLanguageToWhisperCode(const std::string &language);

} // namespace whisper_engine
