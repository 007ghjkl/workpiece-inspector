#pragma once

#include "communication/CommunicationEndpoint.h"

namespace workpiece {

class SimulatedCommunicationEndpoint final : public ICommunicationEndpoint
{
public:
    void configure(const AppConfig &config) override;
    CommunicationResult connectEndpoint() override;
    CommunicationResult disconnectEndpoint() override;
    CommunicationResult poll() override;
    CommunicationResult writeCommand(const CommunicationCommand &command) override;
    bool isReady() const override;
    CommunicationState state() const override;

private:
    bool shouldFault() const;
    CommunicationResult result(bool success, const QString &message) const;
    CommunicationResult fault(const QString &message);

    AppConfig config_;
    CommunicationState state_;
};

} // namespace workpiece
