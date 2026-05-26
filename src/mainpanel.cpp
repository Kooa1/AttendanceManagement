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
      main_hlayout(new QHBoxLayout),
      camera(pix_queue) {
    init_layout();
    init_conn();
}

void MainPanel::refresh() {
    if (!pix_queue.empty()) {
        const auto pix = pix_queue.front();
        pix_queue.pop();
        show_label->setPixmap(pix.scaled(show_label->size(), Qt::KeepAspectRatio));
    }
}

void MainPanel::init_layout() {
    main_vlayout->addWidget(show_label);
    main_vlayout->addLayout(main_hlayout);

    main_hlayout->addWidget(start);
    main_hlayout->addWidget(stop);

    this->resize(600, 400);
    this->show();
}

void MainPanel::init_conn() {
    connect(start, &QPushButton::clicked, this, [this]() {
        timer->start(33);
        capping = true;
    });

    connect(timer, &QTimer::timeout, this, [this]() {
        if (capping) {
            camera.capture();
            refresh();
        } else {
            timer->stop();
        }
    });

    connect(stop, &QPushButton::clicked, this, [this]() {
        capping = false;
        show_label->setPixmap(QPixmap());
    });
}
