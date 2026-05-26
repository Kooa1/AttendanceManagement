#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QImage>
#include <memory>
#include <vector>

#include "CameraThread.h"
#include "FaceEngine.h"
#include "DatabaseManager.h"
struct Employee;

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void on_start_camera();
    void on_stop_camera();
    void on_register_face();
    void on_check_in();
    void on_check_out();
    void on_frame_captured(const QImage &frame);
    void on_face_detected(const QRect &faceRect, const QImage &faceImage);

private:
    void setup_ui();
    void cache_employee_features();
    void recognize_face(const QImage &faceImage);
    void show_status(const QString &msg, bool isError = false);

    // UI components
    QLabel *m_camera_label{};
    QLabel *m_status_label{};
    QLabel *m_recognition_label{};
    QPushButton *m_btn_start_camera{};
    QPushButton *m_btn_stop_camera{};
    QPushButton *m_btn_register{};
    QPushButton *m_btn_check_in{};
    QPushButton *m_btn_check_out{};
    QLineEdit *m_name_input{};
    QTextEdit *m_log_output{};

    // Core components
    std::shared_ptr<FaceEngine> m_face_engine;
    std::shared_ptr<DatabaseManager> m_db;
    std::unique_ptr<CameraThread> m_camera_thread;

    // Current face for registration
    QImage m_current_face_image;
    bool m_face_detected{false};

    // Recognition state
    int m_recognized_id{-1};
    QString m_recognized_name;
    float m_recognized_confidence{0.0f};

    // Cached database features (reloaded after registration)
    std::vector<Employee> m_employees;
    std::vector<std::vector<float>> m_features; // parallel to m_employees

    int m_frame_counter{0};
};

#endif // MAINWINDOW_H