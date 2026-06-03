#include <QTest>

#include "core/DomainModels.h"

using namespace workpiece;

class DomainModelsTest : public QObject
{
    Q_OBJECT

private slots:
    void defaultValuesAreSafe()
    {
        const PositionOffset offset;
        QCOMPARE(offset.x, 0.0);
        QCOMPARE(offset.y, 0.0);
        QCOMPARE(offset.angle, 0.0);

        const InspectionResult inspection;
        QVERIFY(inspection.productId.empty());
        QVERIFY(inspection.decision == InspectionDecision::Unknown);
        QVERIFY(inspection.defectType == DefectType::Unknown);
        QCOMPARE(inspection.defectScore, 0.0);
        QCOMPARE(inspection.offset.x, 0.0);
        QCOMPARE(inspection.offset.y, 0.0);
        QCOMPARE(inspection.offset.angle, 0.0);
        QCOMPARE(inspection.confidence, 0.0);
        QVERIFY(inspection.diagnosticSummary.empty());

        const MotionState motion;
        QCOMPARE(motion.targetX, 0.0);
        QCOMPARE(motion.targetY, 0.0);
        QCOMPARE(motion.actualX, 0.0);
        QCOMPARE(motion.actualY, 0.0);
        QCOMPARE(motion.velocity, 0.0);
        QVERIFY(motion.status == MotionStatus::Idle);

        const CommunicationState communication;
        QVERIFY(communication.status == CommunicationStatus::Disconnected);
        QVERIFY(!communication.polling);
        QVERIFY(!communication.commandInProgress);
        QVERIFY(communication.diagnosticSummary.empty());

        const FaultInfo fault;
        QVERIFY(fault.source.empty());
        QVERIFY(fault.severity == FaultSeverity::Info);
        QVERIFY(fault.message.empty());
    }

    void enumStringsAreStable()
    {
        QCOMPARE(QString::fromStdString(toString(StationState::Idle)), QStringLiteral("idle"));
        QCOMPARE(QString::fromStdString(toString(StationState::Running)), QStringLiteral("running"));
        QCOMPARE(QString::fromStdString(toString(StationState::Paused)), QStringLiteral("paused"));
        QCOMPARE(QString::fromStdString(toString(StationState::Faulted)), QStringLiteral("faulted"));
        QCOMPARE(QString::fromStdString(toString(StationState::Completed)), QStringLiteral("completed"));

        QCOMPARE(QString::fromStdString(toString(ImageSourceState::Closed)), QStringLiteral("closed"));
        QCOMPARE(QString::fromStdString(toString(ImageSourceState::Opening)), QStringLiteral("opening"));
        QCOMPARE(QString::fromStdString(toString(ImageSourceState::Ready)), QStringLiteral("ready"));
        QCOMPARE(QString::fromStdString(toString(ImageSourceState::Capturing)), QStringLiteral("capturing"));
        QCOMPARE(QString::fromStdString(toString(ImageSourceState::Faulted)), QStringLiteral("faulted"));

        QCOMPARE(QString::fromStdString(toString(InspectionDecision::Unknown)), QStringLiteral("unknown"));
        QCOMPARE(QString::fromStdString(toString(InspectionDecision::Pass)), QStringLiteral("pass"));
        QCOMPARE(QString::fromStdString(toString(InspectionDecision::Fail)), QStringLiteral("fail"));

        QCOMPARE(QString::fromStdString(toString(DefectType::None)), QStringLiteral("none"));
        QCOMPARE(QString::fromStdString(toString(DefectType::Scratch)), QStringLiteral("scratch"));
        QCOMPARE(QString::fromStdString(toString(DefectType::Spot)), QStringLiteral("spot"));
        QCOMPARE(QString::fromStdString(toString(DefectType::Deformation)), QStringLiteral("deformation"));
        QCOMPARE(QString::fromStdString(toString(DefectType::Unknown)), QStringLiteral("unknown"));

        QCOMPARE(QString::fromStdString(toString(MotionStatus::Idle)), QStringLiteral("idle"));
        QCOMPARE(QString::fromStdString(toString(MotionStatus::Moving)), QStringLiteral("moving"));
        QCOMPARE(QString::fromStdString(toString(MotionStatus::Completed)), QStringLiteral("completed"));
        QCOMPARE(QString::fromStdString(toString(MotionStatus::Faulted)), QStringLiteral("faulted"));
        QCOMPARE(QString::fromStdString(toString(MotionStatus::Stopped)), QStringLiteral("stopped"));

        QCOMPARE(QString::fromStdString(toString(CommunicationStatus::Disconnected)), QStringLiteral("disconnected"));
        QCOMPARE(QString::fromStdString(toString(CommunicationStatus::Connected)), QStringLiteral("connected"));
        QCOMPARE(QString::fromStdString(toString(CommunicationStatus::Polling)), QStringLiteral("polling"));
        QCOMPARE(QString::fromStdString(toString(CommunicationStatus::CommandInProgress)), QStringLiteral("command_in_progress"));
        QCOMPARE(QString::fromStdString(toString(CommunicationStatus::Faulted)), QStringLiteral("faulted"));

        QCOMPARE(QString::fromStdString(toString(FaultSeverity::Info)), QStringLiteral("info"));
        QCOMPARE(QString::fromStdString(toString(FaultSeverity::Warning)), QStringLiteral("warning"));
        QCOMPARE(QString::fromStdString(toString(FaultSeverity::Error)), QStringLiteral("error"));
    }

    void unsupportedEnumValuesReturnUnknown()
    {
        QCOMPARE(QString::fromStdString(toString(static_cast<StationState>(999))), QStringLiteral("unknown"));
        QCOMPARE(QString::fromStdString(toString(static_cast<ImageSourceState>(999))), QStringLiteral("unknown"));
        QCOMPARE(QString::fromStdString(toString(static_cast<InspectionDecision>(999))), QStringLiteral("unknown"));
        QCOMPARE(QString::fromStdString(toString(static_cast<DefectType>(999))), QStringLiteral("unknown"));
        QCOMPARE(QString::fromStdString(toString(static_cast<MotionStatus>(999))), QStringLiteral("unknown"));
        QCOMPARE(QString::fromStdString(toString(static_cast<CommunicationStatus>(999))), QStringLiteral("unknown"));
        QCOMPARE(QString::fromStdString(toString(static_cast<FaultSeverity>(999))), QStringLiteral("unknown"));
    }
};

QTEST_APPLESS_MAIN(DomainModelsTest)

#include "domain_models_test.moc"
