#include "MainWindow.hpp"

#include "DesktopJobController.hpp"
#include "app/AppPaths.hpp"
#include "app/AppSettings.hpp"
#include "widgets/TranscriptEditor.hpp"
#include "widgets/TranscriptionPanel.hpp"

#include <QFileDialog>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QSplitter>
#include <QStatusBar>
#include <QWidget>

#include <filesystem>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle(QStringLiteral("Offline Transcriber"));
    resize(1100, 700);

    transcriptionPanel_ = new TranscriptionPanel(this);
    transcriptEditor_ = new TranscriptEditor(this);
    jobController_ = new DesktopJobController(this);

    auto *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(transcriptionPanel_);
    splitter->addWidget(transcriptEditor_);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);

    auto *central = new QWidget(this);
    auto *layout = new QHBoxLayout(central);
    layout->addWidget(splitter);
    setCentralWidget(central);
    statusBar()->showMessage(QStringLiteral("Ready"));

    applyDefaultPaths();
    wireSignals();
}

void MainWindow::onBrowseInputFile() {
    const QString path = QFileDialog::getOpenFileName(
        this, QStringLiteral("Select audio or video file"), QString(),
        QStringLiteral("Media files (*.mp3 *.wav *.mp4 *.mkv *.m4a);;All files (*.*)"));
    if (!path.isEmpty()) {
        transcriptionPanel_->setInputFile(path);
    }
}

void MainWindow::onBrowseOutputDirectory() {
    const QString path = QFileDialog::getExistingDirectory(this, QStringLiteral("Select output folder"));
    if (!path.isEmpty()) {
        transcriptionPanel_->setOutputDirectory(path);
    }
}

void MainWindow::onStartRequested() {
    transcriptionPanel_->clearStatus();
    transcriptEditor_->clearTranscript();
    transcriptEditor_->setExportEnabled(false);
    transcriptionPanel_->setBusy(true);
    jobController_->startJob(transcriptionPanel_->buildRequest());
}

void MainWindow::onCancelRequested() {
    jobController_->cancelCurrentJob();
}

void MainWindow::onExportRequested() {
    jobController_->exportCurrentJob();
}

void MainWindow::onValidationFailed(const QString &message, const QString &errorCode) {
    transcriptionPanel_->setBusy(false);
    transcriptionPanel_->showValidationError(message + QStringLiteral(" (") + errorCode +
                                             QStringLiteral(")"));
    statusBar()->showMessage(QStringLiteral("Validation failed"), 5000);
}

void MainWindow::onProgressUpdated(const app::TranscriptionJobProgress &progress) {
    transcriptionPanel_->updateProgress(progress);
    statusBar()->showMessage(QString::fromStdString(progress.message));
}

void MainWindow::onJobCompleted(const QString &jobId, const QString &transcriptText) {
    Q_UNUSED(jobId);
    transcriptionPanel_->setBusy(false);
    transcriptEditor_->setTranscriptText(transcriptText);
    transcriptEditor_->setExportEnabled(true);
    statusBar()->showMessage(QStringLiteral("Transcription completed"), 5000);
}

void MainWindow::onJobFailed(const QString &jobId, const QString &message, const QString &errorCode) {
    Q_UNUSED(jobId);
    transcriptionPanel_->showPipelineError(message, errorCode);
    statusBar()->showMessage(QStringLiteral("Transcription failed"), 5000);
}

void MainWindow::onExportSucceeded(const QString &message) {
    statusBar()->showMessage(message, 5000);
}

void MainWindow::onExportFailed(const QString &message, const QString &errorCode) {
    QMessageBox::warning(this, QStringLiteral("Export failed"),
                         message + QStringLiteral(" (") + errorCode + QStringLiteral(")"));
}

void MainWindow::wireSignals() {
    connect(transcriptionPanel_, &TranscriptionPanel::browseInputFileRequested, this,
            &MainWindow::onBrowseInputFile);
    connect(transcriptionPanel_, &TranscriptionPanel::browseOutputDirectoryRequested, this,
            &MainWindow::onBrowseOutputDirectory);
    connect(transcriptionPanel_, &TranscriptionPanel::startRequested, this, &MainWindow::onStartRequested);
    connect(transcriptionPanel_, &TranscriptionPanel::cancelRequested, this, &MainWindow::onCancelRequested);
    connect(transcriptEditor_, &TranscriptEditor::exportRequested, this, &MainWindow::onExportRequested);

    connect(jobController_, &DesktopJobController::validationFailed, this, &MainWindow::onValidationFailed);
    connect(jobController_, &DesktopJobController::progressUpdated, this, &MainWindow::onProgressUpdated);
    connect(jobController_, &DesktopJobController::jobCompleted, this, &MainWindow::onJobCompleted);
    connect(jobController_, &DesktopJobController::jobFailed, this, &MainWindow::onJobFailed);
    connect(jobController_, &DesktopJobController::exportSucceeded, this, &MainWindow::onExportSucceeded);
    connect(jobController_, &DesktopJobController::exportFailed, this, &MainWindow::onExportFailed);
}

void MainWindow::applyDefaultPaths() {
    const auto paths = app::AppPaths::resolve(std::filesystem::current_path());
    app::AppSettingsStore settingsStore(paths);
    const auto settings = settingsStore.loadOrCreateDefaults();
    if (settings.ok) {
        transcriptionPanel_->setOutputDirectory(
            QString::fromStdString(settings.value.outputDirectory.string()));
    } else {
        transcriptionPanel_->setOutputDirectory(
            QString::fromStdString(paths.exportsDirectory().string()));
    }
}
