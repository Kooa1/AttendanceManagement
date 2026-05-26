#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QString>
#include <QDateTime>
#include <vector>
#include <memory>
#include <mysql.h>

struct Employee {
    int id{};
    QString name;
    std::vector<float> faceFeature;
    QDateTime createdAt;
};

struct AttendanceRecord {
    int id{};
    int employeeId{};
    QDateTime checkTime;
    QString type; // "check_in" or "check_out"
};

class DatabaseManager {
public:
    explicit DatabaseManager(QString host = "localhost",
                             int port = 3306,
                             QString user = "root",
                             QString password = "123456",
                             QString database = "attendance");
    ~DatabaseManager();

    DatabaseManager(const DatabaseManager &) = delete;
    DatabaseManager &operator=(const DatabaseManager &) = delete;
    DatabaseManager(DatabaseManager &&) = delete;
    DatabaseManager &operator=(DatabaseManager &&) = delete;

    bool connect();
    bool init_tables();

    // Employee operations
    int add_employee(const QString &name, const std::vector<float> &feature);
    std::vector<Employee> get_all_employees();
    bool update_face_feature(int employeeId, const std::vector<float> &feature);

    // Attendance operations
    bool record_attendance(int employeeId, const QString &type);
    AttendanceRecord get_last_record(int employeeId);
    std::vector<AttendanceRecord> get_records_by_date(int employeeId, const QDate &date);

    bool is_connected() const { return m_connected; }

private:
    std::vector<float> blob_to_feature(const char *data, size_t len);
    std::string feature_to_blob(const std::vector<float> &feature);

    QString m_host;
    int m_port;
    QString m_user;
    QString m_password;
    QString m_database;
    MYSQL *m_mysql{};
    bool m_connected{false};
};

#endif // DATABASEMANAGER_H
