#include <QLabel>
#include <QPushButton>
#include <QTemporaryDir>
#include <QTest>

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

        QCOMPARE(labelByName(window, "stationStateLabel")->text(), QStringLiteral("completed"));
        QCOMPARE(labelByName(window, "modeLabel")->text(), QStringLiteral("SIMULATION MODE"));
        QCOMPARE(labelByName(window, "decisionLabel")->text(), QStringLiteral("pass"));
        QCOMPARE(labelByName(window, "productLabel")->text(), QStringLiteral("SIM-000001"));
        QVERIFY(labelByName(window, "motionLabel")->text().contains(QStringLiteral("completed")));
        QVERIFY(labelByName(window, "communicationLabel")->text().contains(QStringLiteral("connected")));
        QVERIFY(labelByName(window, "messageLabel")->text().contains(QStringLiteral("completed")));

        auto *imageLabel = labelByName(window, "currentImageLabel");
        QVERIFY(!imageLabel->pixmap().isNull());
    }
};

QTEST_MAIN(MainUiTest)

#include "main_ui_test.moc"
