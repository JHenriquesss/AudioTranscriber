#include "DesktopJobController.hpp"

#include "QtPath.hpp"
#include "app/AppPaths.hpp"
#include "app/JobRequestPaths.hpp"

#include <QMetaObject>
#include <QPointer>
#include <QtConcurrent/QtConcurrentRun>

DesktopJobController::DesktopJobController(const std::filesystem::path &applicationRoot,
                                           QObject *parent)
    : QObject(parent) {
    qRegisterMetaType<app::TranscriptionJobProgress>("app::TranscriptionJobProgress");

    paths_ = app::AppPaths::resolve(applicationRoot);
    auto historyResult = app::initializeJobHistory(paths_);
    if (historyResult.ok) {
        history_ = std::move(historyResult.value);
        jobManager_.emplace(history_.get());
    } else {
        jobManager_.emplace();
    }
}

DesktopJobController::~DesktopJobController() {
    if (cancellationToken_ != nullptr) {
        cancellationToken_->cancel();
    }
    if (workerFuture_.isRunning()) {
        workerFuture_.waitForFinished();
    }
}

app::JobManager &DesktopJobController::jobManager() {
    return *jobManager_;
}

bool DesktopJobController::isRunning() const {
    return running_;
}

QString DesktopJobController::currentJobId() const {
    return QString::fromStdString(activeJobId_);
}

void DesktopJobController::startJob(const app::TranscriptionJobRequest &request) {
    if (running_) {
        emit validationFailed(QStringLiteral("A transcription job is already running."),
                              QStringLiteral("Unknown"));
        return;
    }

    const auto normalized = app::normalizeJobRequest(request, paths_);
    const auto outputReady = app::ensureDirectoryExists(normalized.outputDirectory);
    if (!outputReady.ok) {
        emit validationFailed(QString::fromStdString(outputReady.error.message),
                              QString::fromStdString(shared::to_string(outputReady.error.code)));
        return;
    }

    const auto submission = jobManager().submitJob(normalized);
    if (!submission.ok) {
        emit validationFailed(QString::fromStdString(submission.error.message),
                              QString::fromStdString(shared::to_string(submission.error.code)));
        return;
    }

    activeJobId_ = submission.value;
    cancellationToken_ = std::make_unique<shared::CancellationToken>();
    running_ = true;

    QPointer<DesktopJobController> controller(this);
    auto *token = cancellationToken_.get();
    const std::string jobId = activeJobId_;

    workerFuture_ = QtConcurrent::run([controller, token, jobId]() {
        if (controller.isNull()) {
            return;
        }
        const bool success = controller->runActiveJob(jobId, *token);
        QMetaObject::invokeMethod(controller, [controller, success]() {
            if (!controller.isNull()) {
                controller->onWorkerFinished(success);
            }
        }, Qt::QueuedConnection);
    });
}

bool DesktopJobController::runActiveJob(const std::string &jobId, shared::CancellationToken &token) {
    const auto result = jobManager().runJobPipeline(
        jobId, paths_, token,
        [this](const app::TranscriptionJobProgress &progress) {
            QMetaObject::invokeMethod(
                this, [this, progress]() { emit progressUpdated(progress); }, Qt::QueuedConnection);
        },
        nullptr);
    return result.ok;
}

void DesktopJobController::cancelCurrentJob() {
    if (!running_ || activeJobId_.empty()) {
        return;
    }

    if (cancellationToken_ != nullptr) {
        cancellationToken_->cancel();
    }

    const auto result = jobManager().cancelJob(activeJobId_);
    if (!result.ok) {
        emit jobFailed(QString::fromStdString(activeJobId_),
                       QString::fromStdString(result.error.message),
                       QString::fromStdString(shared::to_string(result.error.code)));
    }
}

void DesktopJobController::exportCurrentJob() {
    if (activeJobId_.empty()) {
        emit exportFailed(QStringLiteral("No completed job is available for export."),
                          QStringLiteral("ExportFailed"));
        return;
    }

    const auto result = jobManager().exportJobResults(activeJobId_);
    if (!result.ok) {
        emit exportFailed(QString::fromStdString(result.error.message),
                          QString::fromStdString(shared::to_string(result.error.code)));
        return;
    }

    emit exportSucceeded(QStringLiteral("Transcript exported successfully."));
}

void DesktopJobController::onWorkerFinished(bool success) {
    const QString jobId = QString::fromStdString(activeJobId_);
    const auto job = jobManager().getJob(activeJobId_);

    if (!success) {
        QString message = QStringLiteral("Transcription failed.");
        QString errorCode = QStringLiteral("Unknown");
        if (job.has_value() && job->error() != nullptr) {
            message = QString::fromStdString(job->error()->message);
            errorCode = QString::fromStdString(shared::to_string(job->error()->code));
        } else if (job.has_value() && job->status() == app::JobStatus::Cancelled) {
            message = QStringLiteral("The job was cancelled.");
            errorCode = QStringLiteral("Cancelled");
        }
        emit jobFailed(jobId, message, errorCode);
        resetWorkerState();
        return;
    }

    QString transcriptText;
    QString exportMessage;
    if (job.has_value()) {
        transcriptText = QString::fromStdString(job->transcriptPlainText());
        const auto baseName = app::exportBaseNameForInputFile(job->request().inputFile);
        exportMessage =
            QStringLiteral("Transcript exported to %1/%2 (.txt, .srt, .vtt, .json)")
                .arg(desktop::fromFilesystemPath(job->request().outputDirectory),
                     QString::fromUtf8(baseName.c_str()));
    }
    emit jobCompleted(jobId, transcriptText);
    if (!exportMessage.isEmpty()) {
        emit exportSucceeded(exportMessage);
    }
    resetWorkerState();
}

void DesktopJobController::resetWorkerState() {
    running_ = false;
    cancellationToken_.reset();
}
