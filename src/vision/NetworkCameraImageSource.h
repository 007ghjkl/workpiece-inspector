#pragma once

#include "vision/ImageSource.h"

#include <memory>

namespace workpiece {

class NetworkCameraImageSource final : public IImageSource
{
public:
    NetworkCameraImageSource();
    ~NetworkCameraImageSource() override;

    void open(const AppConfig &config) override;
    void close() override;
    ImageFrame capture(FrameRole role) override;
    ImageSourceStatus status() const override;

private:
    struct CameraHandle;

    static bool isSupportedUrl(const QString &url);
    ImageFrame emptyFrame(FrameRole role) const;

    ImageSourceStatus status_;
    QString url_;
    std::unique_ptr<CameraHandle> camera_;
};

} // namespace workpiece
