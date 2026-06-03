#include <QTest>

class SmokeTest : public QObject
{
    Q_OBJECT

private slots:
    void testFrameworkRuns()
    {
        QVERIFY(true);
    }
};

QTEST_APPLESS_MAIN(SmokeTest)

#include "smoke_test.moc"
