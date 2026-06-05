#pragma once

#include "configuration/AppConfig.h"
#include "workflow/WorkflowController.h"

#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>
#include <QComboBox>
#include <QString>
#include <QTableWidget>

QT_BEGIN_NAMESPACE
class QChartView;
QT_END_NAMESPACE

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
    void applyHistoryFilters();

private:
    struct Runtime;

    void buildUi();
    bool initializeRuntime();
    void renderSnapshot(const workpiece::WorkflowSnapshot &snapshot);
    void renderResult(const workpiece::WorkflowRunResult &result);
    void refreshHistory();
    void renderHistory(const QList<workpiece::InspectionRecord> &records);
    void renderSummary(const workpiece::QualitySummary &summary);
    workpiece::InspectionHistoryFilter historyFilter() const;
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
    QLineEdit *productFilterEdit_ = nullptr;
    QComboBox *resultFilterCombo_ = nullptr;
    QPushButton *applyFilterButton_ = nullptr;
    QTableWidget *historyTable_ = nullptr;
    QLabel *historyEmptyLabel_ = nullptr;
    QLabel *totalCountLabel_ = nullptr;
    QLabel *passCountLabel_ = nullptr;
    QLabel *failCountLabel_ = nullptr;
    QLabel *passRateLabel_ = nullptr;
    QChartView *qualityChartView_ = nullptr;
};
