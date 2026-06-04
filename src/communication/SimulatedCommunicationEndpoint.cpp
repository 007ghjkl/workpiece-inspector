#include "communication/SimulatedCommunicationEndpoint.h"

namespace workpiece {

void SimulatedCommunicationEndpoint::configure(const AppConfig &config)
{
    config_ = config;
    state_ = CommunicationState{};
}

CommunicationResult SimulatedCommunicationEndpoint::connectEndpoint()
{
    if (!config_.communication.enabled) {
        return fault(QStringLiteral("Simulated communication is disabled."));
    }

    if (shouldFault()) {
        return fault(QStringLiteral("Simulated communication fault during connect."));
    }

    state_.status = CommunicationStatus::Connected;
    state_.polling = false;
    state_.commandInProgress = false;
    state_.diagnosticSummary = "Simulated communication connected.";
    return result(true, QStringLiteral("Simulated communication connected."));
}

CommunicationResult SimulatedCommunicationEndpoint::disconnectEndpoint()
{
    state_.status = CommunicationStatus::Disconnected;
    state_.polling = false;
    state_.commandInProgress = false;
    state_.diagnosticSummary = "Simulated communication disconnected.";
    return result(true, QStringLiteral("Simulated communication disconnected."));
}

CommunicationResult SimulatedCommunicationEndpoint::poll()
{
    if (!isReady()) {
        return fault(QStringLiteral("Simulated communication is not ready for polling."));
    }

    if (shouldFault()) {
        return fault(QStringLiteral("Simulated communication fault during polling."));
    }

    state_.status = CommunicationStatus::Polling;
    state_.polling = true;
    state_.commandInProgress = false;

    state_.status = CommunicationStatus::Connected;
    state_.polling = false;
    state_.diagnosticSummary = QStringLiteral("Simulated poll completed with latency range %1-%2 ms.")
        .arg(config_.communication.minLatencyMs)
        .arg(config_.communication.maxLatencyMs)
        .toStdString();
    return result(true, QStringLiteral("Simulated poll completed."));
}

CommunicationResult SimulatedCommunicationEndpoint::writeCommand(const CommunicationCommand &command)
{
    if (!isReady()) {
        return fault(QStringLiteral("Simulated communication is not ready for commands."));
    }

    if (command.name.trimmed().isEmpty()) {
        return fault(QStringLiteral("Communication command name is empty."));
    }

    if (shouldFault()) {
        return fault(QStringLiteral("Simulated communication fault during command."));
    }

    state_.status = CommunicationStatus::CommandInProgress;
    state_.polling = false;
    state_.commandInProgress = true;

    state_.status = CommunicationStatus::Connected;
    state_.commandInProgress = false;
    state_.diagnosticSummary = QStringLiteral("Simulated command completed: %1.")
        .arg(command.name.trimmed())
        .toStdString();
    return result(true, QStringLiteral("Simulated command completed."));
}

bool SimulatedCommunicationEndpoint::isReady() const
{
    return state_.status == CommunicationStatus::Connected
        && !state_.polling
        && !state_.commandInProgress;
}

CommunicationState SimulatedCommunicationEndpoint::state() const
{
    return state_;
}

bool SimulatedCommunicationEndpoint::shouldFault() const
{
    return config_.communication.faultProbability >= 1.0;
}

CommunicationResult SimulatedCommunicationEndpoint::result(bool success, const QString &message) const
{
    return CommunicationResult{success, state_, message};
}

CommunicationResult SimulatedCommunicationEndpoint::fault(const QString &message)
{
    state_.status = CommunicationStatus::Faulted;
    state_.polling = false;
    state_.commandInProgress = false;
    state_.diagnosticSummary = message.toStdString();
    return result(false, message);
}

} // namespace workpiece
