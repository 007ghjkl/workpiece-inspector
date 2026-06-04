#include "storage/ImageStorage.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>

namespace workpiece {
namespace {

QByteArray imageDigest(const QImage &image)
{
    QByteArray bytes;
    const int width = image.width();
    const int height = image.height();
    const int format = static_cast<int>(image.format());
    const int bytesPerLine = image.bytesPerLine();
    bytes.append(reinterpret_cast<const char *>(&width), sizeof(int));
    bytes.append(reinterpret_cast<const char *>(&height), sizeof(int));
    bytes.append(reinterpret_cast<const char *>(&format), sizeof(int));
    bytes.append(reinterpret_cast<const char *>(&bytesPerLine), sizeof(int));
    bytes.append(reinterpret_cast<const char *>(image.constBits()), image.sizeInBytes());
    return QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex();
}

QString safePathToken(QString value, const QString &fallback)
{
    value = value.trimmed().toLower();
    value.replace(QRegularExpression(QStringLiteral("[^a-z0-9_-]+")), QStringLiteral("_"));
    value.replace(QRegularExpression(QStringLiteral("_+")), QStringLiteral("_"));
    value = value.trimmed();
    while (value.startsWith(QLatin1Char('_'))) {
        value.remove(0, 1);
    }
    while (value.endsWith(QLatin1Char('_'))) {
        value.chop(1);
    }
    return value.isEmpty() ? fallback : value;
}

QString frameRoleToken(FrameRole role)
{
    return safePathToken(toString(role), QStringLiteral("unknown"));
}

bool isUnderDirectory(const QString &targetPath, const QString &directoryPath)
{
    const QString target = QDir::cleanPath(QFileInfo(targetPath).absoluteFilePath());
    const QString directory = QDir::cleanPath(QFileInfo(directoryPath).absoluteFilePath());
    return target.startsWith(directory + QDir::separator());
}

} // namespace

ImageStorageResult ImageStorage::saveFrame(const ImageFrame &frame, const PersistenceConfig &config) const
{
    if (!config.saveImages) {
        return {true, QString(), QString(), QStringLiteral("Image saving is disabled.")};
    }

    if (config.imageOutputDirectory.trimmed().isEmpty()) {
        return {false, QString(), QString(), QStringLiteral("Image output directory is empty.")};
    }

    if (frame.image.isNull()) {
        return {false, QString(), QString(), QStringLiteral("Cannot save a null image frame.")};
    }

    QDir outputDir(config.imageOutputDirectory);
    if (!outputDir.exists() && !outputDir.mkpath(QStringLiteral("."))) {
        return {false, QString(), QString(), QStringLiteral("Image output directory could not be created.")};
    }

    const QString outputRoot = QDir::cleanPath(QFileInfo(outputDir.absolutePath()).absoluteFilePath());
    const QString product = safePathToken(frame.metadata.productId, QStringLiteral("product"));
    const QString role = frameRoleToken(frame.metadata.role);
    const QString digest = QString::fromLatin1(imageDigest(frame.image).left(16));
    const QString fileName = QStringLiteral("%1_%2_s%3_%4.png")
                                 .arg(product)
                                 .arg(role)
                                 .arg(frame.metadata.scenarioId)
                                 .arg(digest);
    const QString absolutePath = QDir(outputRoot).filePath(fileName);

    if (!isUnderDirectory(absolutePath, outputRoot)) {
        return {false, QString(), QString(), QStringLiteral("Generated image path is outside the output directory.")};
    }

    if (!frame.image.save(absolutePath, "PNG")) {
        return {false, QString(), absolutePath, QStringLiteral("Image file could not be written.")};
    }

    return {true, fileName, absolutePath, QStringLiteral("Image frame saved.")};
}

} // namespace workpiece
