#pragma once

#include "communication/CommunicationEndpoint.h"
#include "motion/MotionController.h"
#include "persistence/SQLiteRepository.h"
#include "storage/ImageStorage.h"
#include "vision/RuleBasedInspector.h"

#include <QList>
#include <QString>

#include <functional>

namespace workpiece {

struct WorkflowSnapshot {
    StationState stationState = StationState::Idle;
    QString message;
    ImageFrame positioningFrame;
    ImageFrame inspectionFrame;
    PositioningAnalysis positioningAnalysis;
    InspectionAnalysis inspectionAnalysis;
    MotionState motionState;
    CommunicationState communicationState;
    QString imageRef;
};

struct WorkflowRunResult {
    bool success = false;
    StationState finalState = StationState::Idle;
    QString message;
    InspectionRecord inspectionRecord;
    QList<WorkflowSnapshot> updates;
};

class WorkflowController
{
public:
    using Observer = std::function<void(const WorkflowSnapshot &)>;

    WorkflowController(AppConfig config,
                       IImageSource &imageSource,
                       RuleBasedInspector &inspector,
                       IMotionController &motionController,
                       ICommunicationEndpoint &communicationEndpoint,
                       ImageStorage &imageStorage,
                       SQLiteRepository &repository);

    void addObserver(Observer observer);
    WorkflowRunResult runSingleCycle();
    StationState state() const;
    WorkflowSnapshot lastSnapshot() const;

private:
    WorkflowRunResult faultedResult(const QString &message, const WorkflowRunResult &current);
    void publish(StationState state, const QString &message);
    void persistFault(const QString &message, QString *workflowMessage);
    InspectionRecord makeInspectionRecord(const InspectionAnalysis &analysis,
                                          const ImageFrame &frame,
                                          const QString &imageRef,
                                          int cycleTimeMs) const;

    AppConfig config_;
    IImageSource &imageSource_;
    RuleBasedInspector &inspector_;
    IMotionController &motionController_;
    ICommunicationEndpoint &communicationEndpoint_;
    ImageStorage &imageStorage_;
    SQLiteRepository &repository_;
    QList<Observer> observers_;
    WorkflowSnapshot snapshot_;
};

} // namespace workpiece
