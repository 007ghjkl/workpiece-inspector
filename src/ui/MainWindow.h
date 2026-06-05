#pragma once

#include "configuration/AppConfig.h"
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
    ~MainWindow() override;

private slots:
    void runSingleCycle();

private:
    struct Runtime;

    void buildUi();
    bool initializeRuntime();
    void renderSnapshot(const workpiece::WorkflowSnapshot &snapshot);
    void renderResult(const workpiece::WorkflowRunResult &result);
    void setMessage(const QString &message);
    void updateImage(const QImage &image);
    void setCycleControlsEnabled(bool enabled);

    workpiece::AppConfig config_;
    std::unique_ptr<Runtime> runtime_;

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
