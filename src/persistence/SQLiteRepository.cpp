#include "persistence/SQLiteRepository.h"

#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

#include <atomic>
#include <cstring>
#include <cstdio>

namespace workpiece {
namespace {

std::atomic<int> connectionCounter = 0;

QString dateTimeToString(const QDateTime &dateTime)
{
    const QDateTime value = dateTime.isValid() ? dateTime : QDateTime::currentDateTimeUtc();
    return value.toUTC().toString(Qt::ISODateWithMs);
}

QDateTime dateTimeFromString(const QString &value)
{
    return QDateTime::fromString(value, Qt::ISODateWithMs);
}

QString decisionToString(InspectionDecision decision)
{
    return QString::fromStdString(toString(decision));
}

QString defectTypeToString(DefectType type)
{
    return QString::fromStdString(toString(type));
}

QString communicationStatusToString(CommunicationStatus status)
{
    return QString::fromStdString(toString(status));
}

QString faultSeverityToString(FaultSeverity severity)
{
    return QString::fromStdString(toString(severity));
}

InspectionDecision decisionFromString(const QString &value)
{
    if (value == QStringLiteral("pass")) {
        return InspectionDecision::Pass;
    }
    if (value == QStringLiteral("fail")) {
        return InspectionDecision::Fail;
    }
    return InspectionDecision::Unknown;
}

DefectType defectTypeFromString(const QString &value)
{
    if (value == QStringLiteral("none")) {
        return DefectType::None;
    }
    if (value == QStringLiteral("scratch")) {
        return DefectType::Scratch;
    }
    if (value == QStringLiteral("spot")) {
        return DefectType::Spot;
    }
    if (value == QStringLiteral("deformation")) {
        return DefectType::Deformation;
    }
    return DefectType::Unknown;
}

CommunicationStatus communicationStatusFromString(const QString &value)
{
    if (value == QStringLiteral("connected")) {
        return CommunicationStatus::Connected;
    }
    if (value == QStringLiteral("polling")) {
        return CommunicationStatus::Polling;
    }
    if (value == QStringLiteral("command_in_progress")) {
        return CommunicationStatus::CommandInProgress;
    }
    if (value == QStringLiteral("faulted")) {
        return CommunicationStatus::Faulted;
    }
    return CommunicationStatus::Disconnected;
}

FaultSeverity faultSeverityFromString(const QString &value)
{
    if (value == QStringLiteral("warning")) {
        return FaultSeverity::Warning;
    }
    if (value == QStringLiteral("error")) {
        return FaultSeverity::Error;
    }
    return FaultSeverity::Info;
}

void copyFilterText(char *destination, std::size_t size, const QString &value)
{
    if (size == 0) {
        return;
    }

    const QByteArray bytes = value.trimmed().toUtf8();
    std::snprintf(destination, size, "%s", bytes.constData());
}

} // namespace

InspectionHistoryFilter::InspectionHistoryFilter()
{
    std::memset(productId, 0, sizeof(productId));
    std::memset(fromTimestampIso, 0, sizeof(fromTimestampIso));
    std::memset(toTimestampIso, 0, sizeof(toTimestampIso));
}

SQLiteRepository::SQLiteRepository()
    : connectionName_(QStringLiteral("workpiece_repository_%1").arg(++connectionCounter))
{
}

SQLiteRepository::~SQLiteRepository()
{
    close();
}

PersistenceResult SQLiteRepository::open(const QString &databasePath)
{
    close();

    if (databasePath.trimmed().isEmpty()) {
        lastError_ = QStringLiteral("Database path is empty.");
        return {false, lastError_};
    }

    if (!QSqlDatabase::isDriverAvailable(QStringLiteral("QSQLITE"))) {
        lastError_ = QStringLiteral("Qt SQLite driver QSQLITE is not available.");
        return {false, lastError_};
    }

    database_ = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connectionName_);
    database_.setDatabaseName(databasePath);

    if (!database_.open()) {
        lastError_ = database_.lastError().text();
        return {false, lastError_};
    }

    initialized_ = false;
    lastError_.clear();
    return {true, QStringLiteral("SQLite database opened.")};
}

void SQLiteRepository::close()
{
    if (database_.isValid()) {
        database_.close();
    }

    database_ = QSqlDatabase();
    QSqlDatabase::removeDatabase(connectionName_);
    initialized_ = false;
}

PersistenceResult SQLiteRepository::initialize()
{
    if (!isOpen()) {
        lastError_ = QStringLiteral("Cannot initialize database before opening it.");
        return {false, lastError_};
    }

    const QStringList schema = {
        QStringLiteral(
            "CREATE TABLE IF NOT EXISTS inspection_records ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "timestamp TEXT NOT NULL,"
            "product_id TEXT NOT NULL,"
            "source_type TEXT NOT NULL,"
            "image_ref TEXT,"
            "result TEXT NOT NULL,"
            "defect_type TEXT NOT NULL,"
            "defect_score REAL NOT NULL,"
            "offset_x REAL NOT NULL,"
            "offset_y REAL NOT NULL,"
            "offset_angle REAL NOT NULL,"
            "motion_target_x REAL NOT NULL,"
            "motion_target_y REAL NOT NULL,"
            "motion_actual_x REAL NOT NULL,"
            "motion_actual_y REAL NOT NULL,"
            "communication_state TEXT NOT NULL,"
            "cycle_time_ms INTEGER NOT NULL,"
            "error_message TEXT"
            ")"),
        QStringLiteral(
            "CREATE TABLE IF NOT EXISTS operator_logs ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "timestamp TEXT NOT NULL,"
            "action TEXT NOT NULL,"
            "details TEXT"
            ")"),
        QStringLiteral(
            "CREATE TABLE IF NOT EXISTS fault_logs ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "timestamp TEXT NOT NULL,"
            "source TEXT NOT NULL,"
            "severity TEXT NOT NULL,"
            "message TEXT NOT NULL,"
            "resolved INTEGER NOT NULL"
            ")"),
        QStringLiteral(
            "CREATE TABLE IF NOT EXISTS app_settings ("
            "key TEXT PRIMARY KEY,"
            "value TEXT,"
            "updated_at TEXT NOT NULL"
            ")")
    };

    for (const QString &statement : schema) {
        const PersistenceResult result = executeSchema(statement);
        if (!result.success) {
            return result;
        }
    }

    initialized_ = true;
    lastError_.clear();
    return {true, QStringLiteral("SQLite schema initialized.")};
}

PersistenceResult SQLiteRepository::saveInspectionRecord(const InspectionRecord &record)
{
    if (!ensureReady(QStringLiteral("save inspection record"))) {
        return {false, lastError_};
    }

    QSqlQuery query(database_);
    query.prepare(
        "INSERT INTO inspection_records ("
        "timestamp, product_id, source_type, image_ref, result, defect_type, defect_score,"
        "offset_x, offset_y, offset_angle, motion_target_x, motion_target_y,"
        "motion_actual_x, motion_actual_y, communication_state, cycle_time_ms, error_message"
        ") VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(dateTimeToString(record.timestamp));
    query.addBindValue(record.productId);
    query.addBindValue(record.sourceType);
    query.addBindValue(record.imageRef);
    query.addBindValue(decisionToString(record.result));
    query.addBindValue(defectTypeToString(record.defectType));
    query.addBindValue(record.defectScore);
    query.addBindValue(record.offset.x);
    query.addBindValue(record.offset.y);
    query.addBindValue(record.offset.angle);
    query.addBindValue(record.motionState.targetX);
    query.addBindValue(record.motionState.targetY);
    query.addBindValue(record.motionState.actualX);
    query.addBindValue(record.motionState.actualY);
    query.addBindValue(communicationStatusToString(record.communicationState.status));
    query.addBindValue(record.cycleTimeMs);
    query.addBindValue(record.errorMessage);

    if (!query.exec()) {
        lastError_ = query.lastError().text();
        return {false, lastError_};
    }

    return {true, QStringLiteral("Inspection record saved.")};
}

PersistenceResult SQLiteRepository::saveOperatorLog(const OperatorLog &log)
{
    if (!ensureReady(QStringLiteral("save operator log"))) {
        return {false, lastError_};
    }

    QSqlQuery query(database_);
    query.prepare("INSERT INTO operator_logs (timestamp, action, details) VALUES (?, ?, ?)");
    query.addBindValue(dateTimeToString(log.timestamp));
    query.addBindValue(log.action);
    query.addBindValue(log.details);

    if (!query.exec()) {
        lastError_ = query.lastError().text();
        return {false, lastError_};
    }

    return {true, QStringLiteral("Operator log saved.")};
}

PersistenceResult SQLiteRepository::saveFaultLog(const FaultLog &log)
{
    if (!ensureReady(QStringLiteral("save fault log"))) {
        return {false, lastError_};
    }

    QSqlQuery query(database_);
    query.prepare("INSERT INTO fault_logs (timestamp, source, severity, message, resolved) VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(dateTimeToString(log.timestamp));
    query.addBindValue(log.source);
    query.addBindValue(faultSeverityToString(log.severity));
    query.addBindValue(log.message);
    query.addBindValue(log.resolved ? 1 : 0);

    if (!query.exec()) {
        lastError_ = query.lastError().text();
        return {false, lastError_};
    }

    return {true, QStringLiteral("Fault log saved.")};
}

QList<InspectionRecord> SQLiteRepository::queryInspectionHistory() const
{
    const InspectionHistoryFilter filter;
    return queryInspectionHistory(filter);
}

QList<InspectionRecord> SQLiteRepository::queryInspectionHistory(const InspectionHistoryFilter &filter) const
{
    QList<InspectionRecord> records;
    if (!ensureReady(QStringLiteral("query inspection history"))) {
        return records;
    }

    QString sql = QStringLiteral(
        "SELECT id, timestamp, product_id, source_type, image_ref, result, defect_type, defect_score,"
        "offset_x, offset_y, offset_angle, motion_target_x, motion_target_y,"
        "motion_actual_x, motion_actual_y, communication_state, cycle_time_ms, error_message "
        "FROM inspection_records");

    QStringList conditions;
    const QString productId = QString::fromUtf8(filter.productId).trimmed();
    const QString fromTimestampIso = QString::fromUtf8(filter.fromTimestampIso).trimmed();
    const QString toTimestampIso = QString::fromUtf8(filter.toTimestampIso).trimmed();

    if (!productId.isEmpty()) {
        conditions.push_back(QStringLiteral("product_id = ?"));
    }
    if (filter.result != InspectionDecision::Unknown) {
        conditions.push_back(QStringLiteral("result = ?"));
    }
    if (!fromTimestampIso.isEmpty()) {
        conditions.push_back(QStringLiteral("timestamp >= ?"));
    }
    if (!toTimestampIso.isEmpty()) {
        conditions.push_back(QStringLiteral("timestamp <= ?"));
    }
    if (!conditions.isEmpty()) {
        sql += QStringLiteral(" WHERE ") + conditions.join(QStringLiteral(" AND "));
    }
    sql += QStringLiteral(" ORDER BY id DESC LIMIT ?");

    QSqlQuery query(database_);
    query.prepare(sql);
    if (!productId.isEmpty()) {
        query.addBindValue(productId);
    }
    if (filter.result != InspectionDecision::Unknown) {
        query.addBindValue(decisionToString(filter.result));
    }
    if (!fromTimestampIso.isEmpty()) {
        query.addBindValue(fromTimestampIso);
    }
    if (!toTimestampIso.isEmpty()) {
        query.addBindValue(toTimestampIso);
    }
    query.addBindValue(filter.limit > 0 ? filter.limit : 100);

    if (!query.exec()) {
        lastError_ = query.lastError().text();
        return records;
    }

    while (query.next()) {
        InspectionRecord record;
        record.id = query.value(0).toInt();
        record.timestamp = dateTimeFromString(query.value(1).toString());
        record.productId = query.value(2).toString();
        record.sourceType = query.value(3).toString();
        record.imageRef = query.value(4).toString();
        record.result = decisionFromString(query.value(5).toString());
        record.defectType = defectTypeFromString(query.value(6).toString());
        record.defectScore = query.value(7).toDouble();
        record.offset.x = query.value(8).toDouble();
        record.offset.y = query.value(9).toDouble();
        record.offset.angle = query.value(10).toDouble();
        record.motionState.targetX = query.value(11).toDouble();
        record.motionState.targetY = query.value(12).toDouble();
        record.motionState.actualX = query.value(13).toDouble();
        record.motionState.actualY = query.value(14).toDouble();
        record.communicationState.status = communicationStatusFromString(query.value(15).toString());
        record.cycleTimeMs = query.value(16).toInt();
        record.errorMessage = query.value(17).toString();
        records.push_back(record);
    }

    return records;
}

QList<OperatorLog> SQLiteRepository::queryOperatorLogs(int limit) const
{
    QList<OperatorLog> logs;
    if (!ensureReady(QStringLiteral("query operator logs"))) {
        return logs;
    }

    QSqlQuery query(database_);
    query.prepare("SELECT id, timestamp, action, details FROM operator_logs ORDER BY id DESC LIMIT ?");
    query.addBindValue(limit > 0 ? limit : 100);

    if (!query.exec()) {
        lastError_ = query.lastError().text();
        return logs;
    }

    while (query.next()) {
        OperatorLog log;
        log.id = query.value(0).toInt();
        log.timestamp = dateTimeFromString(query.value(1).toString());
        log.action = query.value(2).toString();
        log.details = query.value(3).toString();
        logs.push_back(log);
    }

    return logs;
}

QList<FaultLog> SQLiteRepository::queryFaultLogs(int limit) const
{
    QList<FaultLog> logs;
    if (!ensureReady(QStringLiteral("query fault logs"))) {
        return logs;
    }

    QSqlQuery query(database_);
    query.prepare("SELECT id, timestamp, source, severity, message, resolved FROM fault_logs ORDER BY id DESC LIMIT ?");
    query.addBindValue(limit > 0 ? limit : 100);

    if (!query.exec()) {
        lastError_ = query.lastError().text();
        return logs;
    }

    while (query.next()) {
        FaultLog log;
        log.id = query.value(0).toInt();
        log.timestamp = dateTimeFromString(query.value(1).toString());
        log.source = query.value(2).toString();
        log.severity = faultSeverityFromString(query.value(3).toString());
        log.message = query.value(4).toString();
        log.resolved = query.value(5).toInt() != 0;
        logs.push_back(log);
    }

    return logs;
}

QualitySummary SQLiteRepository::queryQualitySummary() const
{
    QualitySummary summary;
    if (!ensureReady(QStringLiteral("query quality summary"))) {
        return summary;
    }

    QSqlQuery query(database_);
    if (!query.exec("SELECT result, COUNT(*) FROM inspection_records GROUP BY result")) {
        lastError_ = query.lastError().text();
        return summary;
    }

    while (query.next()) {
        const InspectionDecision decision = decisionFromString(query.value(0).toString());
        const int count = query.value(1).toInt();
        summary.totalCount += count;
        if (decision == InspectionDecision::Pass) {
            summary.passCount += count;
        } else if (decision == InspectionDecision::Fail) {
            summary.failCount += count;
        }
    }

    summary.passRate = summary.totalCount > 0
        ? static_cast<double>(summary.passCount) / static_cast<double>(summary.totalCount)
        : 0.0;
    return summary;
}

QString SQLiteRepository::lastError() const
{
    return lastError_;
}

bool SQLiteRepository::isOpen() const
{
    return database_.isValid() && database_.isOpen();
}

PersistenceResult SQLiteRepository::executeSchema(const QString &sql)
{
    QSqlQuery query(database_);
    if (!query.exec(sql)) {
        lastError_ = query.lastError().text();
        return {false, lastError_};
    }
    return {true, QStringLiteral("Schema statement executed.")};
}

bool SQLiteRepository::ensureReady(const QString &operation) const
{
    if (!isOpen()) {
        lastError_ = QStringLiteral("Cannot %1 before opening database.").arg(operation);
        return false;
    }
    if (!initialized_) {
        lastError_ = QStringLiteral("Cannot %1 before initializing database.").arg(operation);
        return false;
    }
    return true;
}

void setProductIdFilter(InspectionHistoryFilter &filter, const QString &productId)
{
    copyFilterText(filter.productId, sizeof(filter.productId), productId);
}

void setTimestampRangeFilter(InspectionHistoryFilter &filter, const QString &fromTimestampIso, const QString &toTimestampIso)
{
    copyFilterText(filter.fromTimestampIso, sizeof(filter.fromTimestampIso), fromTimestampIso);
    copyFilterText(filter.toTimestampIso, sizeof(filter.toTimestampIso), toTimestampIso);
}

} // namespace workpiece
