#pragma once

#include "vision/ImageSource.h"

#if WORKPIECE_HAVE_OPENCV
#include <opencv2/videoio.hpp>
#endif

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
    static bool isSupportedUrl(const QString &url);
    ImageFrame emptyFrame(FrameRole role) const;

    ImageSourceStatus status_;
    QString url_;

#if WORKPIECE_HAVE_OPENCV
    cv::VideoCapture capture_;
#endif
};

} // namespace workpiece
