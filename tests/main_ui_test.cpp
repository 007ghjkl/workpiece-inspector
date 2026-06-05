#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QTemporaryDir>
#include <QTest>
#include <QTableWidget>
#include <QChartView>

#include "configuration/AppConfig.h"
#include "ui/MainWindow.h"

using namespace workpiece;

namespace {

QLabel *labelByName(QWidget &window, const char *name)
{
    auto *label = window.findChild<QLabel *>(QString::fromLatin1(name));
    Q_ASSERT(label != nullptr);
    return label;
}

} // namespace

class MainUiTest : public QObject
{
    Q_OBJECT

private slots:
    void defaultMainWindowLaunchesBeforeRuntimeInitialization()
    {
        MainWindow window;
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));

        QCOMPARE(labelByName(window, "stationStateLabel")->text(), QStringLiteral("idle"));
        QCOMPARE(labelByName(window, "modeLabel")->text(), QStringLiteral("SIMULATION MODE"));
        QCOMPARE(labelByName(window, "totalCountLabel")->text(), QStringLiteral("0"));
        QVERIFY(labelByName(window, "historyEmptyLabel")->isVisible());

        auto *button = window.findChild<QPushButton *>(QStringLiteral("startCycleButton"));
        QVERIFY(button != nullptr);
        QVERIFY(button->isEnabled());
    }

    void mainWindowRunsSingleCycle()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        AppConfig config = defaultAppConfig();
        config.persistence.databasePath = tempDir.path() + QStringLiteral("/ui.sqlite");
        config.persistence.imageOutputDirectory = tempDir.path() + QStringLiteral("/images");
        config.persistence.saveImages = true;
        config.simulation.seed = 902;
        config.simulation.defectProbability = 0.0;

        MainWindow window(config);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));

        auto *button = window.findChild<QPushButton *>(QStringLiteral("startCycleButton"));
        QVERIFY(button != nullptr);
        QVERIFY(button->isEnabled());

        QTest::mouseClick(button, Qt::LeftButton);
        QTest::mouseClick(button, Qt::LeftButton);

        QCOMPARE(labelByName(window, "stationStateLabel")->text(), QStringLiteral("completed"));
        QCOMPARE(labelByName(window, "modeLabel")->text(), QStringLiteral("SIMULATION MODE"));
        QCOMPARE(labelByName(window, "decisionLabel")->text(), QStringLiteral("pass"));
        QCOMPARE(labelByName(window, "productLabel")->text(), QStringLiteral("SIM-000001"));
        QVERIFY(labelByName(window, "motionLabel")->text().contains(QStringLiteral("completed")));
        QVERIFY(labelByName(window, "communicationLabel")->text().contains(QStringLiteral("connected")));
        QVERIFY(labelByName(window, "messageLabel")->text().contains(QStringLiteral("completed")));

        auto *imageLabel = labelByName(window, "currentImageLabel");
        QVERIFY(!imageLabel->pixmap().isNull());

        auto *historyTable = window.findChild<QTableWidget *>(QStringLiteral("historyTable"));
        QVERIFY(historyTable != nullptr);
        QCOMPARE(historyTable->rowCount(), 2);
        QCOMPARE(labelByName(window, "totalCountLabel")->text(), QStringLiteral("2"));
        QCOMPARE(labelByName(window, "passCountLabel")->text(), QStringLiteral("2"));
        QCOMPARE(labelByName(window, "failCountLabel")->text(), QStringLiteral("0"));
        QCOMPARE(labelByName(window, "passRateLabel")->text(), QStringLiteral("100.0%"));
        QVERIFY(!labelByName(window, "historyEmptyLabel")->isVisible());

        auto *chartView = window.findChild<QChartView *>(QStringLiteral("qualityChartView"));
        QVERIFY(chartView != nullptr);
        QVERIFY(chartView->chart() != nullptr);

        auto *productFilter = window.findChild<QLineEdit *>(QStringLiteral("productFilterEdit"));
        auto *resultFilter = window.findChild<QComboBox *>(QStringLiteral("resultFilterCombo"));
        auto *applyFilter = window.findChild<QPushButton *>(QStringLiteral("applyHistoryFilterButton"));
        QVERIFY(productFilter != nullptr);
        QVERIFY(resultFilter != nullptr);
        QVERIFY(applyFilter != nullptr);

        productFilter->setText(QStringLiteral("SIM-000001"));
        resultFilter->setCurrentText(QStringLiteral("All"));
        QTest::mouseClick(applyFilter, Qt::LeftButton);
        QCOMPARE(historyTable->rowCount(), 2);
        QCOMPARE(historyTable->item(0, 1)->text(), QStringLiteral("SIM-000001"));

        productFilter->clear();
        resultFilter->setCurrentText(QStringLiteral("Pass"));
        QTest::mouseClick(applyFilter, Qt::LeftButton);
        QCOMPARE(historyTable->rowCount(), 2);

        productFilter->clear();
        resultFilter->setCurrentText(QStringLiteral("Fail"));
        QTest::mouseClick(applyFilter, Qt::LeftButton);
        QCOMPARE(historyTable->rowCount(), 0);
    }
};

QTEST_MAIN(MainUiTest)

#include "main_ui_test.moc"
