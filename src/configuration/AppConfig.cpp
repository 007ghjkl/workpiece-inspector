#include "configuration/AppConfig.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>

namespace workpiece {
namespace {

void addDiagnostic(QVector<ConfigDiagnostic> *diagnostics,
                   FaultSeverity severity,
                   const QString &key,
                   const QString &message)
{
    diagnostics->push_back(ConfigDiagnostic{severity, key, message});
}

bool isProbability(double value)
{
    return value >= 0.0 && value <= 1.0;
}

bool isSupportedCameraUrl(const QString &url)
{
    const QUrl parsed(url);
    if (!parsed.isValid() || parsed.scheme().isEmpty()) {
        return false;
    }

    const QString scheme = parsed.scheme().toLower();
    return scheme == QStringLiteral("rtsp")
        || scheme == QStringLiteral("http")
        || scheme == QStringLiteral("https")
        || scheme == QStringLiteral("file");
}

double readDouble(const QJsonObject &object, const QString &key, double fallback)
{
    const QJsonValue value = object.value(key);
    return value.isDouble() ? value.toDouble() : fallback;
}

int readInt(const QJsonObject &object, const QString &key, int fallback)
{
    const QJsonValue value = object.value(key);
    return value.isDouble() ? value.toInt(fallback) : fallback;
}

bool readBool(const QJsonObject &object, const QString &key, bool fallback)
{
    const QJsonValue value = object.value(key);
    return value.isBool() ? value.toBool() : fallback;
}

QString readString(const QJsonObject &object, const QString &key, const QString &fallback)
{
    const QJsonValue value = object.value(key);
    return value.isString() ? value.toString() : fallback;
}

ImageSourceMode parseImageSourceMode(const QString &value, ImageSourceMode fallback)
{
    const QString normalized = value.trimmed().toLower();
    if (normalized == QStringLiteral("simulated")) {
        return ImageSourceMode::Simulated;
    }
    if (normalized == QStringLiteral("network_camera")
        || normalized == QStringLiteral("network-camera")
        || normalized == QStringLiteral("network")) {
        return ImageSourceMode::NetworkCamera;
    }

    return fallback;
}

} // namespace

AppConfig defaultAppConfig()
{
    return AppConfig{};
}

ConfigLoadResult validateAppConfig(const AppConfig &candidate)
{
    ConfigLoadResult result;
    result.config = candidate;

    const AppConfig defaults = defaultAppConfig();

    if (result.config.simulation.seed < 0) {
        result.config.simulation.seed = defaults.simulation.seed;
        addDiagnostic(&result.diagnostics,
                      FaultSeverity::Warning,
                      QStringLiteral("simulation.seed"),
                      QStringLiteral("Simulation seed must be non-negative; using default."));
    }

    if (!isProbability(result.config.simulation.defectProbability)) {
        result.config.simulation.defectProbability = defaults.simulation.defectProbability;
        addDiagnostic(&result.diagnostics,
                      FaultSeverity::Warning,
                      QStringLiteral("simulation.defectProbability"),
                      QStringLiteral("Defect probability must be between 0.0 and 1.0; using default."));
    }

    if (result.config.simulation.noiseLevel < 0.0) {
        result.config.simulation.noiseLevel = defaults.simulation.noiseLevel;
        addDiagnostic(&result.diagnostics,
                      FaultSeverity::Warning,
                      QStringLiteral("simulation.noiseLevel"),
                      QStringLiteral("Noise level must be non-negative; using default."));
    }

    if (result.config.inspection.defectScoreFailThreshold < 0.0) {
        result.config.inspection.defectScoreFailThreshold = defaults.inspection.defectScoreFailThreshold;
        addDiagnostic(&result.diagnostics,
                      FaultSeverity::Warning,
                      QStringLiteral("inspection.defectScoreFailThreshold"),
                      QStringLiteral("Defect score fail threshold must be non-negative; using default."));
    }

    if (!isProbability(result.config.inspection.minimumPositionConfidence)) {
        result.config.inspection.minimumPositionConfidence = defaults.inspection.minimumPositionConfidence;
        addDiagnostic(&result.diagnostics,
                      FaultSeverity::Warning,
                      QStringLiteral("inspection.minimumPositionConfidence"),
                      QStringLiteral("Minimum position confidence must be between 0.0 and 1.0; using default."));
    }

    if (result.config.imageSource.mode == ImageSourceMode::NetworkCamera
        && !isSupportedCameraUrl(result.config.imageSource.networkCameraUrl)) {
        result.config.imageSource.mode = ImageSourceMode::Simulated;
        result.config.imageSource.networkCameraUrl.clear();
        addDiagnostic(&result.diagnostics,
                      FaultSeverity::Warning,
                      QStringLiteral("imageSource.networkCameraUrl"),
                      QStringLiteral("Network camera URL is invalid; falling back to simulated image source."));
    }

    if (result.config.motion.maxVelocity <= 0.0) {
        result.config.motion.maxVelocity = defaults.motion.maxVelocity;
        addDiagnostic(&result.diagnostics,
                      FaultSeverity::Warning,
                      QStringLiteral("motion.maxVelocity"),
                      QStringLiteral("Motion max velocity must be positive; using default."));
    }

    if (result.config.motion.positionTolerance < 0.0) {
        result.config.motion.positionTolerance = defaults.motion.positionTolerance;
        addDiagnostic(&result.diagnostics,
                      FaultSeverity::Warning,
                      QStringLiteral("motion.positionTolerance"),
                      QStringLiteral("Motion position tolerance must be non-negative; using default."));
    }

    if (result.config.communication.minLatencyMs < 0) {
        result.config.communication.minLatencyMs = defaults.communication.minLatencyMs;
        addDiagnostic(&result.diagnostics,
                      FaultSeverity::Warning,
                      QStringLiteral("communication.minLatencyMs"),
                      QStringLiteral("Communication minimum latency must be non-negative; using default."));
    }

    if (result.config.communication.maxLatencyMs < result.config.communication.minLatencyMs) {
        result.config.communication.maxLatencyMs = defaults.communication.maxLatencyMs;
        addDiagnostic(&result.diagnostics,
                      FaultSeverity::Warning,
                      QStringLiteral("communication.maxLatencyMs"),
                      QStringLiteral("Communication maximum latency must be greater than or equal to minimum latency; using default."));
    }

    if (!isProbability(result.config.communication.faultProbability)) {
        result.config.communication.faultProbability = defaults.communication.faultProbability;
        addDiagnostic(&result.diagnostics,
                      FaultSeverity::Warning,
                      QStringLiteral("communication.faultProbability"),
                      QStringLiteral("Communication fault probability must be between 0.0 and 1.0; using default."));
    }

    if (result.config.persistence.databasePath.trimmed().isEmpty()) {
        result.config.persistence.databasePath = defaults.persistence.databasePath;
        addDiagnostic(&result.diagnostics,
                      FaultSeverity::Warning,
                      QStringLiteral("persistence.databasePath"),
                      QStringLiteral("Database path must be non-empty; using default."));
    }

    if (result.config.persistence.imageOutputDirectory.trimmed().isEmpty()) {
        result.config.persistence.imageOutputDirectory = defaults.persistence.imageOutputDirectory;
        addDiagnostic(&result.diagnostics,
                      FaultSeverity::Warning,
                      QStringLiteral("persistence.imageOutputDirectory"),
                      QStringLiteral("Image output directory must be non-empty; using default."));
    }

    return result;
}

ConfigLoadResult loadAppConfig(const QString &path)
{
    if (path.trimmed().isEmpty()) {
        return validateAppConfig(defaultAppConfig());
    }

    QFile file(path);
    if (!file.exists()) {
        ConfigLoadResult result = validateAppConfig(defaultAppConfig());
        addDiagnostic(&result.diagnostics,
                      FaultSeverity::Warning,
                      QStringLiteral("config.path"),
                      QStringLiteral("Configuration file does not exist; using defaults."));
        return result;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        ConfigLoadResult result = validateAppConfig(defaultAppConfig());
        addDiagnostic(&result.diagnostics,
                      FaultSeverity::Error,
                      QStringLiteral("config.path"),
                      QStringLiteral("Configuration file could not be opened; using defaults."));
        return result;
    }

    const QByteArray content = file.readAll();
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(content, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        ConfigLoadResult result = validateAppConfig(defaultAppConfig());
        addDiagnostic(&result.diagnostics,
                      FaultSeverity::Error,
                      QStringLiteral("config.json"),
                      QStringLiteral("Configuration JSON is invalid; using defaults."));
        return result;
    }

    AppConfig config = defaultAppConfig();
    const QJsonObject root = document.object();

    const QJsonObject simulation = root.value(QStringLiteral("simulation")).toObject();
    config.simulation.seed = readInt(simulation, QStringLiteral("seed"), config.simulation.seed);
    config.simulation.defectProbability = readDouble(simulation, QStringLiteral("defectProbability"), config.simulation.defectProbability);
    config.simulation.noiseLevel = readDouble(simulation, QStringLiteral("noiseLevel"), config.simulation.noiseLevel);

    const QJsonObject inspection = root.value(QStringLiteral("inspection")).toObject();
    config.inspection.defectScoreFailThreshold = readDouble(inspection, QStringLiteral("defectScoreFailThreshold"), config.inspection.defectScoreFailThreshold);
    config.inspection.minimumPositionConfidence = readDouble(inspection, QStringLiteral("minimumPositionConfidence"), config.inspection.minimumPositionConfidence);

    const QJsonObject imageSource = root.value(QStringLiteral("imageSource")).toObject();
    config.imageSource.mode = parseImageSourceMode(readString(imageSource, QStringLiteral("mode"), toString(config.imageSource.mode)), config.imageSource.mode);
    config.imageSource.networkCameraUrl = readString(imageSource, QStringLiteral("networkCameraUrl"), config.imageSource.networkCameraUrl);

    const QJsonObject motion = root.value(QStringLiteral("motion")).toObject();
    config.motion.maxVelocity = readDouble(motion, QStringLiteral("maxVelocity"), config.motion.maxVelocity);
    config.motion.positionTolerance = readDouble(motion, QStringLiteral("positionTolerance"), config.motion.positionTolerance);
    config.motion.faultEnabled = readBool(motion, QStringLiteral("faultEnabled"), config.motion.faultEnabled);

    const QJsonObject communication = root.value(QStringLiteral("communication")).toObject();
    config.communication.enabled = readBool(communication, QStringLiteral("enabled"), config.communication.enabled);
    config.communication.minLatencyMs = readInt(communication, QStringLiteral("minLatencyMs"), config.communication.minLatencyMs);
    config.communication.maxLatencyMs = readInt(communication, QStringLiteral("maxLatencyMs"), config.communication.maxLatencyMs);
    config.communication.faultProbability = readDouble(communication, QStringLiteral("faultProbability"), config.communication.faultProbability);

    const QJsonObject persistence = root.value(QStringLiteral("persistence")).toObject();
    config.persistence.databasePath = readString(persistence, QStringLiteral("databasePath"), config.persistence.databasePath);
    config.persistence.imageOutputDirectory = readString(persistence, QStringLiteral("imageOutputDirectory"), config.persistence.imageOutputDirectory);
    config.persistence.saveImages = readBool(persistence, QStringLiteral("saveImages"), config.persistence.saveImages);

    const QJsonObject ui = root.value(QStringLiteral("ui")).toObject();
    config.ui.showCharts = readBool(ui, QStringLiteral("showCharts"), config.ui.showCharts);

    ConfigLoadResult result = validateAppConfig(config);
    result.loadedFromFile = true;
    return result;
}

QString toString(ImageSourceMode mode)
{
    switch (mode) {
    case ImageSourceMode::Simulated:
        return QStringLiteral("simulated");
    case ImageSourceMode::NetworkCamera:
        return QStringLiteral("network_camera");
    }

    return QStringLiteral("unknown");
}

} // namespace workpiece
