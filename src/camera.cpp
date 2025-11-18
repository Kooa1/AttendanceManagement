//
// Created by 吴文泽 on 2025/11/18.
//

#include "../include/camera.h"

Camera::Camera()
    : cap(0) {
}

void Camera::capture() {
    cv::Mat frame;
    cap >> frame;
    if (frame.empty()) {
        qDebug() << "frame is empty\n";
    }
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

    const QImage ret_img(rgb_mat.data, rgb_mat.cols, rgb_mat.rows, rgb_mat.step, QImage::Format_RGB888);
    return QPixmap::fromImage(ret_img);
}
