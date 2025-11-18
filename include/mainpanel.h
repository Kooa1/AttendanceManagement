//
// Created by 吴文泽 on 2025/11/17.
//

#ifndef ZPLAYER_MAINPANEL_H
#define ZPLAYER_MAINPANEL_H

#include <iostream>

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <opencv2/opencv.hpp>

#include "camera.h"

class MainPanel final : QWidget {
public:
    explicit MainPanel(QWidget *parent = nullptr);

public slots:
    void reflesh();

private:
    void init_layout();

    void init_conn();

private:
    Camera camera;

    QTimer *timer;

    QLabel *show_label;

    QPushButton *start;

    QPushButton *stop;

    QVBoxLayout *main_vlayout;

    QHBoxLayout *main_hlayout;

    std::queue<QPixmap> pix_queue;

    bool capping;
};


#endif //ZPLAYER_MAINPANEL_H
