#pragma once

#include "vision/ImageSource.h"

#include <random>

namespace workpiece {

class SimulatedImageSource final : public IImageSource
{
public:
    void open(const AppConfig &config) override;
    void close() override;
    ImageFrame capture(FrameRole role) override;
    ImageSourceStatus status() const override;

private:
    struct Scenario {
        int id = 0;
        PositionOffset offset;
        DefectType defectType = DefectType::None;
        double defectScore = 0.0;
        bool hasDefect = false;
    };

    Scenario createScenario();
    QImage renderFrame(const Scenario &scenario, FrameRole role);
    void drawDefect(QPainter *painter, const Scenario &scenario);
    QString productIdForScenario(int scenarioId) const;

    SimulationConfig simulationConfig_;
    ImageSourceState state_ = ImageSourceState::Closed;
    std::mt19937 randomEngine_;
    int nextScenarioId_ = 1;
    Scenario currentScenario_;
    bool hasCurrentScenario_ = false;
};

} // namespace workpiece
