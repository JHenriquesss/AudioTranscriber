#pragma once

#include "app/ModelCatalog.hpp"
#include "app/TranscriptionJob.hpp"

#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QWidget>

#include <vector>

class TranscriptionPanel : public QWidget {
    Q_OBJECT

  public:
    explicit TranscriptionPanel(QWidget *parent = nullptr);

    [[nodiscard]] app::TranscriptionJobRequest buildRequest() const;
    void setBusy(bool busy);
    void showValidationError(const QString &message);
    void updateProgress(const app::TranscriptionJobProgress &progress);
    void showPipelineError(const QString &message, const QString &errorCode);
    void clearStatus();

  signals:
    void startRequested();
    void cancelRequested();
    void browseInputFileRequested();
    void browseOutputDirectoryRequested();

  public slots:
    void setInputFile(const QString &path);
    void setOutputDirectory(const QString &path);
    void setAvailableModels(const std::vector<app::ModelDisplayEntry> &models);

  private:
    void wireSignals();

    QLineEdit *inputFileEdit_ = nullptr;
    QLineEdit *outputDirectoryEdit_ = nullptr;
    QComboBox *languageCombo_ = nullptr;
    QComboBox *modelCombo_ = nullptr;
    QPushButton *startButton_ = nullptr;
    QPushButton *cancelButton_ = nullptr;
    QProgressBar *progressBar_ = nullptr;
    QLabel *statusLabel_ = nullptr;
    QLabel *errorLabel_ = nullptr;
};
