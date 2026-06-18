#include "TranscriptionPanel.hpp"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

TranscriptionPanel::TranscriptionPanel(QWidget *parent) : QWidget(parent) {
    auto *inputLayout = new QHBoxLayout();
    inputFileEdit_ = new QLineEdit(this);
    inputFileEdit_->setReadOnly(true);
    inputFileEdit_->setPlaceholderText(QStringLiteral("Select an audio or video file"));
    auto *browseInputButton = new QPushButton(QStringLiteral("Browse..."), this);
    inputLayout->addWidget(inputFileEdit_);
    inputLayout->addWidget(browseInputButton);

    auto *outputLayout = new QHBoxLayout();
    outputDirectoryEdit_ = new QLineEdit(this);
    outputDirectoryEdit_->setPlaceholderText(QStringLiteral("Select an output folder"));
    auto *browseOutputButton = new QPushButton(QStringLiteral("Browse..."), this);
    outputLayout->addWidget(outputDirectoryEdit_);
    outputLayout->addWidget(browseOutputButton);

    languageCombo_ = new QComboBox(this);
    languageCombo_->addItem(QStringLiteral("Auto"), QStringLiteral("auto"));
    languageCombo_->addItem(QStringLiteral("Portuguese"), QStringLiteral("pt"));
    languageCombo_->addItem(QStringLiteral("English"), QStringLiteral("en"));

    modelCombo_ = new QComboBox(this);
    modelCombo_->addItem(QStringLiteral("Tiny"), QStringLiteral("tiny"));
    modelCombo_->addItem(QStringLiteral("Base"), QStringLiteral("base"));
    modelCombo_->addItem(QStringLiteral("Small"), QStringLiteral("small"));
    modelCombo_->addItem(QStringLiteral("Medium"), QStringLiteral("medium"));

    startButton_ = new QPushButton(QStringLiteral("Start"), this);
    cancelButton_ = new QPushButton(QStringLiteral("Cancel"), this);
    cancelButton_->setEnabled(false);

    progressBar_ = new QProgressBar(this);
    progressBar_->setRange(0, 100);
    progressBar_->setValue(0);

    statusLabel_ = new QLabel(QStringLiteral("Ready."), this);
    errorLabel_ = new QLabel(this);
    errorLabel_->setStyleSheet(QStringLiteral("color: #b00020;"));
    errorLabel_->setWordWrap(true);
    errorLabel_->hide();

    auto *controlsLayout = new QHBoxLayout();
    controlsLayout->addWidget(startButton_);
    controlsLayout->addWidget(cancelButton_);
    controlsLayout->addStretch();

    auto *form = new QFormLayout();
    form->addRow(QStringLiteral("Input file"), inputLayout);
    form->addRow(QStringLiteral("Output folder"), outputLayout);
    form->addRow(QStringLiteral("Language"), languageCombo_);
    form->addRow(QStringLiteral("Model"), modelCombo_);

    auto *group = new QGroupBox(QStringLiteral("Transcription"), this);
    group->setLayout(form);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(group);
    layout->addLayout(controlsLayout);
    layout->addWidget(progressBar_);
    layout->addWidget(statusLabel_);
    layout->addWidget(errorLabel_);

    connect(browseInputButton, &QPushButton::clicked, this,
            &TranscriptionPanel::browseInputFileRequested);
    connect(browseOutputButton, &QPushButton::clicked, this,
            &TranscriptionPanel::browseOutputDirectoryRequested);
    wireSignals();
}

app::TranscriptionJobRequest TranscriptionPanel::buildRequest() const {
    app::TranscriptionJobRequest request;
    request.inputFile = inputFileEdit_->text().toStdString();
    request.outputDirectory = outputDirectoryEdit_->text().toStdString();
    request.language = languageCombo_->currentData().toString().toStdString();
    request.modelId = modelCombo_->currentData().toString().toStdString();
    return request;
}

void TranscriptionPanel::setBusy(bool busy) {
    startButton_->setEnabled(!busy);
    cancelButton_->setEnabled(busy);
}

void TranscriptionPanel::showValidationError(const QString &message) {
    errorLabel_->setText(message);
    errorLabel_->show();
    statusLabel_->setText(QStringLiteral("Cannot start transcription."));
}

void TranscriptionPanel::updateProgress(const app::TranscriptionJobProgress &progress) {
    errorLabel_->hide();
    progressBar_->setValue(static_cast<int>(progress.progress01 * 100.0));
    statusLabel_->setText(QString::fromStdString(app::to_string(progress.status)) + QStringLiteral(" — ") +
                          QString::fromStdString(progress.message));
}

void TranscriptionPanel::showPipelineError(const QString &message, const QString &errorCode) {
    errorLabel_->setText(message + QStringLiteral(" (") + errorCode + QStringLiteral(")"));
    errorLabel_->show();
    statusLabel_->setText(QStringLiteral("Transcription failed."));
    setBusy(false);
}

void TranscriptionPanel::clearStatus() {
    errorLabel_->hide();
    progressBar_->setValue(0);
    statusLabel_->setText(QStringLiteral("Ready."));
}

void TranscriptionPanel::setInputFile(const QString &path) {
    inputFileEdit_->setText(path);
}

void TranscriptionPanel::setOutputDirectory(const QString &path) {
    outputDirectoryEdit_->setText(path);
}

void TranscriptionPanel::wireSignals() {
    connect(startButton_, &QPushButton::clicked, this, &TranscriptionPanel::startRequested);
    connect(cancelButton_, &QPushButton::clicked, this, &TranscriptionPanel::cancelRequested);
}
