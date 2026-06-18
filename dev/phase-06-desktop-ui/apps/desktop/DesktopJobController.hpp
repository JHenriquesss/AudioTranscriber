#pragma once

#include "app/JobManager.hpp"
#include "app/TranscriptionJob.hpp"
#include "shared/CancellationToken.hpp"

#include <QObject>
#include <QString>

#include <memory>
#include <string>

class DesktopJobController : public QObject {
    Q_OBJECT

  public:
    explicit DesktopJobController(QObject *parent = nullptr);

    [[nodiscard]] bool isRunning() const;
    [[nodiscard]] QString currentJobId() const;

  public slots:
    void startJob(const app::TranscriptionJobRequest &request);
    void cancelCurrentJob();
    void exportCurrentJob();

  signals:
    void validationFailed(const QString &message, const QString &errorCode);
    void progressUpdated(const app::TranscriptionJobProgress &progress);
    void jobCompleted(const QString &jobId, const QString &transcriptText);
    void jobFailed(const QString &jobId, const QString &message, const QString &errorCode);
    void exportSucceeded(const QString &message);
    void exportFailed(const QString &message, const QString &errorCode);

  private slots:
    void onWorkerFinished(bool success);

  private:
    void resetWorkerState();
    [[nodiscard]] bool runActiveJob(shared::CancellationToken &token);

    app::JobManager jobManager_;
    std::unique_ptr<shared::CancellationToken> cancellationToken_;
    std::string activeJobId_;
    bool running_ = false;
};

Q_DECLARE_METATYPE(app::TranscriptionJobProgress)
