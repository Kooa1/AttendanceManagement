//
// Created by 吴文泽 on 2025/11/17.
//

#ifndef ZPLAYER_MAINPANEL_H
#define ZPLAYER_MAINPANEL_H

#include <QMainWindow>

#include <opencv2/opencv.hpp>

class MainPanel final : QMainWindow{
public:
    explicit MainPanel();

    void Mat_to_QPixmap(const cv::Mat &src_mat);

private:
};


#endif //ZPLAYER_MAINPANEL_H
