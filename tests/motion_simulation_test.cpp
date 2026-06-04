#include <QTest>

#include "motion/SimulatedMotionController.h"

using namespace workpiece;

class MotionSimulationTest : public QObject
{
    Q_OBJECT

private slots:
    void defaultStateIsIdle()
    {
        SimulatedMotionController controller;
        controller.configure(defaultAppConfig());

        const MotionState state = controller.state();
        QCOMPARE(state.targetX, 0.0);
        QCOMPARE(state.targetY, 0.0);
        QCOMPARE(state.actualX, 0.0);
        QCOMPARE(state.actualY, 0.0);
        QCOMPARE(state.velocity, 0.0);
        QVERIFY(state.status == MotionStatus::Idle);
    }

    void moveToCompletesDeterministically()
    {
        AppConfig config = defaultAppConfig();
        config.motion.maxVelocity = 125.0;
        config.motion.positionTolerance = 0.05;

        SimulatedMotionController controller;
        controller.configure(config);

        const MotionCommandResult result = controller.moveTo(MotionCommand{10.0, -20.0});

        QVERIFY(result.success);
        QVERIFY(result.state.status == MotionStatus::Completed);
        QCOMPARE(result.state.targetX, 10.0);
        QCOMPARE(result.state.targetY, -20.0);
        QCOMPARE(result.state.actualX, 10.05);
        QCOMPARE(result.state.actualY, -19.95);
        QCOMPARE(result.state.velocity, 125.0);
        QVERIFY(!result.message.isEmpty());
    }

    void correctionUsesNegativePositionOffset()
    {
        AppConfig config = defaultAppConfig();
        config.motion.positionTolerance = 0.01;

        SimulatedMotionController controller;
        controller.configure(config);

        const MotionCommandResult result = controller.applyCorrection(PositionOffset{3.5, -4.0, 2.0});

        QVERIFY(result.success);
        QVERIFY(result.state.status == MotionStatus::Completed);
        QCOMPARE(result.state.targetX, -3.5);
        QCOMPARE(result.state.targetY, 4.0);
        QCOMPARE(result.state.actualX, -3.49);
        QCOMPARE(result.state.actualY, 4.01);
    }

    void stopSetsStoppedStateAndClearsVelocity()
    {
        SimulatedMotionController controller;
        controller.configure(defaultAppConfig());
        QVERIFY(controller.moveTo(MotionCommand{1.0, 2.0}).success);

        controller.stop();
        const MotionState stopped = controller.state();

        QVERIFY(stopped.status == MotionStatus::Stopped);
        QCOMPARE(stopped.velocity, 0.0);
        QCOMPARE(stopped.targetX, 1.0);
        QCOMPARE(stopped.targetY, 2.0);
    }

    void configuredFaultIsSurfacedAndCanBeReset()
    {
        AppConfig config = defaultAppConfig();
        config.motion.faultEnabled = true;

        SimulatedMotionController controller;
        controller.configure(config);

        const MotionCommandResult result = controller.moveTo(MotionCommand{10.0, 20.0});

        QVERIFY(!result.success);
        QVERIFY(result.state.status == MotionStatus::Faulted);
        QVERIFY(result.message.contains(QStringLiteral("fault")));

        controller.resetFault();
        QVERIFY(controller.state().status == MotionStatus::Idle);
        QCOMPARE(controller.state().velocity, 0.0);
    }
};

QTEST_APPLESS_MAIN(MotionSimulationTest)

#include "motion_simulation_test.moc"
