#include "vision/RuleBasedInspector.h"

namespace workpiece {

PositioningAnalysis RuleBasedInspector::analyzePositioning(const ImageFrame &frame, const AppConfig &config) const
{
    PositioningAnalysis analysis;

    if (frame.image.isNull()) {
        analysis.diagnosticSummary = QStringLiteral("Positioning frame is empty.");
        return analysis;
    }

    if (frame.metadata.role != FrameRole::Positioning) {
        analysis.diagnosticSummary = QStringLiteral("Frame role is not positioning.");
        return analysis;
    }

    analysis.offset = frame.metadata.offset;
    analysis.confidence = 1.0;
    analysis.valid = analysis.confidence >= config.inspection.minimumPositionConfidence;
    analysis.diagnosticSummary = analysis.valid
        ? QStringLiteral("Positioning analysis completed.")
        : QStringLiteral("Positioning confidence is below threshold.");

    return analysis;
}

InspectionAnalysis RuleBasedInspector::inspect(const ImageFrame &frame, const AppConfig &config) const
{
    InspectionAnalysis analysis;

    if (frame.image.isNull()) {
        analysis.diagnosticSummary = QStringLiteral("Inspection frame is empty.");
        return analysis;
    }

    if (frame.metadata.role != FrameRole::Inspection) {
        analysis.diagnosticSummary = QStringLiteral("Frame role is not inspection.");
        return analysis;
    }

    InspectionResult result;
    result.productId = frame.metadata.productId.toStdString();
    result.offset = frame.metadata.offset;
    result.confidence = 1.0;

    if (!frame.metadata.hasDefect) {
        result.decision = InspectionDecision::Pass;
        result.defectType = DefectType::None;
        result.defectScore = 0.0;
        result.diagnosticSummary = "No simulated defect detected.";
    } else {
        result.defectType = frame.metadata.defectType;
        result.defectScore = frame.metadata.defectScore;
        result.decision = result.defectScore >= config.inspection.defectScoreFailThreshold
            ? InspectionDecision::Fail
            : InspectionDecision::Pass;
        result.diagnosticSummary = result.decision == InspectionDecision::Fail
            ? "Simulated defect score exceeds fail threshold."
            : "Simulated defect score is below fail threshold.";
    }

    analysis.result = result;
    analysis.valid = true;
    analysis.diagnosticSummary = QString::fromStdString(result.diagnosticSummary);
    return analysis;
}

} // namespace workpiece
