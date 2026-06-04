#include <QCryptographicHash>
#include <QTest>

#include "configuration/AppConfig.h"
#include "vision/SimulatedImageSource.h"

using namespace workpiece;

namespace {

QByteArray imageDigest(const QImage &image)
{
    QByteArray bytes;
    const int width = image.width();
    const int height = image.height();
    const int bytesPerLine = image.bytesPerLine();
    bytes.append(reinterpret_cast<const char *>(&width), sizeof(int));
    bytes.append(reinterpret_cast<const char *>(&height), sizeof(int));
    bytes.append(reinterpret_cast<const char *>(&bytesPerLine), sizeof(int));
    bytes.append(reinterpret_cast<const char *>(image.constBits()), image.sizeInBytes());
    return QCryptographicHash::hash(bytes, QCryptographicHash::Sha256);
}

} // namespace

class SimulatedImageSourceTest : public QObject
{
    Q_OBJECT

private slots:
    void lifecycleTransitionsAreReported()
    {
        SimulatedImageSource source;
        QVERIFY(source.status().state == ImageSourceState::Closed);

        const ImageFrame unopened = source.capture(FrameRole::Positioning);
        QVERIFY(unopened.image.isNull());
        QVERIFY(source.status().state == ImageSourceState::Faulted);

        source.open(defaultAppConfig());
        QVERIFY(source.status().state == ImageSourceState::Ready);

        const ImageFrame frame = source.capture(FrameRole::Positioning);
        QVERIFY(!frame.image.isNull());
        QVERIFY(source.status().state == ImageSourceState::Ready);
        QCOMPARE(frame.image.width(), 640);
        QCOMPARE(frame.image.height(), 480);
        QCOMPARE(frame.image.format(), QImage::Format_RGB32);

        source.close();
        QVERIFY(source.status().state == ImageSourceState::Closed);
    }

    void sameSeedAndConfigProduceSameFrames()
    {
        AppConfig config = defaultAppConfig();
        config.simulation.seed = 777;
        config.simulation.defectProbability = 1.0;
        config.simulation.noiseLevel = 3.0;

        SimulatedImageSource first;
        first.open(config);
        const ImageFrame firstPositioning = first.capture(FrameRole::Positioning);
        const ImageFrame firstInspection = first.capture(FrameRole::Inspection);

        SimulatedImageSource second;
        second.open(config);
        const ImageFrame secondPositioning = second.capture(FrameRole::Positioning);
        const ImageFrame secondInspection = second.capture(FrameRole::Inspection);

        QCOMPARE(firstPositioning.metadata.scenarioId, secondPositioning.metadata.scenarioId);
        QCOMPARE(firstPositioning.metadata.productId, secondPositioning.metadata.productId);
        QCOMPARE(firstPositioning.metadata.hasDefect, secondPositioning.metadata.hasDefect);
        QVERIFY(firstPositioning.metadata.defectType == secondPositioning.metadata.defectType);
        QCOMPARE(firstPositioning.metadata.defectScore, secondPositioning.metadata.defectScore);
        QCOMPARE(firstInspection.metadata.scenarioId, secondInspection.metadata.scenarioId);
        QCOMPARE(firstInspection.metadata.productId, secondInspection.metadata.productId);
        QCOMPARE(imageDigest(firstPositioning.image), imageDigest(secondPositioning.image));
        QCOMPARE(imageDigest(firstInspection.image), imageDigest(secondInspection.image));
    }

    void positioningAndInspectionFramesShareScenario()
    {
        AppConfig config = defaultAppConfig();
        config.simulation.seed = 42;
        config.simulation.defectProbability = 1.0;

        SimulatedImageSource source;
        source.open(config);

        const ImageFrame positioning = source.capture(FrameRole::Positioning);
        const ImageFrame inspection = source.capture(FrameRole::Inspection);

        QCOMPARE(positioning.metadata.scenarioId, inspection.metadata.scenarioId);
        QCOMPARE(positioning.metadata.productId, inspection.metadata.productId);
        QCOMPARE(positioning.metadata.sourceType, QStringLiteral("simulated"));
        QCOMPARE(inspection.metadata.sourceType, QStringLiteral("simulated"));
        QVERIFY(positioning.metadata.role == FrameRole::Positioning);
        QVERIFY(inspection.metadata.role == FrameRole::Inspection);
        QCOMPARE(positioning.metadata.hasDefect, inspection.metadata.hasDefect);
    }

    void defectProbabilityControlsGeneratedCases()
    {
        AppConfig normalConfig = defaultAppConfig();
        normalConfig.simulation.seed = 12;
        normalConfig.simulation.defectProbability = 0.0;

        SimulatedImageSource normalSource;
        normalSource.open(normalConfig);
        const ImageFrame normalPositioning = normalSource.capture(FrameRole::Positioning);
        const ImageFrame normalInspection = normalSource.capture(FrameRole::Inspection);
        QVERIFY(!normalPositioning.metadata.hasDefect);
        QVERIFY(!normalInspection.metadata.hasDefect);
        QVERIFY(normalInspection.metadata.defectType == DefectType::None);
        QCOMPARE(normalInspection.metadata.defectScore, 0.0);

        AppConfig defectConfig = defaultAppConfig();
        defectConfig.simulation.seed = 12;
        defectConfig.simulation.defectProbability = 1.0;

        SimulatedImageSource defectSource;
        defectSource.open(defectConfig);
        const ImageFrame defectPositioning = defectSource.capture(FrameRole::Positioning);
        const ImageFrame defectInspection = defectSource.capture(FrameRole::Inspection);
        QVERIFY(defectPositioning.metadata.hasDefect);
        QVERIFY(defectInspection.metadata.hasDefect);
        QVERIFY(defectInspection.metadata.defectType != DefectType::None);
        QVERIFY(defectInspection.metadata.defectScore > 0.0);
        QVERIFY(imageDigest(normalInspection.image) != imageDigest(defectInspection.image));
    }

    void inspectionCaptureWithoutPositioningCreatesScenario()
    {
        SimulatedImageSource source;
        source.open(defaultAppConfig());

        const ImageFrame inspection = source.capture(FrameRole::Inspection);

        QVERIFY(!inspection.image.isNull());
        QCOMPARE(inspection.metadata.scenarioId, 1);
        QVERIFY(inspection.metadata.role == FrameRole::Inspection);
    }
};

QTEST_GUILESS_MAIN(SimulatedImageSourceTest)

#include "simulated_image_source_test.moc"
