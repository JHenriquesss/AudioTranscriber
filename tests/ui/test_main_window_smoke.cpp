#include "MainWindow.hpp"
#include "DesktopJobController.hpp"
#include "QtPath.hpp"
#include "AudioTestHelpers.hpp"
#include "FakeWhisperEngine.hpp"
#include "app/JobManager.hpp"
#include "app/JobRequestPaths.hpp"
#include "widgets/TranscriptionPanel.hpp"
#include "widgets/TranscriptEditor.hpp"

#include <QApplication>
#include <QPushButton>
#include <QSignalSpy>
#include <QTest>

#include <filesystem>
#include <fstream>
#include <chrono>

namespace {

std::filesystem::path writeSmokeInputFile() {
    const auto path =
        std::filesystem::temp_directory_path() / "phase07-main-window-smoke-input.wav";
    audio_test::writePcmMonoWav(path, 16000, std::vector<std::int16_t>(1600, 0));
    return path;
}

void prepareRuntimeFiles(const std::filesystem::path &workspace) {
    std::filesystem::create_directories(workspace / "data");
    std::filesystem::create_directories(workspace / "models");
    std::filesystem::create_directories(workspace / "tools");

    std::ofstream model(workspace / "models" / "ggml-small.bin", std::ios::binary);
    model << "stub model";

    const auto ffmpeg = audio_test::resolveFfmpegExecutable();
    if (std::filesystem::exists(ffmpeg)) {
        std::filesystem::copy_file(ffmpeg, workspace / "tools" / "ffmpeg.exe",
                                   std::filesystem::copy_options::overwrite_existing);
    }
}

} // namespace

class MainWindowSmokeTest : public QObject {
    Q_OBJECT

  private slots:
    void mainWindowShowsCoreControls();
    void startWithoutInputShowsValidationError();
    void fakeJobUpdatesProgressAndTranscript();
    void pipelineAutoExportsAllFormatsWithUnicodeFilename();
    void mainWindowWorkflowCompletesAndExports();
};

void MainWindowSmokeTest::mainWindowShowsCoreControls() {
    MainWindow window;
    const auto panels = window.findChildren<TranscriptionPanel *>();
    const auto editors = window.findChildren<TranscriptEditor *>();
    QVERIFY(!panels.isEmpty());
    QVERIFY(!editors.isEmpty());

    const auto startButtons = window.findChildren<QPushButton *>();
    bool hasStart = false;
    bool hasCancel = false;
    for (QPushButton *button : startButtons) {
        if (button->text() == QStringLiteral("Start")) {
            hasStart = true;
        }
        if (button->text() == QStringLiteral("Cancel")) {
            hasCancel = true;
        }
    }
    QVERIFY(hasStart);
    QVERIFY(hasCancel);
}

void MainWindowSmokeTest::startWithoutInputShowsValidationError() {
    DesktopJobController controller(std::filesystem::temp_directory_path());
    QSignalSpy validationSpy(&controller, &DesktopJobController::validationFailed);

    app::TranscriptionJobRequest request;
    controller.startJob(request);

    QCOMPARE(validationSpy.count(), 1);
    const auto arguments = validationSpy.takeFirst();
    QVERIFY(arguments.at(0).toString().contains(QStringLiteral("Select an audio or video file")));
    QCOMPARE(arguments.at(1).toString(), QStringLiteral("FileNotFound"));
}

void MainWindowSmokeTest::fakeJobUpdatesProgressAndTranscript() {
    app::JobManager::setTestWhisperEngineSupplier([]() {
        return whisper_engine::WhisperEngine(std::make_unique<test_support::FakeWhisperEngine>());
    });

    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    const auto workspace = std::filesystem::temp_directory_path() /
                           ("phase07-main-window-smoke-workspace-" + std::to_string(stamp));
    prepareRuntimeFiles(workspace);

    const auto input = writeSmokeInputFile();
    const auto output = workspace / "exports";
    std::filesystem::create_directories(output);

    DesktopJobController controller(workspace);
    QSignalSpy completedSpy(&controller, &DesktopJobController::jobCompleted);
    QSignalSpy progressSpy(&controller, &DesktopJobController::progressUpdated);
    QSignalSpy exportSpy(&controller, &DesktopJobController::exportSucceeded);

    app::TranscriptionJobRequest request;
    request.inputFile = input;
    request.outputDirectory = "exports";
    request.language = "pt";
    request.modelId = "small";
    controller.startJob(request);

    QTRY_COMPARE_WITH_TIMEOUT(completedSpy.count(), 1, 20000);
    QTRY_COMPARE_WITH_TIMEOUT(exportSpy.count(), 1, 20000);
    QVERIFY(progressSpy.count() > 0);
    QVERIFY(!completedSpy.at(0).at(1).toString().isEmpty());
    QVERIFY(std::filesystem::exists(output / "phase07-main-window-smoke-input.txt"));
    QVERIFY(std::filesystem::exists(output / "phase07-main-window-smoke-input.srt"));

    app::JobManager::clearTestWhisperEngineSupplier();

    std::error_code cleanupError;
    std::filesystem::remove_all(workspace, cleanupError);
}

void MainWindowSmokeTest::pipelineAutoExportsAllFormatsWithUnicodeFilename() {
    app::JobManager::setTestWhisperEngineSupplier([]() {
        return whisper_engine::WhisperEngine(std::make_unique<test_support::FakeWhisperEngine>());
    });

    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    const auto workspace = std::filesystem::temp_directory_path() /
                           ("phase08-ui-export-workspace-" + std::to_string(stamp));
    prepareRuntimeFiles(workspace);
    std::filesystem::create_directories(workspace / "exports");

#if defined(_WIN32)
    const auto input = workspace / L"r_tr\u00e1s_da_ordena\u00e7\u00e3o_eficiente.wav";
#else
    const auto input = workspace / "r_trás_da_ordenação_eficiente.wav";
#endif
    audio_test::writePcmMonoWav(input, 16000, std::vector<std::int16_t>(1600, 0));

    DesktopJobController controller(workspace);
    QSignalSpy completedSpy(&controller, &DesktopJobController::jobCompleted);
    QSignalSpy exportSpy(&controller, &DesktopJobController::exportSucceeded);

    app::TranscriptionJobRequest request;
    request.inputFile = input;
    request.outputDirectory = "exports";
    request.language = "pt";
    request.modelId = "small";
    controller.startJob(request);

    QTRY_COMPARE_WITH_TIMEOUT(completedSpy.count(), 1, 20000);
    QTRY_COMPARE_WITH_TIMEOUT(exportSpy.count(), 1, 20000);

    const auto outputDirectory = std::filesystem::absolute(workspace / "exports");
    const auto baseName = app::exportBaseNameForInputFile(input);
    QVERIFY(std::filesystem::exists(outputDirectory / (baseName + ".txt")));
    QVERIFY(std::filesystem::exists(outputDirectory / (baseName + ".srt")));
    QVERIFY(std::filesystem::exists(outputDirectory / (baseName + ".vtt")));
    QVERIFY(std::filesystem::exists(outputDirectory / (baseName + ".json")));

    app::JobManager::clearTestWhisperEngineSupplier();

    std::error_code cleanupError;
    std::filesystem::remove_all(workspace, cleanupError);
}

void MainWindowSmokeTest::mainWindowWorkflowCompletesAndExports() {
    app::JobManager::setTestWhisperEngineSupplier([]() {
        return whisper_engine::WhisperEngine(std::make_unique<test_support::FakeWhisperEngine>());
    });

    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    const auto workspace = std::filesystem::temp_directory_path() /
                           ("phase08-ui-main-window-workflow-" + std::to_string(stamp));
    prepareRuntimeFiles(workspace);
    std::filesystem::create_directories(workspace / "data");
    std::filesystem::create_directories(workspace / "exports");

    const auto input = writeSmokeInputFile();
    qputenv("OFFLINE_TRANSCRIBER_APP_ROOT", QByteArray::fromStdString(workspace.string()));

    MainWindow window;
    const auto panels = window.findChildren<TranscriptionPanel *>();
    QVERIFY(!panels.isEmpty());
    TranscriptionPanel *panel = panels.first();

    panel->setInputFile(desktop::fromFilesystemPath(input));
    panel->setOutputDirectory(QStringLiteral("exports"));

    QSignalSpy completedSpy(window.findChild<DesktopJobController *>(),
                            &DesktopJobController::jobCompleted);
    QSignalSpy exportSpy(window.findChild<DesktopJobController *>(),
                         &DesktopJobController::exportSucceeded);

    const auto startButtons = window.findChildren<QPushButton *>();
    QPushButton *startButton = nullptr;
    for (QPushButton *button : startButtons) {
        if (button->text() == QStringLiteral("Start")) {
            startButton = button;
            break;
        }
    }
    QVERIFY(startButton != nullptr);
    QTest::mouseClick(startButton, Qt::LeftButton);

    QTRY_COMPARE_WITH_TIMEOUT(completedSpy.count(), 1, 20000);
    QTRY_COMPARE_WITH_TIMEOUT(exportSpy.count(), 1, 20000);

    const auto outputDirectory = std::filesystem::absolute(workspace / "exports");
    QVERIFY(std::filesystem::exists(outputDirectory / "phase07-main-window-smoke-input.txt"));

    app::JobManager::clearTestWhisperEngineSupplier();
    qunsetenv("OFFLINE_TRANSCRIBER_APP_ROOT");

    std::error_code cleanupError;
    std::filesystem::remove_all(workspace, cleanupError);
}

QTEST_MAIN(MainWindowSmokeTest)

#include "test_main_window_smoke.moc"
