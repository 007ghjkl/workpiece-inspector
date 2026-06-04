#pragma once

#include "configuration/AppConfig.h"
#include "vision/ImageSource.h"

#include <QString>

namespace workpiece {

struct ImageStorageResult {
    bool success = false;
    QString imageRef;
    QString absolutePath;
    QString message;
};

class ImageStorage
{
public:
    ImageStorageResult saveFrame(const ImageFrame &frame, const PersistenceConfig &config) const;
};

} // namespace workpiece
