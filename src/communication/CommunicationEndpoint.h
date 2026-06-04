#pragma once

#include "configuration/AppConfig.h"
#include "core/DomainModels.h"

#include <QString>

namespace workpiece {

struct CommunicationCommand {
    QString name;
};

struct CommunicationResult {
    bool success = false;
    CommunicationState state;
    QString message;
};

class ICommunicationEndpoint
{
public:
    virtual ~ICommunicationEndpoint() = default;

    virtual void configure(const AppConfig &config) = 0;
    virtual CommunicationResult connectEndpoint() = 0;
    virtual CommunicationResult disconnectEndpoint() = 0;
    virtual CommunicationResult poll() = 0;
    virtual CommunicationResult writeCommand(const CommunicationCommand &command) = 0;
    virtual bool isReady() const = 0;
    virtual CommunicationState state() const = 0;
};

} // namespace workpiece
