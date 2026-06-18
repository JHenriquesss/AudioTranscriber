#pragma once

#include "app/TranscriptionJob.hpp"

#include <QMainWindow>

class DesktopJobController;
class TranscriptEditor;
class TranscriptionPanel;

class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    explicit MainWindow(QWidget *parent = nullptr);

  private slots:
    void onBrowseInputFile();
    void onBrowseOutputDirectory();
    void onStartRequested();
    void onCancelRequested();
    void onExportRequested();
    void onValidationFailed(const QString &message, const QString &errorCode);
    void onProgressUpdated(const app::TranscriptionJobProgress &progress);
    void onJobCompleted(const QString &jobId, const QString &transcriptText);
    void onJobFailed(const QString &jobId, const QString &message, const QString &errorCode);
    void onExportSucceeded(const QString &message);
    void onExportFailed(const QString &message, const QString &errorCode);

  private:
    void wireSignals();
    void applyDefaultPaths();

    TranscriptionPanel *transcriptionPanel_ = nullptr;
    TranscriptEditor *transcriptEditor_ = nullptr;
    DesktopJobController *jobController_ = nullptr;
};
