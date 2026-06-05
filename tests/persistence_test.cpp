#include <QFile>
#include <QSqlDatabase>
#include <QTemporaryDir>
#include <QTest>

#include "persistence/SQLiteRepository.h"

using namespace workpiece;

namespace {

InspectionRecord makeRecord(const QString &productId, InspectionDecision decision)
{
    InspectionRecord record;
    record.timestamp = QDateTime::fromString(QStringLiteral("2026-06-04T01:02:03.000Z"), Qt::ISODateWithMs);
    record.productId = productId;
    record.sourceType = QStringLiteral("simulated");
    record.imageRef = QStringLiteral("inspection_images/%1.png").arg(productId);
    record.result = decision;
    record.defectType = decision == InspectionDecision::Fail ? DefectType::Scratch : DefectType::None;
    record.defectScore = decision == InspectionDecision::Fail ? 82.5 : 0.0;
    record.offset = PositionOffset{1.25, -2.5, 0.75};
    record.motionState.targetX = -1.25;
    record.motionState.targetY = 2.5;
    record.motionState.actualX = -1.24;
    record.motionState.actualY = 2.51;
    record.communicationState.status = CommunicationStatus::Connected;
    record.cycleTimeMs = 123;
    record.errorMessage = decision == InspectionDecision::Fail ? QStringLiteral("defect over threshold") : QString();
    return record;
}

InspectionRecord makeRecordAt(const QString &productId, InspectionDecision decision, const QString &timestamp)
{
    InspectionRecord record = makeRecord(productId, decision);
    record.timestamp = QDateTime::fromString(timestamp, Qt::ISODateWithMs);
    return record;
}

} // namespace

class PersistenceTest : public QObject
{
    Q_OBJECT

private slots:
    void sqliteDriverIsAvailable()
    {
        QVERIFY(QSqlDatabase::isDriverAvailable(QStringLiteral("QSQLITE")));
    }

    void initializeCreatesDatabaseFile()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString databasePath = directory.filePath(QStringLiteral("test.sqlite"));
        SQLiteRepository repository;

        QVERIFY(repository.open(databasePath).success);
        QVERIFY(repository.initialize().success);
        QVERIFY(repository.isOpen());
        QVERIFY(QFile::exists(databasePath));
    }

    void inspectionRecordsCanBeInsertedQueriedAndSummarized()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        SQLiteRepository repository;
        QVERIFY(repository.open(directory.filePath(QStringLiteral("records.sqlite"))).success);
        QVERIFY(repository.initialize().success);

        QVERIFY(repository.saveInspectionRecord(makeRecord(QStringLiteral("P-001"), InspectionDecision::Pass)).success);
        QVERIFY(repository.saveInspectionRecord(makeRecord(QStringLiteral("P-002"), InspectionDecision::Fail)).success);
        QVERIFY(repository.saveInspectionRecord(makeRecord(QStringLiteral("P-003"), InspectionDecision::Pass)).success);

        const QList<InspectionRecord> records = repository.queryInspectionHistory();
        QCOMPARE(records.size(), 3);
        QCOMPARE(records.first().productId, QStringLiteral("P-003"));
        QCOMPARE(records.first().imageRef, QStringLiteral("inspection_images/P-003.png"));
        QVERIFY(records.first().result == InspectionDecision::Pass);
        QVERIFY(records.first().communicationState.status == CommunicationStatus::Connected);
        QCOMPARE(records.first().cycleTimeMs, 123);

        InspectionHistoryFilter failFilter;
        failFilter.result = InspectionDecision::Fail;
        const QList<InspectionRecord> failed = repository.queryInspectionHistory(failFilter);
        QCOMPARE(failed.size(), 1);
        QCOMPARE(failed.first().productId, QStringLiteral("P-002"));
        QVERIFY(failed.first().defectType == DefectType::Scratch);
        QCOMPARE(failed.first().defectScore, 82.5);

        InspectionHistoryFilter productFilter;
        productFilter.productId = QStringLiteral("P-001");
        const QList<InspectionRecord> p001 = repository.queryInspectionHistory(productFilter);
        QCOMPARE(p001.size(), 1);
        QCOMPARE(p001.first().productId, QStringLiteral("P-001"));

        InspectionHistoryFilter combinedFilter;
        combinedFilter.productId = QStringLiteral("P-002");
        combinedFilter.result = InspectionDecision::Fail;
        const QList<InspectionRecord> p002Failed = repository.queryInspectionHistory(combinedFilter);
        QCOMPARE(p002Failed.size(), 1);
        QCOMPARE(p002Failed.first().productId, QStringLiteral("P-002"));

        const QualitySummary summary = repository.queryQualitySummary();
        QCOMPARE(summary.totalCount, 3);
        QCOMPARE(summary.passCount, 2);
        QCOMPARE(summary.failCount, 1);
        QCOMPARE(summary.passRate, 2.0 / 3.0);
    }

    void inspectionHistoryCanBeFilteredByTimestamp()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        SQLiteRepository repository;
        QVERIFY(repository.open(directory.filePath(QStringLiteral("time-filter.sqlite"))).success);
        QVERIFY(repository.initialize().success);

        QVERIFY(repository.saveInspectionRecord(makeRecordAt(QStringLiteral("OLD"), InspectionDecision::Pass, QStringLiteral("2026-06-04T01:00:00.000Z"))).success);
        QVERIFY(repository.saveInspectionRecord(makeRecordAt(QStringLiteral("MID"), InspectionDecision::Fail, QStringLiteral("2026-06-04T02:00:00.000Z"))).success);
        QVERIFY(repository.saveInspectionRecord(makeRecordAt(QStringLiteral("NEW"), InspectionDecision::Pass, QStringLiteral("2026-06-04T03:00:00.000Z"))).success);

        InspectionHistoryFilter filter;
        filter.fromTimestamp = QDateTime::fromString(QStringLiteral("2026-06-04T01:30:00.000Z"), Qt::ISODateWithMs);
        filter.toTimestamp = QDateTime::fromString(QStringLiteral("2026-06-04T02:30:00.000Z"), Qt::ISODateWithMs);

        const QList<InspectionRecord> records = repository.queryInspectionHistory(filter);
        QCOMPARE(records.size(), 1);
        QCOMPARE(records.first().productId, QStringLiteral("MID"));
    }

    void operatorAndFaultLogsCanBeInsertedAndQueried()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        SQLiteRepository repository;
        QVERIFY(repository.open(directory.filePath(QStringLiteral("logs.sqlite"))).success);
        QVERIFY(repository.initialize().success);

        OperatorLog operatorLog;
        operatorLog.timestamp = QDateTime::currentDateTimeUtc();
        operatorLog.action = QStringLiteral("start_cycle");
        operatorLog.details = QStringLiteral("manual test");
        QVERIFY(repository.saveOperatorLog(operatorLog).success);

        FaultLog faultLog;
        faultLog.timestamp = QDateTime::currentDateTimeUtc();
        faultLog.source = QStringLiteral("vision");
        faultLog.severity = FaultSeverity::Error;
        faultLog.message = QStringLiteral("test fault");
        faultLog.resolved = false;
        QVERIFY(repository.saveFaultLog(faultLog).success);

        const QList<OperatorLog> operatorLogs = repository.queryOperatorLogs();
        QCOMPARE(operatorLogs.size(), 1);
        QCOMPARE(operatorLogs.first().action, QStringLiteral("start_cycle"));
        QCOMPARE(operatorLogs.first().details, QStringLiteral("manual test"));

        const QList<FaultLog> faultLogs = repository.queryFaultLogs();
        QCOMPARE(faultLogs.size(), 1);
        QCOMPARE(faultLogs.first().source, QStringLiteral("vision"));
        QVERIFY(faultLogs.first().severity == FaultSeverity::Error);
        QCOMPARE(faultLogs.first().message, QStringLiteral("test fault"));
        QVERIFY(!faultLogs.first().resolved);
    }

    void operationsBeforeOpenOrInitializeFailClearly()
    {
        SQLiteRepository repository;

        QVERIFY(!repository.saveInspectionRecord(makeRecord(QStringLiteral("P-001"), InspectionDecision::Pass)).success);
        QVERIFY(repository.lastError().contains(QStringLiteral("opening")));

        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        QVERIFY(repository.open(directory.filePath(QStringLiteral("not_initialized.sqlite"))).success);

        QVERIFY(!repository.saveOperatorLog(OperatorLog{}).success);
        QVERIFY(repository.lastError().contains(QStringLiteral("initializing")));
    }

    void emptyDatabasePathFails()
    {
        SQLiteRepository repository;

        const PersistenceResult result = repository.open(QStringLiteral("   "));

        QVERIFY(!result.success);
        QVERIFY(result.message.contains(QStringLiteral("empty")));
        QVERIFY(!repository.isOpen());
    }
};

QTEST_MAIN(PersistenceTest)

#include "persistence_test.moc"
