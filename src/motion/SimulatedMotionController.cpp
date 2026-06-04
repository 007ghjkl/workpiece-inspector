#include "motion/SimulatedMotionController.h"

namespace workpiece {

void SimulatedMotionController::configure(const AppConfig &config)
{
    config_ = config;
    state_ = MotionState{};
}

MotionCommandResult SimulatedMotionController::moveTo(const MotionCommand &command)
{
    if (config_.motion.faultEnabled) {
        return faultResult(QStringLiteral("Simulated motion fault is enabled."));
    }

    state_.targetX = command.targetX;
    state_.targetY = command.targetY;
    state_.status = MotionStatus::Moving;
    state_.velocity = config_.motion.maxVelocity;

    state_.actualX = state_.targetX + config_.motion.positionTolerance;
    state_.actualY = state_.targetY + config_.motion.positionTolerance;
    state_.status = MotionStatus::Completed;

    return MotionCommandResult{true, state_, QStringLiteral("Simulated motion completed.")};
}

MotionCommandResult SimulatedMotionController::applyCorrection(const PositionOffset &offset)
{
    return moveTo(MotionCommand{-offset.x, -offset.y});
}

void SimulatedMotionController::stop()
{
    state_.velocity = 0.0;
    state_.status = MotionStatus::Stopped;
}

void SimulatedMotionController::resetFault()
{
    if (state_.status == MotionStatus::Faulted) {
        state_.velocity = 0.0;
        state_.status = MotionStatus::Idle;
    }
}

MotionState SimulatedMotionController::state() const
{
    return state_;
}

MotionCommandResult SimulatedMotionController::faultResult(const QString &message)
{
    state_.velocity = 0.0;
    state_.status = MotionStatus::Faulted;
    return MotionCommandResult{false, state_, message};
}

} // namespace workpiece
