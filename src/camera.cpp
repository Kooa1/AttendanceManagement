//
// Created by 吴文泽 on 2025/11/18.
//

#include "../include/camera.h"

Camera::Camera(std::queue<QPixmap> &pix_queue)
    : cap(0),
      pix_queue(pix_queue),
      face_cascade(
          "/opt/homebrew/Cellar/opencv/4.12.0_11/share/opencv4/haarcascades/haarcascade_frontalface_default.xml") {
}

void Camera::capture() {
    cv::Mat frame;
    cap >> frame;
    if (frame.empty()) {
        qDebug() << "frame is empty\n";
        return;
    }


    pix_queue.push(Mat_to_QPixmap(frame));
}

void Camera::stop() {
    cap.release();
}

QPixmap Camera::Mat_to_QPixmap(const cv::Mat &src_mat) {
    cv::Mat rgb_mat;

    if (src_mat.channels() == 3) {
        cv::cvtColor(src_mat, rgb_mat, cv::COLOR_BGR2RGB);
    } else if (src_mat.channels() == 1) {
        cv::cvtColor(src_mat, rgb_mat, cv::COLOR_GRAY2RGB);
    } else {
        rgb_mat = src_mat.clone();
    }

    auto dest_frame = draw_face_rectangle(src_mat, face_detection(src_mat));

    const QImage ret_img(dest_frame.data, dest_frame.cols, dest_frame.rows, dest_frame.step, QImage::Format_RGB888);
    return QPixmap::fromImage(ret_img);
}

std::vector<cv::Rect> Camera::face_detection(const cv::Mat &src_mat) {
    std::vector<cv::Rect> faces;
    cv::Mat gray_frame;

    cv::cvtColor(src_mat, gray_frame, cv::COLOR_RGB2GRAY);

    cv::equalizeHist(gray_frame, gray_frame);

    face_cascade.detectMultiScale(
        gray_frame,
        faces,
        1.1,
        3,
        0,
        cv::Size(30, 30)
    );

    return faces;
}

cv::Mat Camera::draw_face_rectangle(const cv::Mat &frame, const std::vector<cv::Rect> &faces) {
    cv::Mat result = frame.clone();

    for (const auto &face: faces) {
        cv::rectangle(result, face, cv::Scalar(0, 255, 0), 2);
    }

    return result;
}
