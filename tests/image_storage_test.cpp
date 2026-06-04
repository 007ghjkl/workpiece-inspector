#include <QFile>
#include <QTemporaryDir>
#include <QTest>

#include "persistence/SQLiteRepository.h"
#include "storage/ImageStorage.h"
#include "vision/SimulatedImageSource.h"

using namespace workpiece;

namespace {

bool isStoredUnderDirectory(const QString &path, const QString &directoryPath)
{
    QDir directory(QFileInfo(directoryPath).absoluteFilePath());
    const QString relativePath = directory.relativeFilePath(QFileInfo(path).absoluteFilePath());
    return !relativePath.startsWith(QStringLiteral("../"))
        && relativePath != QStringLiteral("..")
        && !QDir::isAbsolutePath(relativePath);
}

ImageFrame simulatedInspectionFrame()
{
    AppConfig config = defaultAppConfig();
    config.simulation.seed = 321;
    config.simulation.defectProbability = 1.0;

    SimulatedImageSource source;
    source.open(config);
    return source.capture(FrameRole::Inspection);
}

InspectionRecord recordForFrame(const ImageFrame &frame, const QString &imageRef)
{
    InspectionRecord record;
    record.timestamp = QDateTime::currentDateTimeUtc();
    record.productId = frame.metadata.productId;
    record.sourceType = frame.metadata.sourceType;
    record.imageRef = imageRef;
    record.result = InspectionDecision::Pass;
    record.defectType = frame.metadata.defectType;
    record.defectScore = frame.metadata.defectScore;
    record.offset = frame.metadata.offset;
    record.communicationState.status = CommunicationStatus::Connected;
    record.cycleTimeMs = 42;
    return record;
}

} // namespace

class ImageStorageTest : public QObject
{
    Q_OBJECT

private slots:
    void savesImageAndPersistsReference()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        PersistenceConfig config;
        config.imageOutputDirectory = tempDir.path() + QStringLiteral("/images");
        config.saveImages = true;

        const ImageFrame frame = simulatedInspectionFrame();
        const ImageStorage storage;
        const ImageStorageResult stored = storage.saveFrame(frame, config);

        QVERIFY(stored.success);
        QVERIFY(!stored.imageRef.isEmpty());
        QVERIFY(QFile::exists(stored.absolutePath));
        QCOMPARE(QFileInfo(stored.absolutePath).absolutePath(), QFileInfo(config.imageOutputDirectory).absoluteFilePath());
        QVERIFY(!stored.imageRef.contains(QLatin1Char('/')));
        QVERIFY(!stored.imageRef.contains(QLatin1Char('\\')));

        SQLiteRepository repository;
        QVERIFY(repository.open(tempDir.path() + QStringLiteral("/records.sqlite")).success);
        QVERIFY(repository.initialize().success);
        QVERIFY(repository.saveInspectionRecord(recordForFrame(frame, stored.imageRef)).success);

        const QList<InspectionRecord> records = repository.queryInspectionHistory();
        QCOMPARE(records.size(), 1);
        QCOMPARE(records.first().imageRef, stored.imageRef);
    }

    void disabledSavingReturnsEmptyReference()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        PersistenceConfig config;
        config.imageOutputDirectory = tempDir.path() + QStringLiteral("/images");
        config.saveImages = false;

        const ImageStorage storage;
        const ImageStorageResult stored = storage.saveFrame(simulatedInspectionFrame(), config);

        QVERIFY(stored.success);
        QVERIFY(stored.imageRef.isEmpty());
        QVERIFY(stored.absolutePath.isEmpty());
        QVERIFY(!QFileInfo::exists(config.imageOutputDirectory));
    }

    void sanitizesFileNames()
    {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        ImageFrame frame = simulatedInspectionFrame();
        frame.metadata.productId = QStringLiteral("../SIM Product:001");

        PersistenceConfig config;
        config.imageOutputDirectory = tempDir.path();
        config.saveImages = true;

        const ImageStorage storage;
        const ImageStorageResult stored = storage.saveFrame(frame, config);

        QVERIFY(stored.success);
        QVERIFY(QFile::exists(stored.absolutePath));
        QVERIFY(!stored.imageRef.contains(QStringLiteral("..")));
        QVERIFY(!stored.imageRef.contains(QLatin1Char('/')));
        QVERIFY(!stored.imageRef.contains(QLatin1Char('\\')));
        QVERIFY(isStoredUnderDirectory(stored.absolutePath, tempDir.path()));
    }

    void invalidInputsFailClearly()
    {
        const ImageStorage storage;

        PersistenceConfig emptyDirectory;
        emptyDirectory.imageOutputDirectory.clear();
        ImageStorageResult result = storage.saveFrame(simulatedInspectionFrame(), emptyDirectory);
        QVERIFY(!result.success);
        QVERIFY(result.message.contains(QStringLiteral("directory")));

        PersistenceConfig validDirectory;
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        validDirectory.imageOutputDirectory = tempDir.path();
        ImageFrame emptyFrame;

        result = storage.saveFrame(emptyFrame, validDirectory);
        QVERIFY(!result.success);
        QVERIFY(result.message.contains(QStringLiteral("null image")));

        const QString filePath = tempDir.path() + QStringLiteral("/not-a-directory");
        QFile file(filePath);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("occupied");
        file.close();

        PersistenceConfig fileAsDirectory;
        fileAsDirectory.imageOutputDirectory = filePath;
        result = storage.saveFrame(simulatedInspectionFrame(), fileAsDirectory);
        QVERIFY(!result.success);
        QVERIFY(result.message.contains(QStringLiteral("directory")));
    }
};

QTEST_GUILESS_MAIN(ImageStorageTest)

#include "image_storage_test.moc"
