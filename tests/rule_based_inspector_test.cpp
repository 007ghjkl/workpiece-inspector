#include <QTest>

#include "configuration/AppConfig.h"
#include "vision/RuleBasedInspector.h"
#include "vision/SimulatedImageSource.h"

using namespace workpiece;

namespace {

ImageFrame makeInspectionFrame(DefectType type, double score, bool hasDefect)
{
    ImageFrame frame;
    frame.image = QImage(64, 64, QImage::Format_RGB32);
    frame.image.fill(Qt::white);
    frame.metadata.scenarioId = 1;
    frame.metadata.productId = QStringLiteral("TEST-000001");
    frame.metadata.role = FrameRole::Inspection;
    frame.metadata.sourceType = QStringLiteral("simulated");
    frame.metadata.offset = PositionOffset{1.0, -2.0, 3.0};
    frame.metadata.defectType = type;
    frame.metadata.defectScore = score;
    frame.metadata.hasDefect = hasDefect;
    return frame;
}

ImageFrame makePositioningFrame()
{
    ImageFrame frame;
    frame.image = QImage(64, 64, QImage::Format_RGB32);
    frame.image.fill(Qt::white);
    frame.metadata.role = FrameRole::Positioning;
    frame.metadata.sourceType = QStringLiteral("simulated");
    frame.metadata.offset = PositionOffset{4.5, -6.25, 1.5};
    return frame;
}

} // namespace

class RuleBasedInspectorTest : public QObject
{
    Q_OBJECT

private slots:
    void positioningAnalysisReturnsOffsetAndConfidence()
    {
        const AppConfig config = defaultAppConfig();
        const RuleBasedInspector inspector;

        const PositioningAnalysis analysis = inspector.analyzePositioning(makePositioningFrame(), config);

        QVERIFY(analysis.valid);
        QCOMPARE(analysis.confidence, 1.0);
        QCOMPARE(analysis.offset.x, 4.5);
        QCOMPARE(analysis.offset.y, -6.25);
        QCOMPARE(analysis.offset.angle, 1.5);
        QVERIFY(!analysis.diagnosticSummary.isEmpty());
    }

    void wrongRoleAndEmptyFramesAreInvalid()
    {
        const AppConfig config = defaultAppConfig();
        const RuleBasedInspector inspector;

        ImageFrame empty;
        const PositioningAnalysis emptyPositioning = inspector.analyzePositioning(empty, config);
        QVERIFY(!emptyPositioning.valid);
        QCOMPARE(emptyPositioning.confidence, 0.0);

        const PositioningAnalysis wrongPositioning = inspector.analyzePositioning(makeInspectionFrame(DefectType::None, 0.0, false), config);
        QVERIFY(!wrongPositioning.valid);

        const InspectionAnalysis emptyInspection = inspector.inspect(empty, config);
        QVERIFY(!emptyInspection.valid);
        QVERIFY(emptyInspection.result.decision == InspectionDecision::Unknown);

        const InspectionAnalysis wrongInspection = inspector.inspect(makePositioningFrame(), config);
        QVERIFY(!wrongInspection.valid);
        QVERIFY(wrongInspection.result.decision == InspectionDecision::Unknown);
    }

    void normalSimulatedCasePasses()
    {
        const AppConfig config = defaultAppConfig();
        const RuleBasedInspector inspector;

        const InspectionAnalysis analysis = inspector.inspect(makeInspectionFrame(DefectType::None, 0.0, false), config);

        QVERIFY(analysis.valid);
        QVERIFY(analysis.result.decision == InspectionDecision::Pass);
        QVERIFY(analysis.result.defectType == DefectType::None);
        QCOMPARE(analysis.result.defectScore, 0.0);
        QCOMPARE(analysis.result.confidence, 1.0);
        QCOMPARE(QString::fromStdString(analysis.result.productId), QStringLiteral("TEST-000001"));
    }

    void defectTypesCanFail()
    {
        const AppConfig config = defaultAppConfig();
        const RuleBasedInspector inspector;

        const InspectionAnalysis scratch = inspector.inspect(makeInspectionFrame(DefectType::Scratch, 80.0, true), config);
        QVERIFY(scratch.valid);
        QVERIFY(scratch.result.decision == InspectionDecision::Fail);
        QVERIFY(scratch.result.defectType == DefectType::Scratch);

        const InspectionAnalysis spot = inspector.inspect(makeInspectionFrame(DefectType::Spot, 80.0, true), config);
        QVERIFY(spot.valid);
        QVERIFY(spot.result.decision == InspectionDecision::Fail);
        QVERIFY(spot.result.defectType == DefectType::Spot);

        const InspectionAnalysis deformation = inspector.inspect(makeInspectionFrame(DefectType::Deformation, 80.0, true), config);
        QVERIFY(deformation.valid);
        QVERIFY(deformation.result.decision == InspectionDecision::Fail);
        QVERIFY(deformation.result.defectType == DefectType::Deformation);
    }

    void thresholdControlsPassFailDecision()
    {
        AppConfig config = defaultAppConfig();
        config.inspection.defectScoreFailThreshold = 70.0;
        const RuleBasedInspector inspector;

        const InspectionAnalysis below = inspector.inspect(makeInspectionFrame(DefectType::Scratch, 69.9, true), config);
        QVERIFY(below.valid);
        QVERIFY(below.result.decision == InspectionDecision::Pass);

        const InspectionAnalysis equal = inspector.inspect(makeInspectionFrame(DefectType::Scratch, 70.0, true), config);
        QVERIFY(equal.valid);
        QVERIFY(equal.result.decision == InspectionDecision::Fail);

        const InspectionAnalysis above = inspector.inspect(makeInspectionFrame(DefectType::Scratch, 70.1, true), config);
        QVERIFY(above.valid);
        QVERIFY(above.result.decision == InspectionDecision::Fail);
    }

    void simulatedSourceInputsProduceDeterministicResults()
    {
        AppConfig config = defaultAppConfig();
        config.simulation.seed = 99;
        config.simulation.defectProbability = 1.0;
        const RuleBasedInspector inspector;

        SimulatedImageSource firstSource;
        firstSource.open(config);
        const ImageFrame firstInspection = firstSource.capture(FrameRole::Inspection);
        const InspectionAnalysis first = inspector.inspect(firstInspection, config);

        SimulatedImageSource secondSource;
        secondSource.open(config);
        const ImageFrame secondInspection = secondSource.capture(FrameRole::Inspection);
        const InspectionAnalysis second = inspector.inspect(secondInspection, config);

        QVERIFY(first.valid);
        QVERIFY(second.valid);
        QVERIFY(first.result.decision == second.result.decision);
        QVERIFY(first.result.defectType == second.result.defectType);
        QCOMPARE(first.result.defectScore, second.result.defectScore);
        QCOMPARE(QString::fromStdString(first.result.productId), QString::fromStdString(second.result.productId));
    }
};

QTEST_GUILESS_MAIN(RuleBasedInspectorTest)

#include "rule_based_inspector_test.moc"
