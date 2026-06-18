#include "MainWindow.hpp"
#include "DesktopJobController.hpp"
#include "widgets/TranscriptionPanel.hpp"
#include "widgets/TranscriptEditor.hpp"

#include <QApplication>
#include <QPushButton>
#include <QSignalSpy>
#include <QTest>

#include <filesystem>
#include <fstream>

namespace {

std::filesystem::path writeSmokeInputFile() {
    const auto path =
        std::filesystem::temp_directory_path() / "phase06-main-window-smoke-input.mp3";
    std::ofstream stream(path, std::ios::binary);
    stream << "fake media";
    return path;
}

} // namespace

class MainWindowSmokeTest : public QObject {
    Q_OBJECT

  private slots:
    void mainWindowShowsCoreControls();
    void startWithoutInputShowsValidationError();
    void fakeJobUpdatesProgressAndTranscript();
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
    DesktopJobController controller;
    QSignalSpy validationSpy(&controller, &DesktopJobController::validationFailed);

    app::TranscriptionJobRequest request;
    controller.startJob(request);

    QCOMPARE(validationSpy.count(), 1);
    const auto arguments = validationSpy.takeFirst();
    QVERIFY(arguments.at(0).toString().contains(QStringLiteral("Select an audio or video file")));
    QCOMPARE(arguments.at(1).toString(), QStringLiteral("FileNotFound"));
}

void MainWindowSmokeTest::fakeJobUpdatesProgressAndTranscript() {
    MainWindow window;
    auto *panel = window.findChild<TranscriptionPanel *>();
    auto *editor = window.findChild<TranscriptEditor *>();
    QVERIFY(panel != nullptr);
    QVERIFY(editor != nullptr);

    const auto input = writeSmokeInputFile();
    const auto output =
        std::filesystem::temp_directory_path() / "phase06-main-window-smoke-exports";
    std::filesystem::create_directories(output);

    panel->setInputFile(QString::fromStdString(input.string()));
    panel->setOutputDirectory(QString::fromStdString(output.string()));

    QPushButton *startButton = nullptr;
    for (QPushButton *button : panel->findChildren<QPushButton *>()) {
        if (button->text() == QStringLiteral("Start")) {
            startButton = button;
            break;
        }
    }
    QVERIFY(startButton != nullptr);
    QTest::mouseClick(startButton, Qt::LeftButton);

    QPushButton *exportButton = nullptr;
    for (QPushButton *button : editor->findChildren<QPushButton *>()) {
        if (button->text() == QStringLiteral("Export")) {
            exportButton = button;
            break;
        }
    }
    QVERIFY(exportButton != nullptr);
    QTRY_VERIFY_WITH_TIMEOUT(exportButton->isEnabled(), 5000);
}

QTEST_MAIN(MainWindowSmokeTest)

#include "test_main_window_smoke.moc"
