#include "CameraThread.h"
#include <QDebug>

CameraThread::CameraThread(std::shared_ptr<FaceEngine> faceEngine, QObject *parent)
    : QThread(parent)
    , m_face_engine(std::move(faceEngine)) {
}

CameraThread::~CameraThread() {
    stop();
}

void CameraThread::stop() {
    m_running = false;
    if (isRunning()) {
        wait();
    }
}

void CameraThread::run() {
    cv::VideoCapture cap;
    if (!cap.open(0, cv::CAP_DSHOW)) {
        emit errorOccurred("无法打开摄像头，请检查摄像头连接");
        return;
    }

    // Set camera properties
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    cap.set(cv::CAP_PROP_FPS, 30);

    m_running = true;
    cv::Mat frame;

    while (m_running) {
        if (!cap.read(frame)) {
            emit errorOccurred("读取摄像头画面失败");
            break;
        }

        if (frame.empty()) continue;

        // Face detection
        if (m_detect_face && m_face_engine && m_face_engine->is_loaded()) {
            cv::Rect faceRect = m_face_engine->detect_face(frame);

            if (faceRect.area() > 0) {
                // Draw rectangle
                m_face_engine->draw_face_rect(frame, faceRect);

                // Extract face region and emit
                cv::Mat faceROI = frame(faceRect).clone();
                QImage faceImage = mat_to_q_image(faceROI);

                emit faceDetected(QRect(faceRect.x, faceRect.y, faceRect.width, faceRect.height),
                                  faceImage);
            }
        }

        // Convert and emit frame for display
        QImage qimg = mat_to_q_image(frame);
        emit frameCaptured(qimg);

        // Throttle to ~30 FPS
        msleep(30);
    }

    cap.release();
}

QImage CameraThread::mat_to_q_image(const cv::Mat &mat) {
    if (mat.empty()) return {};

    cv::Mat rgb;
    if (mat.channels() == 3) {
        cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
    } else if (mat.channels() == 4) {
        cv::cvtColor(mat, rgb, cv::COLOR_BGRA2RGB);
    } else {
        rgb = mat.clone();
    }

    QImage qimg(rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888);
    return qimg.copy(); // Deep copy since mat data will be freed
}
