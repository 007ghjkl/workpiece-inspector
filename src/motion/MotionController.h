#pragma once

#include "configuration/AppConfig.h"
#include "core/DomainModels.h"

#include <QString>

namespace workpiece {

struct MotionCommand {
    double targetX = 0.0;
    double targetY = 0.0;
};

struct MotionCommandResult {
    bool success = false;
    MotionState state;
    QString message;
};

class IMotionController
{
public:
    virtual ~IMotionController() = default;

    virtual void configure(const AppConfig &config) = 0;
    virtual MotionCommandResult moveTo(const MotionCommand &command) = 0;
    virtual MotionCommandResult applyCorrection(const PositionOffset &offset) = 0;
    virtual void stop() = 0;
    virtual void resetFault() = 0;
    virtual MotionState state() const = 0;
};

} // namespace workpiece
