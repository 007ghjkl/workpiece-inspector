#include "ui/MainWindow.h"

#include <QApplication>
#include <QDateTime>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QPixmap>
#include <QSizePolicy>
#include <QVBoxLayout>

namespace {

QString stationStateText(workpiece::StationState state)
{
    return QString::fromStdString(workpiece::toString(state));
}

QString decisionText(workpiece::InspectionDecision decision)
{
    return QString::fromStdString(workpiece::toString(decision));
}

QString defectText(workpiece::DefectType type)
{
    return QString::fromStdString(workpiece::toString(type));
}

QString motionText(const workpiece::MotionState &state)
{
    return QStringLiteral("%1  target(%2, %3)  actual(%4, %5)")
        .arg(QString::fromStdString(workpiece::toString(state.status)))
        .arg(state.targetX, 0, 'f', 2)
        .arg(state.targetY, 0, 'f', 2)
        .arg(state.actualX, 0, 'f', 2)
        .arg(state.actualY, 0, 'f', 2);
}

QString communicationText(const workpiece::CommunicationState &state)
{
    return QStringLiteral("%1  %2")
        .arg(QString::fromStdString(workpiece::toString(state.status)))
        .arg(QString::fromStdString(state.diagnosticSummary));
}

QLabel *makeValueLabel(const QString &objectName)
{
    auto *label = new QLabel;
    label->setObjectName(objectName);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    label->setWordWrap(true);
    return label;
}

void addRow(QGridLayout *layout, int row, const QString &name, QLabel *value)
{
    auto *nameLabel = new QLabel(name);
    nameLabel->setAlignment(Qt::AlignTop | Qt::AlignRight);
    layout->addWidget(nameLabel, row, 0);
    layout->addWidget(value, row, 1);
}

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : MainWindow(workpiece::defaultAppConfig(), parent)
{
}

MainWindow::MainWindow(const workpiece::AppConfig &config, QWidget *parent)
    : QMainWindow(parent)
    , config_(config)
{
    setWindowTitle(QStringLiteral("Workpiece Inspector"));
    resize(1180, 760);

    buildUi();
    initializeRuntime();
}

void MainWindow::runSingleCycle()
{
    if (!workflow_) {
        setMessage(QStringLiteral("Workflow is not available."));
        return;
    }

    setCycleControlsEnabled(false);
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

    const workpiece::WorkflowRunResult result = workflow_->runSingleCycle();
    renderResult(result);

    setCycleControlsEnabled(true);
}

void MainWindow::buildUi()
{
    auto *central = new QWidget(this);
    auto *root = new QVBoxLayout(central);
    root->setContentsMargins(14, 14, 14, 14);
    root->setSpacing(10);

    auto *topBar = new QHBoxLayout;
    startButton_ = new QPushButton(QStringLiteral("Start Cycle"));
    startButton_->setObjectName(QStringLiteral("startCycleButton"));
    connect(startButton_, &QPushButton::clicked, this, &MainWindow::runSingleCycle);

    modeLabel_ = makeValueLabel(QStringLiteral("modeLabel"));
    modeLabel_->setText(QStringLiteral("SIMULATION MODE"));
    modeLabel_->setAlignment(Qt::AlignCenter);
    modeLabel_->setStyleSheet(QStringLiteral("font-weight: 700; color: #0f5132; background: #d1e7dd; padding: 6px 10px; border: 1px solid #badbcc;"));

    stateLabel_ = makeValueLabel(QStringLiteral("stationStateLabel"));
    stateLabel_->setText(stationStateText(workpiece::StationState::Idle));
    stateLabel_->setStyleSheet(QStringLiteral("font-weight: 700;"));

    topBar->addWidget(startButton_);
    topBar->addWidget(modeLabel_);
    topBar->addStretch();
    topBar->addWidget(new QLabel(QStringLiteral("Station")));
    topBar->addWidget(stateLabel_);
    root->addLayout(topBar);

    auto *content = new QHBoxLayout;
    content->setSpacing(12);

    imageLabel_ = new QLabel;
    imageLabel_->setObjectName(QStringLiteral("currentImageLabel"));
    imageLabel_->setMinimumSize(640, 480);
    imageLabel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    imageLabel_->setAlignment(Qt::AlignCenter);
    imageLabel_->setFrameShape(QFrame::StyledPanel);
    imageLabel_->setText(QStringLiteral("No image"));
    content->addWidget(imageLabel_, 3);

    auto *sidePanel = new QWidget;
    auto *sideLayout = new QVBoxLayout(sidePanel);
    sideLayout->setContentsMargins(0, 0, 0, 0);
    sideLayout->setSpacing(10);

    auto *resultBox = new QGroupBox(QStringLiteral("Latest Result"));
    auto *resultGrid = new QGridLayout(resultBox);
    decisionLabel_ = makeValueLabel(QStringLiteral("decisionLabel"));
    productLabel_ = makeValueLabel(QStringLiteral("productLabel"));
    defectLabel_ = makeValueLabel(QStringLiteral("defectLabel"));
    scoreLabel_ = makeValueLabel(QStringLiteral("scoreLabel"));
    confidenceLabel_ = makeValueLabel(QStringLiteral("confidenceLabel"));
    addRow(resultGrid, 0, QStringLiteral("Decision"), decisionLabel_);
    addRow(resultGrid, 1, QStringLiteral("Product"), productLabel_);
    addRow(resultGrid, 2, QStringLiteral("Defect"), defectLabel_);
    addRow(resultGrid, 3, QStringLiteral("Score"), scoreLabel_);
    addRow(resultGrid, 4, QStringLiteral("Confidence"), confidenceLabel_);
    sideLayout->addWidget(resultBox);

    auto *deviceBox = new QGroupBox(QStringLiteral("Cell State"));
    auto *deviceGrid = new QGridLayout(deviceBox);
    offsetLabel_ = makeValueLabel(QStringLiteral("offsetLabel"));
    motionLabel_ = makeValueLabel(QStringLiteral("motionLabel"));
    communicationLabel_ = makeValueLabel(QStringLiteral("communicationLabel"));
    addRow(deviceGrid, 0, QStringLiteral("Offset"), offsetLabel_);
    addRow(deviceGrid, 1, QStringLiteral("Motion"), motionLabel_);
    addRow(deviceGrid, 2, QStringLiteral("Communication"), communicationLabel_);
    sideLayout->addWidget(deviceBox);

    sideLayout->addStretch();
    content->addWidget(sidePanel, 2);
    root->addLayout(content, 1);

    messageLabel_ = makeValueLabel(QStringLiteral("messageLabel"));
    messageLabel_->setFrameShape(QFrame::StyledPanel);
    messageLabel_->setMinimumHeight(44);
    root->addWidget(messageLabel_);

    setCentralWidget(central);

    decisionLabel_->setText(QStringLiteral("unknown"));
    productLabel_->setText(QStringLiteral("-"));
    defectLabel_->setText(QStringLiteral("unknown"));
    scoreLabel_->setText(QStringLiteral("0.00"));
    confidenceLabel_->setText(QStringLiteral("0.00"));
    offsetLabel_->setText(QStringLiteral("x 0.00  y 0.00  angle 0.00"));
    motionLabel_->setText(motionText(workpiece::MotionState{}));
    communicationLabel_->setText(communicationText(workpiece::CommunicationState{}));
    setMessage(QStringLiteral("Ready."));
}

void MainWindow::initializeRuntime()
{
    const workpiece::ConfigLoadResult validated = workpiece::validateAppConfig(config_);
    config_ = validated.config;
    config_.imageSource.mode = workpiece::ImageSourceMode::Simulated;
    config_.imageSource.networkCameraUrl.clear();

    workpiece::PersistenceResult opened = repository_.open(config_.persistence.databasePath);
    if (!opened.success) {
        setMessage(opened.message);
        setCycleControlsEnabled(false);
        return;
    }

    const workpiece::PersistenceResult initialized = repository_.initialize();
    if (!initialized.success) {
        setMessage(initialized.message);
        setCycleControlsEnabled(false);
        return;
    }

    workflow_ = std::make_unique<workpiece::WorkflowController>(config_,
                                                                imageSource_,
                                                                inspector_,
                                                                motionController_,
                                                                communicationEndpoint_,
                                                                imageStorage_,
                                                                repository_);
    workflow_->addObserver([this](const workpiece::WorkflowSnapshot &snapshot) {
        renderSnapshot(snapshot);
    });

    setCycleControlsEnabled(true);
}

void MainWindow::renderSnapshot(const workpiece::WorkflowSnapshot &snapshot)
{
    stateLabel_->setText(stationStateText(snapshot.stationState));
    setMessage(snapshot.message);

    if (!snapshot.inspectionFrame.image.isNull()) {
        updateImage(snapshot.inspectionFrame.image);
    } else if (!snapshot.positioningFrame.image.isNull()) {
        updateImage(snapshot.positioningFrame.image);
    }

    offsetLabel_->setText(QStringLiteral("x %1  y %2  angle %3")
                              .arg(snapshot.positioningAnalysis.offset.x, 0, 'f', 2)
                              .arg(snapshot.positioningAnalysis.offset.y, 0, 'f', 2)
                              .arg(snapshot.positioningAnalysis.offset.angle, 0, 'f', 2));
    motionLabel_->setText(motionText(snapshot.motionState));
    communicationLabel_->setText(communicationText(snapshot.communicationState));

    if (snapshot.inspectionAnalysis.valid) {
        decisionLabel_->setText(decisionText(snapshot.inspectionAnalysis.result.decision));
        productLabel_->setText(QString::fromStdString(snapshot.inspectionAnalysis.result.productId));
        defectLabel_->setText(defectText(snapshot.inspectionAnalysis.result.defectType));
        scoreLabel_->setText(QString::number(snapshot.inspectionAnalysis.result.defectScore, 'f', 2));
        confidenceLabel_->setText(QString::number(snapshot.inspectionAnalysis.result.confidence, 'f', 2));
    }
}

void MainWindow::renderResult(const workpiece::WorkflowRunResult &result)
{
    stateLabel_->setText(stationStateText(result.finalState));
    setMessage(result.message);
    if (result.success) {
        decisionLabel_->setText(decisionText(result.inspectionRecord.result));
        productLabel_->setText(result.inspectionRecord.productId);
        defectLabel_->setText(defectText(result.inspectionRecord.defectType));
        scoreLabel_->setText(QString::number(result.inspectionRecord.defectScore, 'f', 2));
    }
}

void MainWindow::setMessage(const QString &message)
{
    messageLabel_->setText(QStringLiteral("%1  %2")
                               .arg(QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss")))
                               .arg(message));
}

void MainWindow::updateImage(const QImage &image)
{
    if (image.isNull()) {
        imageLabel_->setPixmap(QPixmap());
        imageLabel_->setText(QStringLiteral("No image"));
        return;
    }

    imageLabel_->setText(QString());
    imageLabel_->setPixmap(QPixmap::fromImage(image).scaled(imageLabel_->size(),
                                                            Qt::KeepAspectRatio,
                                                            Qt::SmoothTransformation));
}

void MainWindow::setCycleControlsEnabled(bool enabled)
{
    startButton_->setEnabled(enabled);
}
