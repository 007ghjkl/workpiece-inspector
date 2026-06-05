#pragma once

#include "communication/SimulatedCommunicationEndpoint.h"
#include "configuration/AppConfig.h"
#include "motion/SimulatedMotionController.h"
#include "persistence/SQLiteRepository.h"
#include "storage/ImageStorage.h"
#include "vision/RuleBasedInspector.h"
#include "vision/SimulatedImageSource.h"
#include "workflow/WorkflowController.h"

#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QString>

#include <memory>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    explicit MainWindow(const workpiece::AppConfig &config, QWidget *parent = nullptr);

private slots:
    void runSingleCycle();

private:
    void buildUi();
    void initializeRuntime();
    void renderSnapshot(const workpiece::WorkflowSnapshot &snapshot);
    void renderResult(const workpiece::WorkflowRunResult &result);
    void setMessage(const QString &message);
    void updateImage(const QImage &image);
    void setCycleControlsEnabled(bool enabled);

    workpiece::AppConfig config_;
    workpiece::SimulatedImageSource imageSource_;
    workpiece::RuleBasedInspector inspector_;
    workpiece::SimulatedMotionController motionController_;
    workpiece::SimulatedCommunicationEndpoint communicationEndpoint_;
    workpiece::ImageStorage imageStorage_;
    workpiece::SQLiteRepository repository_;
    std::unique_ptr<workpiece::WorkflowController> workflow_;

    QPushButton *startButton_ = nullptr;
    QLabel *modeLabel_ = nullptr;
    QLabel *stateLabel_ = nullptr;
    QLabel *messageLabel_ = nullptr;
    QLabel *imageLabel_ = nullptr;
    QLabel *decisionLabel_ = nullptr;
    QLabel *productLabel_ = nullptr;
    QLabel *defectLabel_ = nullptr;
    QLabel *scoreLabel_ = nullptr;
    QLabel *confidenceLabel_ = nullptr;
    QLabel *offsetLabel_ = nullptr;
    QLabel *motionLabel_ = nullptr;
    QLabel *communicationLabel_ = nullptr;
};
