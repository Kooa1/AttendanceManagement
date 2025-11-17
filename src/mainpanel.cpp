//
// Created by 吴文泽 on 2025/11/17.
//

#include "../include/mainpanel.h"

MainPanel::MainPanel() {
    this->resize(600, 400);
    this->show();
}

void MainPanel::Mat_to_QPixmap(const cv::Mat &src_mat) {
    cv::Mat rgb_mat;

    if (src_mat.channels() == 3) {
        cv::cvtColor(src_mat, rgb_mat, cv::COLOR_BGR2RGB);
    } else if (src_mat.channels() == 1) {
        cv::cvtColor(src_mat, rgb_mat, cv::COLOR_GRAY2RGB);
    } else {
        rgb_mat = src_mat.clone();
    }
}
