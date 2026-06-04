#include "workflow/WorkflowController.h"

#include <QDateTime>
#include <QElapsedTimer>

#include <utility>

namespace workpiece {

WorkflowController::WorkflowController(AppConfig config,
                                       IImageSource &imageSource,
                                       RuleBasedInspector &inspector,
                                       IMotionController &motionController,
                                       ICommunicationEndpoint &communicationEndpoint,
                                       ImageStorage &imageStorage,
                                       SQLiteRepository &repository)
    : config_(std::move(config))
    , imageSource_(imageSource)
    , inspector_(inspector)
    , motionController_(motionController)
    , communicationEndpoint_(communicationEndpoint)
    , imageStorage_(imageStorage)
    , repository_(repository)
{
    snapshot_.stationState = StationState::Idle;
}

void WorkflowController::addObserver(Observer observer)
{
    observers_.push_back(std::move(observer));
}

WorkflowRunResult WorkflowController::runSingleCycle()
{
    WorkflowRunResult result;
    result.finalState = snapshot_.stationState;

    if (snapshot_.stationState == StationState::Running) {
        result.message = QStringLiteral("Workflow is already running.");
        result.finalState = snapshot_.stationState;
        result.updates = result.updates;
        return result;
    }

    QElapsedTimer timer;
    timer.start();

    publish(StationState::Running, QStringLiteral("Inspection cycle started."));
    result.updates.push_back(snapshot_);

    motionController_.configure(config_);
    communicationEndpoint_.configure(config_);
    imageSource_.open(config_);
    if (imageSource_.status().state != ImageSourceState::Ready) {
        return faultedResult(QStringLiteral("Image source is not ready."), result);
    }

    CommunicationResult communication = communicationEndpoint_.connectEndpoint();
    snapshot_.communicationState = communication.state;
    if (!communication.success) {
        return faultedResult(communication.message, result);
    }
    result.updates.push_back(snapshot_);

    communication = communicationEndpoint_.poll();
    snapshot_.communicationState = communication.state;
    if (!communication.success) {
        return faultedResult(communication.message, result);
    }
    result.updates.push_back(snapshot_);

    snapshot_.positioningFrame = imageSource_.capture(FrameRole::Positioning);
    if (snapshot_.positioningFrame.image.isNull()) {
        return faultedResult(QStringLiteral("Positioning image capture failed."), result);
    }
    publish(StationState::Running, QStringLiteral("Positioning frame captured."));
    result.updates.push_back(snapshot_);

    snapshot_.positioningAnalysis = inspector_.analyzePositioning(snapshot_.positioningFrame, config_);
    if (!snapshot_.positioningAnalysis.valid) {
        return faultedResult(snapshot_.positioningAnalysis.diagnosticSummary, result);
    }
    result.updates.push_back(snapshot_);

    const MotionCommandResult motion = motionController_.applyCorrection(snapshot_.positioningAnalysis.offset);
    snapshot_.motionState = motion.state;
    if (!motion.success) {
        return faultedResult(motion.message, result);
    }
    result.updates.push_back(snapshot_);

    snapshot_.inspectionFrame = imageSource_.capture(FrameRole::Inspection);
    if (snapshot_.inspectionFrame.image.isNull()) {
        return faultedResult(QStringLiteral("Inspection image capture failed."), result);
    }
    publish(StationState::Running, QStringLiteral("Inspection frame captured."));
    result.updates.push_back(snapshot_);

    snapshot_.inspectionAnalysis = inspector_.inspect(snapshot_.inspectionFrame, config_);
    if (!snapshot_.inspectionAnalysis.valid) {
        return faultedResult(snapshot_.inspectionAnalysis.diagnosticSummary, result);
    }
    result.updates.push_back(snapshot_);

    const ImageStorageResult imageStorage = imageStorage_.saveFrame(snapshot_.inspectionFrame, config_.persistence);
    if (!imageStorage.success) {
        return faultedResult(imageStorage.message, result);
    }
    snapshot_.imageRef = imageStorage.imageRef;
    result.updates.push_back(snapshot_);

    InspectionRecord record = makeInspectionRecord(snapshot_.inspectionAnalysis,
                                                   snapshot_.inspectionFrame,
                                                   imageStorage.imageRef,
                                                   static_cast<int>(timer.elapsed()));
    const PersistenceResult saved = repository_.saveInspectionRecord(record);
    if (!saved.success) {
        return faultedResult(saved.message, result);
    }

    result.success = true;
    result.inspectionRecord = record;
    publish(StationState::Completed, QStringLiteral("Inspection cycle completed."));
    result.finalState = StationState::Completed;
    result.message = QStringLiteral("Inspection cycle completed.");
    result.updates.push_back(snapshot_);
    return result;
}

StationState WorkflowController::state() const
{
    return snapshot_.stationState;
}

WorkflowSnapshot WorkflowController::lastSnapshot() const
{
    return snapshot_;
}

WorkflowRunResult WorkflowController::faultedResult(const QString &message, const WorkflowRunResult &current)
{
    WorkflowRunResult result = current;
    QString workflowMessage = message.trimmed().isEmpty()
        ? QStringLiteral("Workflow cycle failed.")
        : message;

    persistFault(message, &workflowMessage);
    publish(StationState::Faulted, workflowMessage);
    result.success = false;
    result.finalState = StationState::Faulted;
    result.message = workflowMessage;
    result.updates.push_back(snapshot_);
    return result;
}

void WorkflowController::publish(StationState state, const QString &message)
{
    snapshot_.stationState = state;
    snapshot_.message = message;
    for (const Observer &observer : observers_) {
        observer(snapshot_);
    }
}

void WorkflowController::persistFault(const QString &message, QString *workflowMessage)
{
    FaultLog fault;
    fault.timestamp = QDateTime::currentDateTimeUtc();
    fault.source = QStringLiteral("workflow");
    fault.severity = FaultSeverity::Error;
    fault.message = message.trimmed().isEmpty() ? QStringLiteral("Workflow cycle failed.") : message;

    const PersistenceResult saved = repository_.saveFaultLog(fault);
    if (!saved.success && workflowMessage != nullptr) {
        *workflowMessage += QStringLiteral(" Fault log was not saved: %1").arg(saved.message);
    }
}

InspectionRecord WorkflowController::makeInspectionRecord(const InspectionAnalysis &analysis,
                                                          const ImageFrame &frame,
                                                          const QString &imageRef,
                                                          int cycleTimeMs) const
{
    InspectionRecord record;
    record.timestamp = QDateTime::currentDateTimeUtc();
    record.productId = QString::fromStdString(analysis.result.productId);
    if (record.productId.trimmed().isEmpty()) {
        record.productId = frame.metadata.productId;
    }
    record.sourceType = frame.metadata.sourceType;
    record.imageRef = imageRef;
    record.result = analysis.result.decision;
    record.defectType = analysis.result.defectType;
    record.defectScore = analysis.result.defectScore;
    record.offset = analysis.result.offset;
    record.motionState = snapshot_.motionState;
    record.communicationState = snapshot_.communicationState;
    record.cycleTimeMs = cycleTimeMs;
    record.errorMessage.clear();
    return record;
}

} // namespace workpiece
