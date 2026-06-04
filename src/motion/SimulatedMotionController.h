#pragma once

#include "motion/MotionController.h"

namespace workpiece {

class SimulatedMotionController final : public IMotionController
{
public:
    void configure(const AppConfig &config) override;
    MotionCommandResult moveTo(const MotionCommand &command) override;
    MotionCommandResult applyCorrection(const PositionOffset &offset) override;
    void stop() override;
    void resetFault() override;
    MotionState state() const override;

private:
    MotionCommandResult faultResult(const QString &message);

    AppConfig config_;
    MotionState state_;
};

} // namespace workpiece
