#pragma once

#include <QPlainTextEdit>
#include <QPushButton>
#include <QWidget>

class TranscriptEditor : public QWidget {
    Q_OBJECT

  public:
    explicit TranscriptEditor(QWidget *parent = nullptr);

    void setTranscriptText(const QString &text);
    void clearTranscript();
    void setExportEnabled(bool enabled);

  signals:
    void exportRequested();

  private:
    QPlainTextEdit *editor_ = nullptr;
    QPushButton *exportButton_ = nullptr;
};
