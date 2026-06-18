#include "TranscriptEditor.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

TranscriptEditor::TranscriptEditor(QWidget *parent) : QWidget(parent) {
    auto *title = new QLabel(QStringLiteral("Transcript"), this);
    editor_ = new QPlainTextEdit(this);
    editor_->setReadOnly(true);
    editor_->setPlaceholderText(QStringLiteral("Transcript text will appear here after a job completes."));

    exportButton_ = new QPushButton(QStringLiteral("Export"), this);
    exportButton_->setEnabled(false);

    auto *actions = new QHBoxLayout();
    actions->addStretch();
    actions->addWidget(exportButton_);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(title);
    layout->addWidget(editor_);
    layout->addLayout(actions);

    connect(exportButton_, &QPushButton::clicked, this, &TranscriptEditor::exportRequested);
}

void TranscriptEditor::setTranscriptText(const QString &text) {
    editor_->setPlainText(text);
}

void TranscriptEditor::clearTranscript() {
    editor_->clear();
}

void TranscriptEditor::setExportEnabled(bool enabled) {
    exportButton_->setEnabled(enabled);
}
