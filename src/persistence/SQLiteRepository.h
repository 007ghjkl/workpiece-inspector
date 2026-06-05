#pragma once

#include "core/DomainModels.h"

#include <QDateTime>
#include <QList>
#include <QSqlDatabase>
#include <QString>

namespace workpiece {

struct PersistenceResult {
    bool success = false;
    QString message;
};

struct InspectionRecord {
    int id = 0;
    QDateTime timestamp;
    QString productId;
    QString sourceType;
    QString imageRef;
    InspectionDecision result = InspectionDecision::Unknown;
    DefectType defectType = DefectType::Unknown;
    double defectScore = 0.0;
    PositionOffset offset;
    MotionState motionState;
    CommunicationState communicationState;
    int cycleTimeMs = 0;
    QString errorMessage;
};

struct OperatorLog {
    int id = 0;
    QDateTime timestamp;
    QString action;
    QString details;
};

struct FaultLog {
    int id = 0;
    QDateTime timestamp;
    QString source;
    FaultSeverity severity = FaultSeverity::Info;
    QString message;
    bool resolved = false;
};

struct InspectionHistoryFilter {
    InspectionHistoryFilter();

    char productId[128] = {};
    InspectionDecision result = InspectionDecision::Unknown;
    char fromTimestampIso[32] = {};
    char toTimestampIso[32] = {};
    int limit = 100;
};

struct QualitySummary {
    int totalCount = 0;
    int passCount = 0;
    int failCount = 0;
    double passRate = 0.0;
};

class SQLiteRepository
{
public:
    SQLiteRepository();
    ~SQLiteRepository();

    PersistenceResult open(const QString &databasePath);
    void close();
    PersistenceResult initialize();

    PersistenceResult saveInspectionRecord(const InspectionRecord &record);
    PersistenceResult saveOperatorLog(const OperatorLog &log);
    PersistenceResult saveFaultLog(const FaultLog &log);

    QList<InspectionRecord> queryInspectionHistory() const;
    QList<InspectionRecord> queryInspectionHistory(const InspectionHistoryFilter &filter) const;
    QList<OperatorLog> queryOperatorLogs(int limit = 100) const;
    QList<FaultLog> queryFaultLogs(int limit = 100) const;
    QualitySummary queryQualitySummary() const;

    QString lastError() const;
    bool isOpen() const;

private:
    PersistenceResult executeSchema(const QString &sql);
    bool ensureReady(const QString &operation) const;

    QString connectionName_;
    QSqlDatabase database_;
    mutable QString lastError_;
    bool initialized_ = false;
};

void setProductIdFilter(InspectionHistoryFilter &filter, const QString &productId);
void setTimestampRangeFilter(InspectionHistoryFilter &filter, const QString &fromTimestampIso, const QString &toTimestampIso);

} // namespace workpiece
