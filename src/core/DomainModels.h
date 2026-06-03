#pragma once

#include <string>

namespace workpiece {

enum class StationState {
    Idle,
    Running,
    Paused,
    Faulted,
    Completed
};

enum class ImageSourceState {
    Closed,
    Opening,
    Ready,
    Capturing,
    Faulted
};

enum class InspectionDecision {
    Unknown,
    Pass,
    Fail
};

enum class DefectType {
    None,
    Scratch,
    Spot,
    Deformation,
    Unknown
};

enum class MotionStatus {
    Idle,
    Moving,
    Completed,
    Faulted,
    Stopped
};

enum class CommunicationStatus {
    Disconnected,
    Connected,
    Polling,
    CommandInProgress,
    Faulted
};

enum class FaultSeverity {
    Info,
    Warning,
    Error
};

struct PositionOffset {
    double x = 0.0;
    double y = 0.0;
    double angle = 0.0;
};

struct InspectionResult {
    std::string productId;
    InspectionDecision decision = InspectionDecision::Unknown;
    DefectType defectType = DefectType::Unknown;
    double defectScore = 0.0;
    PositionOffset offset;
    double confidence = 0.0;
    std::string diagnosticSummary;
};

struct MotionState {
    double targetX = 0.0;
    double targetY = 0.0;
    double actualX = 0.0;
    double actualY = 0.0;
    double velocity = 0.0;
    MotionStatus status = MotionStatus::Idle;
};

struct CommunicationState {
    CommunicationStatus status = CommunicationStatus::Disconnected;
    bool polling = false;
    bool commandInProgress = false;
    std::string diagnosticSummary;
};

struct FaultInfo {
    std::string source;
    FaultSeverity severity = FaultSeverity::Info;
    std::string message;
};

std::string toString(StationState state);
std::string toString(ImageSourceState state);
std::string toString(InspectionDecision decision);
std::string toString(DefectType type);
std::string toString(MotionStatus status);
std::string toString(CommunicationStatus status);
std::string toString(FaultSeverity severity);

} // namespace workpiece
