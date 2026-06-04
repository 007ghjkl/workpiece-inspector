#include "vision/SimulatedImageSource.h"

#include <QColor>
#include <QPainter>
#include <QtMath>

namespace workpiece {
namespace {

constexpr int kImageWidth = 640;
constexpr int kImageHeight = 480;

QPointF workpieceCenterWithOffset(const PositionOffset &offset)
{
    return QPointF(kImageWidth / 2.0 + offset.x, kImageHeight / 2.0 + offset.y);
}

} // namespace

QString toString(FrameRole role)
{
    switch (role) {
    case FrameRole::Positioning:
        return QStringLiteral("positioning");
    case FrameRole::Inspection:
        return QStringLiteral("inspection");
    }

    return QStringLiteral("unknown");
}

void SimulatedImageSource::open(const AppConfig &config)
{
    simulationConfig_ = config.simulation;
    randomEngine_.seed(static_cast<std::mt19937::result_type>(simulationConfig_.seed));
    nextScenarioId_ = 1;
    currentScenario_ = Scenario{};
    hasCurrentScenario_ = false;
    state_ = ImageSourceState::Ready;
}

void SimulatedImageSource::close()
{
    currentScenario_ = Scenario{};
    hasCurrentScenario_ = false;
    state_ = ImageSourceState::Closed;
}

ImageFrame SimulatedImageSource::capture(FrameRole role)
{
    if (state_ != ImageSourceState::Ready) {
        state_ = ImageSourceState::Faulted;
        ImageFrame frame;
        frame.metadata.role = role;
        frame.metadata.sourceType = QStringLiteral("simulated");
        return frame;
    }

    state_ = ImageSourceState::Capturing;

    if (role == FrameRole::Positioning || !hasCurrentScenario_) {
        currentScenario_ = createScenario();
        hasCurrentScenario_ = true;
    }

    ImageFrame frame;
    frame.image = renderFrame(currentScenario_, role);
    frame.metadata.scenarioId = currentScenario_.id;
    frame.metadata.productId = productIdForScenario(currentScenario_.id);
    frame.metadata.role = role;
    frame.metadata.sourceType = QStringLiteral("simulated");
    frame.metadata.offset = currentScenario_.offset;
    frame.metadata.defectType = currentScenario_.defectType;
    frame.metadata.defectScore = currentScenario_.defectScore;
    frame.metadata.hasDefect = currentScenario_.hasDefect;

    state_ = ImageSourceState::Ready;
    return frame;
}

ImageSourceStatus SimulatedImageSource::status() const
{
    switch (state_) {
    case ImageSourceState::Closed:
        return ImageSourceStatus{state_, "Simulated image source closed."};
    case ImageSourceState::Opening:
        return ImageSourceStatus{state_, "Simulated image source opening."};
    case ImageSourceState::Ready:
        return ImageSourceStatus{state_, "Simulated image source ready."};
    case ImageSourceState::Capturing:
        return ImageSourceStatus{state_, "Capturing simulated frame."};
    case ImageSourceState::Faulted:
        return ImageSourceStatus{state_, "Simulated image source is not open."};
    }

    return ImageSourceStatus{ImageSourceState::Faulted, "Simulated image source status is unknown."};
}

SimulatedImageSource::Scenario SimulatedImageSource::createScenario()
{
    std::uniform_real_distribution<double> offsetDistribution(-18.0, 18.0);
    std::uniform_real_distribution<double> angleDistribution(-4.0, 4.0);
    std::uniform_real_distribution<double> probabilityDistribution(0.0, 1.0);
    std::uniform_int_distribution<int> defectTypeDistribution(0, 2);
    std::uniform_real_distribution<double> defectScoreDistribution(55.0, 95.0);

    Scenario scenario;
    scenario.id = nextScenarioId_++;
    scenario.offset.x = offsetDistribution(randomEngine_);
    scenario.offset.y = offsetDistribution(randomEngine_);
    scenario.offset.angle = angleDistribution(randomEngine_);
    scenario.hasDefect = probabilityDistribution(randomEngine_) < simulationConfig_.defectProbability;

    if (scenario.hasDefect) {
        switch (defectTypeDistribution(randomEngine_)) {
        case 0:
            scenario.defectType = DefectType::Scratch;
            break;
        case 1:
            scenario.defectType = DefectType::Spot;
            break;
        default:
            scenario.defectType = DefectType::Deformation;
            break;
        }
        scenario.defectScore = defectScoreDistribution(randomEngine_);
    } else {
        scenario.defectType = DefectType::None;
        scenario.defectScore = 0.0;
    }

    return scenario;
}

QImage SimulatedImageSource::renderFrame(const Scenario &scenario, FrameRole role)
{
    QImage image(kImageWidth, kImageHeight, QImage::Format_RGB32);
    image.fill(QColor(236, 239, 241));

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.setPen(QPen(QColor(190, 197, 204), 1));
    for (int x = 0; x < kImageWidth; x += 40) {
        painter.drawLine(x, 0, x, kImageHeight);
    }
    for (int y = 0; y < kImageHeight; y += 40) {
        painter.drawLine(0, y, kImageWidth, y);
    }

    painter.save();
    painter.translate(workpieceCenterWithOffset(scenario.offset));
    painter.rotate(scenario.offset.angle);

    const QRectF workpieceRect(-130.0, -75.0, 260.0, 150.0);
    painter.setPen(QPen(QColor(57, 73, 89), 4));
    painter.setBrush(QColor(120, 144, 156));
    painter.drawRoundedRect(workpieceRect, 10.0, 10.0);

    painter.setPen(QPen(QColor(84, 110, 122), 2));
    painter.drawLine(QPointF(-105.0, 0.0), QPointF(105.0, 0.0));
    painter.drawLine(QPointF(0.0, -55.0), QPointF(0.0, 55.0));

    if (role == FrameRole::Inspection && scenario.hasDefect) {
        painter.end();
        drawDefect(&image, scenario);
        painter.begin(&image);
    }

    painter.restore();

    if (simulationConfig_.noiseLevel > 0.0) {
        std::mt19937 noiseEngine(static_cast<std::mt19937::result_type>(
            simulationConfig_.seed + scenario.id * 97 + (role == FrameRole::Inspection ? 17 : 3)));
        std::uniform_int_distribution<int> noiseDistribution(
            -static_cast<int>(simulationConfig_.noiseLevel),
            static_cast<int>(simulationConfig_.noiseLevel));

        for (int y = 0; y < image.height(); ++y) {
            auto *scanLine = reinterpret_cast<QRgb *>(image.scanLine(y));
            for (int x = 0; x < image.width(); ++x) {
                const QColor color(scanLine[x]);
                const int noise = noiseDistribution(noiseEngine);
                scanLine[x] = qRgb(qBound(0, color.red() + noise, 255),
                                   qBound(0, color.green() + noise, 255),
                                   qBound(0, color.blue() + noise, 255));
            }
        }
    }

    return image;
}

QString SimulatedImageSource::productIdForScenario(int scenarioId) const
{
    return QStringLiteral("SIM-%1").arg(scenarioId, 6, 10, QLatin1Char('0'));
}

void SimulatedImageSource::drawDefect(QImage *image, const Scenario &scenario)
{
    QPainter painter(image);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.translate(workpieceCenterWithOffset(scenario.offset));
    painter.rotate(scenario.offset.angle);

    if (scenario.defectType == DefectType::Scratch) {
        painter.setPen(QPen(QColor(33, 33, 33), 5));
        painter.drawLine(QPointF(-75.0, -25.0), QPointF(85.0, 30.0));
        return;
    }

    if (scenario.defectType == DefectType::Spot) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(69, 90, 100));
        painter.drawEllipse(QPointF(45.0, -18.0), 22.0, 22.0);
        return;
    }

    if (scenario.defectType == DefectType::Deformation) {
        painter.setPen(QPen(QColor(176, 54, 54), 4));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(QPointF(-45.0, 20.0), 36.0, 24.0);
    }
}

} // namespace workpiece
