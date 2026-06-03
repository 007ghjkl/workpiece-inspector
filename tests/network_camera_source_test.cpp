#include <QTest>

#include "configuration/AppConfig.h"
#include "vision/NetworkCameraImageSource.h"
#include "vision/SimulatedImageSource.h"

using namespace workpiece;

class NetworkCameraSourceTest : public QObject
{
    Q_OBJECT

private slots:
    void invalidUrlFailsClearly()
    {
        AppConfig config = defaultAppConfig();
        config.imageSource.mode = ImageSourceMode::NetworkCamera;
        config.imageSource.networkCameraUrl = QStringLiteral("not-a-url");

        NetworkCameraImageSource source;
        source.open(config);

        QVERIFY(source.status().state == ImageSourceState::Faulted);
        QVERIFY(source.status().message.contains(QStringLiteral("URL")));

        const ImageFrame frame = source.capture(FrameRole::Inspection);
        QVERIFY(frame.image.isNull());
        QCOMPARE(frame.metadata.sourceType, QStringLiteral("network_camera"));
        QVERIFY(frame.metadata.role == FrameRole::Inspection);
    }

    void wrongModeFailsClearly()
    {
        AppConfig config = defaultAppConfig();
        config.imageSource.mode = ImageSourceMode::Simulated;

        NetworkCameraImageSource source;
        source.open(config);

        QVERIFY(source.status().state == ImageSourceState::Faulted);
        QVERIFY(source.status().message.contains(QStringLiteral("mode")));
    }

    void closeReturnsSourceToClosedState()
    {
        AppConfig config = defaultAppConfig();
        config.imageSource.mode = ImageSourceMode::NetworkCamera;
        config.imageSource.networkCameraUrl = QStringLiteral("not-a-url");

        NetworkCameraImageSource source;
        source.open(config);
        QVERIFY(source.status().state == ImageSourceState::Faulted);

        source.close();
        QVERIFY(source.status().state == ImageSourceState::Closed);
    }

    void simulationSourceStillWorksAfterNetworkFailure()
    {
        AppConfig networkConfig = defaultAppConfig();
        networkConfig.imageSource.mode = ImageSourceMode::NetworkCamera;
        networkConfig.imageSource.networkCameraUrl = QStringLiteral("not-a-url");

        NetworkCameraImageSource networkSource;
        networkSource.open(networkConfig);
        QVERIFY(networkSource.status().state == ImageSourceState::Faulted);

        AppConfig simulatedConfig = defaultAppConfig();
        simulatedConfig.simulation.seed = 123;
        simulatedConfig.simulation.defectProbability = 0.0;

        SimulatedImageSource simulatedSource;
        simulatedSource.open(simulatedConfig);
        const ImageFrame frame = simulatedSource.capture(FrameRole::Positioning);

        QVERIFY(simulatedSource.status().state == ImageSourceState::Ready);
        QVERIFY(!frame.image.isNull());
        QCOMPARE(frame.metadata.sourceType, QStringLiteral("simulated"));
    }
};

QTEST_APPLESS_MAIN(NetworkCameraSourceTest)

#include "network_camera_source_test.moc"
