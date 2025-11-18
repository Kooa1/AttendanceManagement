//
// Created by 吴文泽 on 2025/11/17.
//

#include "mainpanel.h"

MainPanel::MainPanel(QWidget *parent)
    : QWidget(parent),
      timer(new QTimer),
      show_label(new QLabel),
      start(new QPushButton("cap")),
      stop(new QPushButton("stop")),
      main_vlayout(new QVBoxLayout(this)),
      main_hlayout(new QHBoxLayout) {
    init_layout();
    init_conn();
}

void MainPanel::reflesh(const cv::Mat &frame) {
}

void MainPanel::init_layout() {
    show_label->setStyleSheet("background : white");

    main_vlayout->addWidget(show_label);
    main_vlayout->addLayout(main_hlayout);

    main_hlayout->addWidget(start);
    main_hlayout->addWidget(stop);

    this->resize(400, 600);
    this->show();
}

void MainPanel::init_conn() {
    connect(start, &QPushButton::clicked, this, [this]() {
        timer->start(33);
    });

    connect(timer, &QTimer::timeout, this, [this]() {
        camera.capture();
    });
}
