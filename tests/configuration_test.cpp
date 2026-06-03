#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTest>

#include "configuration/AppConfig.h"

using namespace workpiece;

class ConfigurationTest : public QObject
{
    Q_OBJECT

private slots:
    void defaultConfigurationLoadsWithoutDiagnostics()
    {
        const ConfigLoadResult result = loadAppConfig();

        QVERIFY(!result.loadedFromFile);
        QVERIFY(result.diagnostics.isEmpty());
        QCOMPARE(result.config.simulation.seed, 12345);
        QCOMPARE(result.config.simulation.defectProbability, 0.20);
        QCOMPARE(result.config.simulation.noiseLevel, 0.0);
        QCOMPARE(result.config.inspection.defectScoreFailThreshold, 50.0);
        QCOMPARE(result.config.inspection.minimumPositionConfidence, 0.60);
        QVERIFY(result.config.imageSource.mode == ImageSourceMode::Simulated);
        QVERIFY(result.config.imageSource.networkCameraUrl.isEmpty());
        QCOMPARE(result.config.motion.maxVelocity, 100.0);
        QCOMPARE(result.config.motion.positionTolerance, 0.01);
        QVERIFY(result.config.communication.enabled);
        QCOMPARE(result.config.communication.minLatencyMs, 5);
        QCOMPARE(result.config.communication.maxLatencyMs, 50);
        QCOMPARE(result.config.communication.faultProbability, 0.0);
        QCOMPARE(result.config.persistence.databasePath, QStringLiteral("workpiece_inspector.sqlite"));
        QCOMPARE(result.config.persistence.imageOutputDirectory, QStringLiteral("inspection_images"));
        QVERIFY(result.config.persistence.saveImages);
        QVERIFY(result.config.ui.showCharts);
    }

    void validJsonOverridesDefaults()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString path = directory.filePath(QStringLiteral("config.json"));
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(QByteArray(
            "{"
            "\"simulation\":{\"seed\":7,\"defectProbability\":0.75,\"noiseLevel\":2.5},"
            "\"inspection\":{\"defectScoreFailThreshold\":42.5,\"minimumPositionConfidence\":0.8},"
            "\"imageSource\":{\"mode\":\"network_camera\",\"networkCameraUrl\":\"rtsp://example.com/stream\"},"
            "\"motion\":{\"maxVelocity\":250.0,\"positionTolerance\":0.05},"
            "\"communication\":{\"enabled\":false,\"minLatencyMs\":10,\"maxLatencyMs\":20,\"faultProbability\":0.1},"
            "\"persistence\":{\"databasePath\":\"data/app.sqlite\",\"imageOutputDirectory\":\"data/images\",\"saveImages\":false},"
            "\"ui\":{\"showCharts\":false}"
            "}"));
        file.close();

        const ConfigLoadResult result = loadAppConfig(path);

        QVERIFY(result.loadedFromFile);
        QVERIFY(result.diagnostics.isEmpty());
        QCOMPARE(result.config.simulation.seed, 7);
        QCOMPARE(result.config.simulation.defectProbability, 0.75);
        QCOMPARE(result.config.simulation.noiseLevel, 2.5);
        QCOMPARE(result.config.inspection.defectScoreFailThreshold, 42.5);
        QCOMPARE(result.config.inspection.minimumPositionConfidence, 0.8);
        QVERIFY(result.config.imageSource.mode == ImageSourceMode::NetworkCamera);
        QCOMPARE(result.config.imageSource.networkCameraUrl, QStringLiteral("rtsp://example.com/stream"));
        QCOMPARE(result.config.motion.maxVelocity, 250.0);
        QCOMPARE(result.config.motion.positionTolerance, 0.05);
        QVERIFY(!result.config.communication.enabled);
        QCOMPARE(result.config.communication.minLatencyMs, 10);
        QCOMPARE(result.config.communication.maxLatencyMs, 20);
        QCOMPARE(result.config.communication.faultProbability, 0.1);
        QCOMPARE(result.config.persistence.databasePath, QStringLiteral("data/app.sqlite"));
        QCOMPARE(result.config.persistence.imageOutputDirectory, QStringLiteral("data/images"));
        QVERIFY(!result.config.persistence.saveImages);
        QVERIFY(!result.config.ui.showCharts);
    }

    void invalidValuesFallBackToSafeDefaults()
    {
        AppConfig candidate = defaultAppConfig();
        candidate.simulation.seed = -1;
        candidate.simulation.defectProbability = 2.0;
        candidate.simulation.noiseLevel = -0.1;
        candidate.inspection.defectScoreFailThreshold = -5.0;
        candidate.inspection.minimumPositionConfidence = 1.5;
        candidate.imageSource.mode = ImageSourceMode::NetworkCamera;
        candidate.imageSource.networkCameraUrl = QStringLiteral("not-a-camera-url");
        candidate.motion.maxVelocity = 0.0;
        candidate.motion.positionTolerance = -0.1;
        candidate.communication.minLatencyMs = -1;
        candidate.communication.maxLatencyMs = -10;
        candidate.communication.faultProbability = -0.5;
        candidate.persistence.databasePath = QStringLiteral("   ");
        candidate.persistence.imageOutputDirectory = QString();

        const ConfigLoadResult result = validateAppConfig(candidate);

        QCOMPARE(result.config.simulation.seed, 12345);
        QCOMPARE(result.config.simulation.defectProbability, 0.20);
        QCOMPARE(result.config.simulation.noiseLevel, 0.0);
        QCOMPARE(result.config.inspection.defectScoreFailThreshold, 50.0);
        QCOMPARE(result.config.inspection.minimumPositionConfidence, 0.60);
        QVERIFY(result.config.imageSource.mode == ImageSourceMode::Simulated);
        QVERIFY(result.config.imageSource.networkCameraUrl.isEmpty());
        QCOMPARE(result.config.motion.maxVelocity, 100.0);
        QCOMPARE(result.config.motion.positionTolerance, 0.01);
        QCOMPARE(result.config.communication.minLatencyMs, 5);
        QCOMPARE(result.config.communication.maxLatencyMs, 50);
        QCOMPARE(result.config.communication.faultProbability, 0.0);
        QCOMPARE(result.config.persistence.databasePath, QStringLiteral("workpiece_inspector.sqlite"));
        QCOMPARE(result.config.persistence.imageOutputDirectory, QStringLiteral("inspection_images"));
        QVERIFY(result.diagnostics.size() >= 12);
    }

    void missingAndInvalidFilesUseDefaultsWithDiagnostics()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const ConfigLoadResult missing = loadAppConfig(directory.filePath(QStringLiteral("missing.json")));
        QVERIFY(!missing.loadedFromFile);
        QVERIFY(!missing.diagnostics.isEmpty());
        QCOMPARE(missing.config.persistence.databasePath, QStringLiteral("workpiece_inspector.sqlite"));

        const QString invalidPath = directory.filePath(QStringLiteral("invalid.json"));
        QFile invalidFile(invalidPath);
        QVERIFY(invalidFile.open(QIODevice::WriteOnly));
        invalidFile.write("{ invalid json");
        invalidFile.close();

        const ConfigLoadResult invalid = loadAppConfig(invalidPath);
        QVERIFY(!invalid.loadedFromFile);
        QVERIFY(!invalid.diagnostics.isEmpty());
        QCOMPARE(invalid.config.persistence.databasePath, QStringLiteral("workpiece_inspector.sqlite"));
    }

    void imageSourceModeStringIsStable()
    {
        QCOMPARE(toString(ImageSourceMode::Simulated), QStringLiteral("simulated"));
        QCOMPARE(toString(ImageSourceMode::NetworkCamera), QStringLiteral("network_camera"));
    }
};

QTEST_APPLESS_MAIN(ConfigurationTest)

#include "configuration_test.moc"
