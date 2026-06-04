#include "vision/NetworkCameraImageSource.h"

#include <QUrl>

#if WORKPIECE_HAVE_OPENCV
#include <opencv2/imgproc.hpp>
#endif

namespace workpiece {

NetworkCameraImageSource::NetworkCameraImageSource()
    : status_{ImageSourceState::Closed, "Network camera source closed."}
{
}

NetworkCameraImageSource::~NetworkCameraImageSource()
{
    close();
}

void NetworkCameraImageSource::open(const AppConfig &config)
{
    close();

    if (config.imageSource.mode != ImageSourceMode::NetworkCamera) {
        status_ = ImageSourceStatus{ImageSourceState::Faulted, "Network camera mode is not selected."};
        return;
    }

    if (!isSupportedUrl(config.imageSource.networkCameraUrl)) {
        status_ = ImageSourceStatus{ImageSourceState::Faulted, "Network camera URL is invalid or unsupported."};
        return;
    }

    url_ = config.imageSource.networkCameraUrl;

#if WORKPIECE_HAVE_OPENCV
    status_ = ImageSourceStatus{ImageSourceState::Opening, "Opening network camera stream."};
    if (!capture_.open(url_.toStdString())) {
        status_ = ImageSourceStatus{ImageSourceState::Faulted, "OpenCV VideoCapture could not open the network camera stream."};
        return;
    }

    status_ = ImageSourceStatus{ImageSourceState::Ready, "Network camera source ready."};
#else
    status_ = ImageSourceStatus{ImageSourceState::Faulted, "OpenCV support is not available in this build."};
#endif
}

void NetworkCameraImageSource::close()
{
#if WORKPIECE_HAVE_OPENCV
    if (capture_.isOpened()) {
        capture_.release();
    }
#endif

    url_.clear();
    status_ = ImageSourceStatus{ImageSourceState::Closed, "Network camera source closed."};
}

ImageFrame NetworkCameraImageSource::capture(FrameRole role)
{
    if (status_.state != ImageSourceState::Ready) {
        if (status_.state != ImageSourceState::Faulted) {
            status_ = ImageSourceStatus{ImageSourceState::Faulted, "Network camera source is not ready."};
        }
        return emptyFrame(role);
    }

#if WORKPIECE_HAVE_OPENCV
    status_ = ImageSourceStatus{ImageSourceState::Capturing, "Capturing network camera frame."};

    cv::Mat frame;
    if (!capture_.read(frame) || frame.empty()) {
        status_ = ImageSourceStatus{ImageSourceState::Faulted, "OpenCV VideoCapture failed to read a frame."};
        return emptyFrame(role);
    }

    cv::Mat rgbFrame;
    if (frame.channels() == 1) {
        cv::cvtColor(frame, rgbFrame, cv::COLOR_GRAY2RGB);
    } else {
        cv::cvtColor(frame, rgbFrame, cv::COLOR_BGR2RGB);
    }

    QImage image(rgbFrame.data,
                 rgbFrame.cols,
                 rgbFrame.rows,
                 static_cast<int>(rgbFrame.step),
                 QImage::Format_RGB888);

    ImageFrame result;
    result.image = image.copy();
    result.metadata = emptyFrame(role).metadata;

    status_ = ImageSourceStatus{ImageSourceState::Ready, "Network camera frame captured."};
    return result;
#else
    status_ = ImageSourceStatus{ImageSourceState::Faulted, "OpenCV support is not available in this build."};
    return emptyFrame(role);
#endif
}

ImageSourceStatus NetworkCameraImageSource::status() const
{
    return status_;
}

bool NetworkCameraImageSource::isSupportedUrl(const QString &url)
{
    const QUrl parsed(url);
    if (!parsed.isValid() || parsed.scheme().isEmpty()) {
        return false;
    }

    const QString scheme = parsed.scheme().toLower();
    return scheme == QStringLiteral("rtsp")
        || scheme == QStringLiteral("http")
        || scheme == QStringLiteral("https")
        || scheme == QStringLiteral("file");
}

ImageFrame NetworkCameraImageSource::emptyFrame(FrameRole role) const
{
    ImageFrame frame;
    frame.metadata.role = role;
    frame.metadata.sourceType = QStringLiteral("network_camera");
    frame.metadata.defectType = DefectType::Unknown;
    return frame;
}

} // namespace workpiece
