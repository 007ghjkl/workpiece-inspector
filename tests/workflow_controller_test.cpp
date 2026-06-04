#include <QFile>
#include <QTemporaryDir>
#include <QTest>

#include "communication/CommunicationEndpoint.h"
#include "motion/MotionController.h"
#include "persistence/SQLiteRepository.h"
#include "storage/ImageStorage.h"
#include "workflow/WorkflowController.h"

using namespace workpiece;

namespace {

ImageFrame makeFrame(FrameRole requestedRole)
{
    ImageFrame frame;
    frame.image = QImage(64, 64, QImage::Format_RGB32);
    frame.image.fill(requestedRole == FrameRole::Positioning ? Qt::white : Qt::lightGray);
    frame.metadata.scenarioId = 7;
    frame.metadata.productId = QStringLiteral("WF-000007");
    frame.metadata.role = requestedRole;
    frame.metadata.sourceType = QStringLiteral("test_source");
    frame.metadata.offset = PositionOffset{2.0, -3.0, 1.0};
    frame.metadata.defectType = DefectType::None;
    frame.metadata.defectScore = 0.0;
    frame.metadata.hasDefect = false;
    return frame;
}

class FakeImageSource final : public IImageSource
{
public:
    bool readyAfterOpen = true;
    bool failPositioning = false;
    bool failInspection = false;
    bool wrongInspectionRole = false;
    int positioningCaptures = 0;
    int inspectionCaptures = 0;

    void open(const AppConfig &) override
    {
        state_ = readyAfterOpen ? ImageSourceState::Ready : ImageSourceState::Faulted;
    }

    void close() override
    {
        state_ = ImageSourceState::Closed;
    }

    ImageFrame capture(FrameRole role) override
    {
        if (state_ != ImageSourceState::Ready) {
            return ImageFrame{};
        }

        if (role == FrameRole::Positioning) {
            ++positioningCaptures;
            return failPositioning ? ImageFrame{} : makeFrame(FrameRole::Positioning);
        }

        ++inspectionCaptures;
        if (failInspection) {
            return ImageFrame{};
        }

        return wrongInspectionRole ? makeFrame(FrameRole::Positioning) : makeFrame(FrameRole::Inspection);
    }

    ImageSourceStatus status() const override
    {
        return ImageSourceStatus{state_, "Fake image source status."};
    }

private:
    ImageSourceState state_ = ImageSourceState::Closed;
};

class FakeMotionController final : public IMotionController
{
public:
    bool failCorrection = false;
    int configureCalls = 0;
    int correctionCalls = 0;

    void configure(const AppConfig &) override
    {
        ++configureCalls;
        state_ = MotionState{};
    }

    MotionCommandResult moveTo(const MotionCommand &command) override
    {
        state_.targetX = command.targetX;
        state_.targetY = command.targetY;
        state_.actualX = command.targetX;
        state_.actualY = command.targetY;
        state_.status = MotionStatus::Completed;
        return MotionCommandResult{true, state_, QStringLiteral("Fake move completed.")};
    }

    MotionCommandResult applyCorrection(const PositionOffset &offset) override
    {
        ++correctionCalls;
        if (failCorrection) {
            state_.status = MotionStatus::Faulted;
            return MotionCommandResult{false, state_, QStringLiteral("Fake motion correction failed.")};
        }

        return moveTo(MotionCommand{-offset.x, -offset.y});
    }

    void stop() override
    {
        state_.status = MotionStatus::Stopped;
    }

    void resetFault() override
    {
        state_.status = MotionStatus::Idle;
    }

    MotionState state() const override
    {
        return state_;
    }

private:
    MotionState state_;
};

class FakeCommunicationEndpoint final : public ICommunicationEndpoint
{
public:
    bool failConnect = false;
    bool failPoll = false;
    int configureCalls = 0;
    int connectCalls = 0;
    int pollCalls = 0;

    void configure(const AppConfig &) override
    {
        ++configureCalls;
        state_ = CommunicationState{};
    }

    CommunicationResult connectEndpoint() override
    {
        ++connectCalls;
        if (failConnect) {
            state_.status = CommunicationStatus::Faulted;
            return CommunicationResult{false, state_, QStringLiteral("Fake communication connect failed.")};
        }

        state_.status = CommunicationStatus::Connected;
        return CommunicationResult{true, state_, QStringLiteral("Fake communication connected.")};
    }

    CommunicationResult disconnectEndpoint() override
    {
        state_.status = CommunicationStatus::Disconnected;
        return CommunicationResult{true, state_, QStringLiteral("Fake communication disconnected.")};
    }

    CommunicationResult poll() override
    {
        ++pollCalls;
        if (failPoll) {
            state_.status = CommunicationStatus::Faulted;
            return CommunicationResult{false, state_, QStringLiteral("Fake communication poll failed.")};
        }

        state_.status = CommunicationStatus::Connected;
        return CommunicationResult{true, state_, QStringLiteral("Fake communication poll completed.")};
    }

    CommunicationResult writeCommand(const CommunicationCommand &) override
    {
        return CommunicationResult{true, state_, QStringLiteral("Fake communication command completed.")};
    }

    bool isReady() const override
    {
        return state_.status == CommunicationStatus::Connected;
    }

    CommunicationState state() const override
    {
        return state_;
    }

private:
    CommunicationState state_;
};

struct Fixture {
    QTemporaryDir tempDir;
    AppConfig config;
    FakeImageSource imageSource;
    RuleBasedInspector inspector;
    FakeMotionController motion;
    FakeCommunicationEndpoint communication;
    ImageStorage storage;
    SQLiteRepository repository;

    Fixture()
    {
        config = defaultAppConfig();
        config.persistence.databasePath = tempDir.path() + QStringLiteral("/workflow.sqlite");
        config.persistence.imageOutputDirectory = tempDir.path() + QStringLiteral("/images");
        config.persistence.saveImages = true;
    }

    bool openRepository()
    {
        return tempDir.isValid()
            && repository.open(config.persistence.databasePath).success
            && repository.initialize().success;
    }

    WorkflowController controller()
    {
        return WorkflowController(config, imageSource, inspector, motion, communication, storage, repository);
    }
};

} // namespace

class WorkflowControllerTest : public QObject
{
    Q_OBJECT

private slots:
    void successfulSingleCyclePersistsRecordAndPublishesStates()
    {
        Fixture fixture;
        QVERIFY(fixture.openRepository());

        WorkflowController controller = fixture.controller();
        QList<StationState> states;
        controller.addObserver([&states](const WorkflowSnapshot &snapshot) {
            states.push_back(snapshot.stationState);
        });

        const WorkflowRunResult result = controller.runSingleCycle();

        QVERIFY(result.success);
        QVERIFY(result.finalState == StationState::Completed);
        QVERIFY(controller.state() == StationState::Completed);
        QVERIFY(states.contains(StationState::Running));
        QVERIFY(states.contains(StationState::Completed));
        QCOMPARE(fixture.communication.connectCalls, 1);
        QCOMPARE(fixture.communication.pollCalls, 1);
        QCOMPARE(fixture.imageSource.positioningCaptures, 1);
        QCOMPARE(fixture.imageSource.inspectionCaptures, 1);
        QCOMPARE(fixture.motion.correctionCalls, 1);

        const QList<InspectionRecord> records = fixture.repository.queryInspectionHistory();
        QCOMPARE(records.size(), 1);
        QCOMPARE(records.first().productId, QStringLiteral("WF-000007"));
        QVERIFY(records.first().result == InspectionDecision::Pass);
        QVERIFY(!records.first().imageRef.isEmpty());
        QVERIFY(QFile::exists(fixture.config.persistence.imageOutputDirectory + QStringLiteral("/") + records.first().imageRef));
        QCOMPARE(fixture.repository.queryFaultLogs().size(), 0);
    }

    void communicationFailureFaultsAndPersistsFaultLog()
    {
        Fixture fixture;
        QVERIFY(fixture.openRepository());
        fixture.communication.failPoll = true;

        WorkflowController controller = fixture.controller();
        const WorkflowRunResult result = controller.runSingleCycle();

        QVERIFY(!result.success);
        QVERIFY(result.finalState == StationState::Faulted);
        QCOMPARE(fixture.repository.queryInspectionHistory().size(), 0);
        const QList<FaultLog> faults = fixture.repository.queryFaultLogs();
        QCOMPARE(faults.size(), 1);
        QCOMPARE(faults.first().source, QStringLiteral("workflow"));
        QVERIFY(faults.first().message.contains(QStringLiteral("poll")));
    }

    void imageCaptureFailureFaultsAndPersistsFaultLog()
    {
        Fixture fixture;
        QVERIFY(fixture.openRepository());
        fixture.imageSource.failPositioning = true;

        WorkflowController controller = fixture.controller();
        const WorkflowRunResult result = controller.runSingleCycle();

        QVERIFY(!result.success);
        QVERIFY(result.finalState == StationState::Faulted);
        QCOMPARE(fixture.repository.queryInspectionHistory().size(), 0);
        QVERIFY(fixture.repository.queryFaultLogs().first().message.contains(QStringLiteral("Positioning")));
    }

    void motionFailureFaultsAndPersistsFaultLog()
    {
        Fixture fixture;
        QVERIFY(fixture.openRepository());
        fixture.motion.failCorrection = true;

        WorkflowController controller = fixture.controller();
        const WorkflowRunResult result = controller.runSingleCycle();

        QVERIFY(!result.success);
        QVERIFY(result.finalState == StationState::Faulted);
        QCOMPARE(fixture.repository.queryInspectionHistory().size(), 0);
        QVERIFY(fixture.repository.queryFaultLogs().first().message.contains(QStringLiteral("motion")));
    }

    void inspectionFailureFaultsAndPersistsFaultLog()
    {
        Fixture fixture;
        QVERIFY(fixture.openRepository());
        fixture.imageSource.wrongInspectionRole = true;

        WorkflowController controller = fixture.controller();
        const WorkflowRunResult result = controller.runSingleCycle();

        QVERIFY(!result.success);
        QVERIFY(result.finalState == StationState::Faulted);
        QCOMPARE(fixture.repository.queryInspectionHistory().size(), 0);
        QVERIFY(fixture.repository.queryFaultLogs().first().message.contains(QStringLiteral("inspection")));
    }

    void storageFailureFaultsBeforeRecordPersistence()
    {
        Fixture fixture;
        QVERIFY(fixture.openRepository());
        fixture.config.persistence.imageOutputDirectory.clear();

        WorkflowController controller = fixture.controller();
        const WorkflowRunResult result = controller.runSingleCycle();

        QVERIFY(!result.success);
        QVERIFY(result.finalState == StationState::Faulted);
        QCOMPARE(fixture.repository.queryInspectionHistory().size(), 0);
        QVERIFY(fixture.repository.queryFaultLogs().first().message.contains(QStringLiteral("directory")));
    }

    void persistenceFailureFaultsCycle()
    {
        Fixture fixture;
        fixture.config.persistence.saveImages = false;

        WorkflowController controller = fixture.controller();
        const WorkflowRunResult result = controller.runSingleCycle();

        QVERIFY(!result.success);
        QVERIFY(result.finalState == StationState::Faulted);
        QVERIFY(result.message.contains(QStringLiteral("save inspection record")));
        QVERIFY(result.message.contains(QStringLiteral("Fault log was not saved")));
    }
};

QTEST_GUILESS_MAIN(WorkflowControllerTest)

#include "workflow_controller_test.moc"
