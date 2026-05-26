#ifndef CAMERATHREAD_H
#define CAMERATHREAD_H

#include <QThread>
#include <QImage>
#include <atomic>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include "FaceEngine.h"
#include <memory>

class CameraThread : public QThread {
Q_OBJECT

public:
    explicit CameraThread(std::shared_ptr<FaceEngine> faceEngine, QObject *parent = nullptr);
    ~CameraThread() override;

    void stop();
    void set_detect_face(bool detect) { m_detect_face = detect; }

signals:
    void frameCaptured(const QImage &frame);
    void faceDetected(const QRect &faceRect, const QImage &faceImage);
    void errorOccurred(const QString &message);

protected:
    void run() override;

private:
    QImage mat_to_q_image(const cv::Mat &mat);

    std::shared_ptr<FaceEngine> m_face_engine;
    std::atomic<bool> m_running{false};
    std::atomic<bool> m_detect_face{true};
};

#endif // CAMERATHREAD_H
