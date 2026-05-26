#include "MainWindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QMessageBox>
#include <QDateTime>
#include <QDebug>
#include <QApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {

    m_face_engine = std::make_shared<FaceEngine>("haarcascade_frontalface_default.xml");
    m_db = std::make_shared<DatabaseManager>();

    setup_ui();

    if (m_db->connect()) {
        m_db->init_tables();
        cache_employee_features();
        show_status("数据库连接成功");
    } else {
        show_status("数据库连接失败，请检查MySQL服务是否运行", true);
    }

    if (!m_face_engine->is_loaded()) {
        show_status("人脸检测模型加载失败", true);
    } else {
        show_status("人脸检测模型已加载");
    }

    resize(900, 700);
    setWindowTitle("人脸识别打卡系统");
}

MainWindow::~MainWindow() {
    if (m_camera_thread) {
        m_camera_thread->stop();
    }
}

void MainWindow::setup_ui() {
    auto *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto *mainLayout = new QHBoxLayout(centralWidget);

    // ========== Left: Camera ==========
    auto *leftPanel = new QWidget;
    auto *leftLayout = new QVBoxLayout(leftPanel);

    m_camera_label = new QLabel("点击「开启摄像头」开始");
    m_camera_label->setAlignment(Qt::AlignCenter);
    m_camera_label->setMinimumSize(640, 480);
    m_camera_label->setStyleSheet("QLabel { background-color: #1a1a2e; color: #ccc; "
                                  "border: 2px solid #444; border-radius: 8px; "
                                  "font-size: 16px; }");

    auto *cameraBtnLayout = new QHBoxLayout;
    m_btn_start_camera = new QPushButton("开启摄像头");
    m_btn_stop_camera = new QPushButton("关闭摄像头");
    m_btn_stop_camera->setEnabled(false);
    cameraBtnLayout->addWidget(m_btn_start_camera);
    cameraBtnLayout->addWidget(m_btn_stop_camera);

    leftLayout->addWidget(m_camera_label);
    leftLayout->addLayout(cameraBtnLayout);

    // ========== Right: Controls ==========
    auto *rightPanel = new QWidget;
    rightPanel->setMaximumWidth(300);
    auto *rightLayout = new QVBoxLayout(rightPanel);

    // Status
    m_status_label = new QLabel("就绪");
    m_status_label->setWordWrap(true);
    m_status_label->setStyleSheet("QLabel { padding: 8px; background: #f0f0f0; "
                                  "border-radius: 4px; font-size: 13px; }");

    // Recognition status
    m_recognition_label = new QLabel("等待识别...");
    m_recognition_label->setAlignment(Qt::AlignCenter);
    m_recognition_label->setStyleSheet(
        "QLabel { padding: 12px; font-size: 16px; font-weight: bold; "
        "border: 2px solid #ddd; border-radius: 6px; }");

    // Registration group
    auto *regGroup = new QGroupBox("人脸注册");
    auto *regLayout = new QFormLayout(regGroup);
    m_name_input = new QLineEdit;
    m_name_input->setPlaceholderText("输入员工姓名");
    m_btn_register = new QPushButton("注册人脸");
    m_btn_register->setEnabled(false);
    regLayout->addRow("姓名:", m_name_input);
    regLayout->addRow("", m_btn_register);

    // Attendance group
    auto *attGroup = new QGroupBox("人脸打卡");
    auto *attLayout = new QVBoxLayout(attGroup);

    m_btn_check_in = new QPushButton("签到 (Check-in)");
    m_btn_check_in->setEnabled(false);
    m_btn_check_in->setStyleSheet(
        "QPushButton { background-color: #27ae60; color: white; padding: 10px; "
        "font-size: 14px; border-radius: 6px; }"
        "QPushButton:hover { background-color: #2ecc71; }"
        "QPushButton:disabled { background-color: #bdc3c7; }");

    m_btn_check_out = new QPushButton("签退 (Check-out)");
    m_btn_check_out->setEnabled(false);
    m_btn_check_out->setStyleSheet(
        "QPushButton { background-color: #e74c3c; color: white; padding: 10px; "
        "font-size: 14px; border-radius: 6px; }"
        "QPushButton:hover { background-color: #ec7063; }"
        "QPushButton:disabled { background-color: #bdc3c7; }");

    attLayout->addWidget(m_btn_check_in);
    attLayout->addWidget(m_btn_check_out);

    // Log group
    auto *logGroup = new QGroupBox("操作日志");
    auto *logLayout = new QVBoxLayout(logGroup);
    m_log_output = new QTextEdit;
    m_log_output->setReadOnly(true);
    m_log_output->setMaximumHeight(200);
    logLayout->addWidget(m_log_output);

    // Assemble right panel
    rightLayout->addWidget(m_status_label);
    rightLayout->addWidget(m_recognition_label);
    rightLayout->addWidget(regGroup);
    rightLayout->addWidget(attGroup);
    rightLayout->addWidget(logGroup);
    rightLayout->addStretch();

    mainLayout->addWidget(leftPanel, 1);
    mainLayout->addWidget(rightPanel, 0);

    // Signals
    connect(m_btn_start_camera, &QPushButton::clicked, this, &MainWindow::on_start_camera);
    connect(m_btn_stop_camera, &QPushButton::clicked, this, &MainWindow::on_stop_camera);
    connect(m_btn_register, &QPushButton::clicked, this, &MainWindow::on_register_face);
    connect(m_btn_check_in, &QPushButton::clicked, this, &MainWindow::on_check_in);
    connect(m_btn_check_out, &QPushButton::clicked, this, &MainWindow::on_check_out);
}

void MainWindow::on_start_camera() {
    if (!m_face_engine->is_loaded()) {
        QMessageBox::warning(this, "错误", "人脸检测模型未加载，无法启动摄像头");
        return;
    }

    m_camera_thread = std::make_unique<CameraThread>(m_face_engine);
    connect(m_camera_thread.get(), &CameraThread::frameCaptured,
            this, &MainWindow::on_frame_captured);
    connect(m_camera_thread.get(), &CameraThread::faceDetected,
            this, &MainWindow::on_face_detected);
    connect(m_camera_thread.get(), &CameraThread::errorOccurred,
            this, [this](const QString &msg) {
                show_status(msg, true);
                on_stop_camera();
            });
    connect(m_camera_thread.get(), &CameraThread::finished,
            this, [this]() {
                show_status("摄像头已停止");
                m_btn_start_camera->setEnabled(true);
                m_btn_stop_camera->setEnabled(false);
                m_btn_register->setEnabled(false);
                m_recognition_label->setText("等待识别...");
                m_recognition_label->setStyleSheet(
                    "QLabel { padding: 12px; font-size: 16px; font-weight: bold; "
                    "border: 2px solid #ddd; border-radius: 6px; }");
                m_btn_check_in->setEnabled(false);
                m_btn_check_out->setEnabled(false);
            });

    m_camera_thread->start();
    m_btn_start_camera->setEnabled(false);
    m_btn_stop_camera->setEnabled(true);
    m_btn_register->setEnabled(!m_name_input->text().isEmpty());
    show_status("摄像头已开启");
}

void MainWindow::on_stop_camera() {
    if (m_camera_thread) {
        m_camera_thread->stop();
        m_camera_thread.reset();
    }
    m_btn_start_camera->setEnabled(true);
    m_btn_stop_camera->setEnabled(false);
    m_btn_register->setEnabled(false);
    m_camera_label->setText("摄像头已关闭");
    m_face_detected = false;
    m_current_face_image = {};
    m_recognized_id = -1;
    m_recognized_name.clear();
}

void MainWindow::on_register_face() {
    QString name = m_name_input->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入员工姓名");
        return;
    }

    if (!m_face_detected || m_current_face_image.isNull()) {
        QMessageBox::warning(this, "提示", "未检测到人脸，请面对摄像头");
        return;
    }

    // Convert QImage → cv::Mat
    QImage img = m_current_face_image.convertToFormat(QImage::Format_RGB888);
    cv::Mat mat(img.height(), img.width(), CV_8UC3,
                const_cast<uchar *>(img.bits()), img.bytesPerLine());
    cv::Mat face;
    cv::cvtColor(mat, face, cv::COLOR_RGB2BGR);

    std::vector<float> feature = m_face_engine->extract_feature(face);
    if (feature.empty()) {
        show_status("特征提取失败", true);
        return;
    }

    int id = m_db->add_employee(name, feature);
    if (id > 0) {
        QString msg = QString("员工「%1」注册成功 (ID: %2)").arg(name).arg(id);
        show_status(msg);
        m_log_output->append(QString("[%1] %2")
            .arg(QDateTime::currentDateTime().toString("HH:mm:ss"))
            .arg(msg));
        m_name_input->clear();
        cache_employee_features();
        m_face_detected = false;
        m_current_face_image = {};
    } else {
        show_status("注册失败，请检查数据库连接", true);
    }
}

void MainWindow::on_check_in() {
    if (m_recognized_id < 0) {
        QMessageBox::warning(this, "提示", "未识别到已注册人脸，请面对摄像头");
        return;
    }

    // Check-in rule: allow if no unchecked check-out (i.e. last action today is check_out or none)
    auto records = m_db->get_records_by_date(m_recognized_id, QDate::currentDate());
    bool canCheckIn = true;
    for (const auto &r : records) {
        if (r.type == "check_in") canCheckIn = false;
        if (r.type == "check_out") canCheckIn = true;
    }

    if (!canCheckIn) {
        show_status(QString("「%1」已签到，请先签退").arg(m_recognized_name), true);
        return;
    }

    if (m_db->record_attendance(m_recognized_id, "check_in")) {
        QString msg = QString("「%1」签到成功 ✓").arg(m_recognized_name);
        show_status(msg);
        m_log_output->append(QString("[%1] %2")
            .arg(QDateTime::currentDateTime().toString("HH:mm:ss"))
            .arg(msg));
    }
}

void MainWindow::on_check_out() {
    if (m_recognized_id < 0) {
        QMessageBox::warning(this, "提示", "未识别到已注册人脸，请面对摄像头");
        return;
    }

    auto lastRecord = m_db->get_last_record(m_recognized_id);
    if (lastRecord.type != "check_in" || lastRecord.checkTime.date() != QDate::currentDate()) {
        show_status(QString("「%1」今天尚未签到，无法签退").arg(m_recognized_name), true);
        return;
    }

    if (m_db->record_attendance(m_recognized_id, "check_out")) {
        QString msg = QString("「%1」签退成功 ✓").arg(m_recognized_name);
        show_status(msg);
        m_log_output->append(QString("[%1] %2")
            .arg(QDateTime::currentDateTime().toString("HH:mm:ss"))
            .arg(msg));
    }
}

void MainWindow::on_frame_captured(const QImage &frame) {
    m_camera_label->setPixmap(QPixmap::fromImage(frame).scaled(
        m_camera_label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void MainWindow::on_face_detected(const QRect &, const QImage &faceImage) {
    m_current_face_image = faceImage;
    m_face_detected = true;
    m_btn_register->setEnabled(!m_name_input->text().isEmpty());

    // Run face recognition every ~15 frames (~0.5s at 30fps)
    m_frame_counter++;
    if (m_frame_counter % 15 == 0 && !m_features.empty()) {
        recognize_face(faceImage);
    }
}

void MainWindow::recognize_face(const QImage &faceImage) {
    QImage img = faceImage.convertToFormat(QImage::Format_RGB888);
    cv::Mat mat(img.height(), img.width(), CV_8UC3,
                const_cast<uchar *>(img.bits()), img.bytesPerLine());
    cv::Mat face;
    cv::cvtColor(mat, face, cv::COLOR_RGB2BGR);

    std::vector<float> feature = m_face_engine->extract_feature(face);
    if (feature.empty()) return;

    auto [idx, dist] = m_face_engine->match_face(feature, m_features, 0.65f);

    if (idx >= 0 && idx < static_cast<int>(m_employees.size())) {
        if (m_recognized_id != m_employees[idx].id) {
            QString msg = QString("识别到: %1").arg(m_employees[idx].name);
            show_status(msg);
        }
        m_recognized_id = m_employees[idx].id;
        m_recognized_name = m_employees[idx].name;
        m_recognized_confidence = dist;

        float similarity = 1.0f - std::min(dist / 1.414f, 1.0f);
        m_recognition_label->setText(
            QString("已识别: %1\n匹配度: %2%")
                .arg(m_recognized_name)
                .arg(static_cast<int>(similarity * 100)));
        m_recognition_label->setStyleSheet(
            "QLabel { padding: 12px; font-size: 16px; font-weight: bold; "
            "border: 2px solid #27ae60; border-radius: 6px; color: #27ae60; }");

        m_btn_check_in->setEnabled(true);
        m_btn_check_out->setEnabled(true);
    } else {
        if (m_recognized_id >= 0) {
            show_status("人脸未匹配到已注册员工");
        }
        m_recognized_id = -1;
        m_recognized_name.clear();
        m_recognition_label->setText("未识别");
        m_recognition_label->setStyleSheet(
            "QLabel { padding: 12px; font-size: 16px; font-weight: bold; "
            "border: 2px solid #e74c3c; border-radius: 6px; color: #e74c3c; }");
        m_btn_check_in->setEnabled(false);
        m_btn_check_out->setEnabled(false);
    }
}

void MainWindow::cache_employee_features() {
    m_employees = m_db->get_all_employees();
    m_features.clear();
    for (const auto &emp : m_employees) {
        if (!emp.faceFeature.empty()) {
            m_features.push_back(emp.faceFeature);
        }
    }
    qDebug() << "Cached" << m_features.size() << "employee features";
}

void MainWindow::show_status(const QString &msg, bool isError) {
    m_status_label->setText(msg);
    m_status_label->setStyleSheet(
        isError
        ? "QLabel { padding: 8px; background: #fde8e8; color: #c0392b; "
          "border-radius: 4px; font-size: 13px; }"
        : "QLabel { padding: 8px; background: #e8f8e8; color: #27ae60; "
          "border-radius: 4px; font-size: 13px; }");
    qDebug() << (isError ? "[ERROR]" : "[INFO]") << msg;
}