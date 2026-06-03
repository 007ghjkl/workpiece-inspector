#include "core/DomainModels.h"

namespace workpiece {

std::string toString(StationState state)
{
    switch (state) {
    case StationState::Idle:
        return "idle";
    case StationState::Running:
        return "running";
    case StationState::Paused:
        return "paused";
    case StationState::Faulted:
        return "faulted";
    case StationState::Completed:
        return "completed";
    }

    return "unknown";
}

std::string toString(ImageSourceState state)
{
    switch (state) {
    case ImageSourceState::Closed:
        return "closed";
    case ImageSourceState::Opening:
        return "opening";
    case ImageSourceState::Ready:
        return "ready";
    case ImageSourceState::Capturing:
        return "capturing";
    case ImageSourceState::Faulted:
        return "faulted";
    }

    return "unknown";
}

std::string toString(InspectionDecision decision)
{
    switch (decision) {
    case InspectionDecision::Unknown:
        return "unknown";
    case InspectionDecision::Pass:
        return "pass";
    case InspectionDecision::Fail:
        return "fail";
    }

    return "unknown";
}

std::string toString(DefectType type)
{
    switch (type) {
    case DefectType::None:
        return "none";
    case DefectType::Scratch:
        return "scratch";
    case DefectType::Spot:
        return "spot";
    case DefectType::Deformation:
        return "deformation";
    case DefectType::Unknown:
        return "unknown";
    }

    return "unknown";
}

std::string toString(MotionStatus status)
{
    switch (status) {
    case MotionStatus::Idle:
        return "idle";
    case MotionStatus::Moving:
        return "moving";
    case MotionStatus::Completed:
        return "completed";
    case MotionStatus::Faulted:
        return "faulted";
    case MotionStatus::Stopped:
        return "stopped";
    }

    return "unknown";
}

std::string toString(CommunicationStatus status)
{
    switch (status) {
    case CommunicationStatus::Disconnected:
        return "disconnected";
    case CommunicationStatus::Connected:
        return "connected";
    case CommunicationStatus::Polling:
        return "polling";
    case CommunicationStatus::CommandInProgress:
        return "command_in_progress";
    case CommunicationStatus::Faulted:
        return "faulted";
    }

    return "unknown";
}

std::string toString(FaultSeverity severity)
{
    switch (severity) {
    case FaultSeverity::Info:
        return "info";
    case FaultSeverity::Warning:
        return "warning";
    case FaultSeverity::Error:
        return "error";
    }

    return "unknown";
}

} // namespace workpiece
