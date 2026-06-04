#pragma once

#include "vision/ImageSource.h"

#include <optional>
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
        QString productId;
        PositionOffset offset;
        DefectType defectType = DefectType::None;
        double defectScore = 0.0;
        bool hasDefect = false;
    };

    Scenario createScenario();
    QImage renderFrame(const Scenario &scenario, FrameRole role);
    void drawDefect(QImage *image, const Scenario &scenario);

    SimulationConfig simulationConfig_;
    ImageSourceStatus status_;
    std::mt19937 randomEngine_;
    int nextScenarioId_ = 1;
    std::optional<Scenario> currentScenario_;
};

} // namespace workpiece
