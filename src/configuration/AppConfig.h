#pragma once

#include "core/DomainModels.h"

#include <QString>
#include <QVector>

namespace workpiece {

enum class ImageSourceMode {
    Simulated,
    NetworkCamera
};

struct SimulationConfig {
    int seed = 12345;
    double defectProbability = 0.20;
    double noiseLevel = 0.0;
};

struct InspectionThresholdConfig {
    double defectScoreFailThreshold = 50.0;
    double minimumPositionConfidence = 0.60;
};

struct ImageSourceConfig {
    ImageSourceMode mode = ImageSourceMode::Simulated;
    QString networkCameraUrl;
};

struct MotionSimulationConfig {
    double maxVelocity = 100.0;
    double positionTolerance = 0.01;
};

struct CommunicationSimulationConfig {
    bool enabled = true;
    int minLatencyMs = 5;
    int maxLatencyMs = 50;
    double faultProbability = 0.0;
};

struct PersistenceConfig {
    QString databasePath = QStringLiteral("workpiece_inspector.sqlite");
    QString imageOutputDirectory = QStringLiteral("inspection_images");
    bool saveImages = true;
};

struct UiConfig {
    bool showCharts = true;
};

struct AppConfig {
    SimulationConfig simulation;
    InspectionThresholdConfig inspection;
    ImageSourceConfig imageSource;
    MotionSimulationConfig motion;
    CommunicationSimulationConfig communication;
    PersistenceConfig persistence;
    UiConfig ui;
};

struct ConfigDiagnostic {
    FaultSeverity severity = FaultSeverity::Info;
    QString key;
    QString message;
};

struct ConfigLoadResult {
    AppConfig config;
    QVector<ConfigDiagnostic> diagnostics;
    bool loadedFromFile = false;
};

AppConfig defaultAppConfig();
ConfigLoadResult loadAppConfig(const QString &path = QString());
ConfigLoadResult validateAppConfig(const AppConfig &candidate);
QString toString(ImageSourceMode mode);

} // namespace workpiece
