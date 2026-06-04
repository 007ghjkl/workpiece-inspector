#include <QTest>

#include "communication/SimulatedCommunicationEndpoint.h"

using namespace workpiece;

class CommunicationSimulationTest : public QObject
{
    Q_OBJECT

private slots:
    void defaultStateIsDisconnected()
    {
        SimulatedCommunicationEndpoint endpoint;
        endpoint.configure(defaultAppConfig());

        const CommunicationState state = endpoint.state();
        QVERIFY(state.status == CommunicationStatus::Disconnected);
        QVERIFY(!state.polling);
        QVERIFY(!state.commandInProgress);
        QVERIFY(state.diagnosticSummary.empty());
        QVERIFY(!endpoint.isReady());
    }

    void connectAndDisconnectUpdateReadiness()
    {
        SimulatedCommunicationEndpoint endpoint;
        endpoint.configure(defaultAppConfig());

        const CommunicationResult connected = endpoint.connectEndpoint();
        QVERIFY(connected.success);
        QVERIFY(connected.state.status == CommunicationStatus::Connected);
        QVERIFY(endpoint.isReady());

        const CommunicationResult disconnected = endpoint.disconnectEndpoint();
        QVERIFY(disconnected.success);
        QVERIFY(disconnected.state.status == CommunicationStatus::Disconnected);
        QVERIFY(!endpoint.isReady());
    }

    void pollingRequiresConnectedStateAndReturnsReady()
    {
        SimulatedCommunicationEndpoint endpoint;
        endpoint.configure(defaultAppConfig());

        const CommunicationResult beforeConnect = endpoint.poll();
        QVERIFY(!beforeConnect.success);
        QVERIFY(beforeConnect.state.status == CommunicationStatus::Faulted);

        endpoint.configure(defaultAppConfig());
        QVERIFY(endpoint.connectEndpoint().success);
        const CommunicationResult poll = endpoint.poll();

        QVERIFY(poll.success);
        QVERIFY(poll.state.status == CommunicationStatus::Connected);
        QVERIFY(!poll.state.polling);
        QVERIFY(endpoint.isReady());
        QVERIFY(QString::fromStdString(poll.state.diagnosticSummary).contains(QStringLiteral("latency")));
    }

    void commandRequiresConnectedStateAndName()
    {
        SimulatedCommunicationEndpoint endpoint;
        endpoint.configure(defaultAppConfig());

        const CommunicationResult beforeConnect = endpoint.writeCommand(CommunicationCommand{QStringLiteral("start_cycle")});
        QVERIFY(!beforeConnect.success);
        QVERIFY(beforeConnect.state.status == CommunicationStatus::Faulted);

        endpoint.configure(defaultAppConfig());
        QVERIFY(endpoint.connectEndpoint().success);
        const CommunicationResult empty = endpoint.writeCommand(CommunicationCommand{QStringLiteral("   ")});
        QVERIFY(!empty.success);
        QVERIFY(empty.state.status == CommunicationStatus::Faulted);

        endpoint.configure(defaultAppConfig());
        QVERIFY(endpoint.connectEndpoint().success);
        const CommunicationResult command = endpoint.writeCommand(CommunicationCommand{QStringLiteral("start_cycle")});
        QVERIFY(command.success);
        QVERIFY(command.state.status == CommunicationStatus::Connected);
        QVERIFY(!command.state.commandInProgress);
        QVERIFY(endpoint.isReady());
    }

    void configuredFaultsAreReported()
    {
        AppConfig config = defaultAppConfig();
        config.communication.faultProbability = 1.0;

        SimulatedCommunicationEndpoint endpoint;
        endpoint.configure(config);

        const CommunicationResult result = endpoint.connectEndpoint();

        QVERIFY(!result.success);
        QVERIFY(result.state.status == CommunicationStatus::Faulted);
        QVERIFY(result.message.contains(QStringLiteral("fault")));
        QVERIFY(!endpoint.isReady());
    }

    void disabledCommunicationFailsConnect()
    {
        AppConfig config = defaultAppConfig();
        config.communication.enabled = false;

        SimulatedCommunicationEndpoint endpoint;
        endpoint.configure(config);

        const CommunicationResult result = endpoint.connectEndpoint();

        QVERIFY(!result.success);
        QVERIFY(result.state.status == CommunicationStatus::Faulted);
        QVERIFY(result.message.contains(QStringLiteral("disabled")));
    }
};

QTEST_APPLESS_MAIN(CommunicationSimulationTest)

#include "communication_simulation_test.moc"
