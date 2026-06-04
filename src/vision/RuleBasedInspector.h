#pragma once

#include "vision/ImageSource.h"

namespace workpiece {

struct PositioningAnalysis {
    PositionOffset offset;
    double confidence = 0.0;
    bool valid = false;
    QString diagnosticSummary;
};

struct InspectionAnalysis {
    InspectionResult result;
    bool valid = false;
    QString diagnosticSummary;
};

class RuleBasedInspector
{
public:
    PositioningAnalysis analyzePositioning(const ImageFrame &frame, const AppConfig &config) const;
    InspectionAnalysis inspect(const ImageFrame &frame, const AppConfig &config) const;
};

} // namespace workpiece
