//
// Created by 吴文泽 on 2025/11/18.
//

#ifndef FACEDETECTION_CAMERA_H
#define FACEDETECTION_CAMERA_H

#include <iostream>

#include <QPixmap>
#include <QDebug>

#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>

class Camera {
public:
    explicit Camera(std::queue<QPixmap> &pix_queue);

    void capture();

    void stop();

private:
    QPixmap mat_to_q_pixmap(const cv::Mat &src_mat);

    std::vector<cv::Rect>  face_detection(const cv::Mat &src_mat);

    cv::Mat draw_face_rectangle(const cv::Mat &frame, const std::vector<cv::Rect> &faces);

private:
    cv::VideoCapture cap;

    std::queue<QPixmap> &pix_queue;

    cv::CascadeClassifier face_cascade;
};


#endif //FACEDETECTION_CAMERA_H
