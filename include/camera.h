//
// Created by 吴文泽 on 2025/11/18.
//

#ifndef FACEDETECTION_CAMERA_H
#define FACEDETECTION_CAMERA_H

#include <iostream>

#include <QPixmap>
#include <QDebug>

#include <opencv2/opencv.hpp>

class Camera {
public:
    explicit Camera();

    void capture();

private:
    QPixmap Mat_to_QPixmap(const cv::Mat &src_mat);

private:
    cv::VideoCapture cap;
};


#endif //FACEDETECTION_CAMERA_H
