#pragma once

#include "configuration/AppConfig.h"
#include "core/DomainModels.h"

#include <QImage>
#include <QString>
#include <string>

namespace workpiece {

enum class FrameRole {
    Positioning,
    Inspection
};

struct ImageFrameMetadata {
    int scenarioId = 0;
    QString productId;
    FrameRole role = FrameRole::Positioning;
    QString sourceType;
    PositionOffset offset;
    DefectType defectType = DefectType::Unknown;
    double defectScore = 0.0;
    bool hasDefect = false;
};

struct ImageFrame {
    QImage image;
    ImageFrameMetadata metadata;
};

struct ImageSourceStatus {
    ImageSourceState state = ImageSourceState::Closed;
    std::string message;
};

class IImageSource
{
public:
    virtual ~IImageSource() = default;

    virtual void open(const AppConfig &config) = 0;
    virtual void close() = 0;
    virtual ImageFrame capture(FrameRole role) = 0;
    virtual ImageSourceStatus status() const = 0;
};

QString toString(FrameRole role);

} // namespace workpiece
