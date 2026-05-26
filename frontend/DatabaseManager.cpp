#include "DatabaseManager.h"
#include <QDebug>
#include <cstring>

DatabaseManager::DatabaseManager(QString host, int port, QString user,
                                 QString password, QString database)
    : m_host(std::move(host))
    , m_port(port)
    , m_user(std::move(user))
    , m_password(std::move(password))
    , m_database(std::move(database)) {
}

DatabaseManager::~DatabaseManager() {
    if (m_mysql) {
        mysql_close(m_mysql);
    }
}

bool DatabaseManager::connect() {
    m_mysql = mysql_init(nullptr);
    if (!m_mysql) {
        qWarning() << "mysql_init failed";
        return false;
    }

    unsigned int timeout = 5;
    mysql_options(m_mysql, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);

    if (!mysql_real_connect(m_mysql, m_host.toStdString().c_str(),
                            m_user.toStdString().c_str(),
                            m_password.toStdString().c_str(),
                            nullptr, m_port, nullptr, 0)) {
        qWarning() << "mysql_real_connect failed:" << mysql_error(m_mysql);
        mysql_close(m_mysql);
        m_mysql = nullptr;
        return false;
    }

    // Create database if not exists
    std::string createDb = "CREATE DATABASE IF NOT EXISTS " + m_database.toStdString()
                           + " DEFAULT CHARACTER SET utf8mb4";
    if (mysql_query(m_mysql, createDb.c_str()) != 0) {
        qWarning() << "Failed to create database:" << mysql_error(m_mysql);
        mysql_close(m_mysql);
        m_mysql = nullptr;
        return false;
    }

    // Select database
    if (mysql_select_db(m_mysql, m_database.toStdString().c_str()) != 0) {
        qWarning() << "Failed to select database:" << mysql_error(m_mysql);
        mysql_close(m_mysql);
        m_mysql = nullptr;
        return false;
    }

    // Set utf8mb4
    mysql_set_character_set(m_mysql, "utf8mb4");

    m_connected = true;
    qDebug() << "Database connected successfully";
    return true;
}

bool DatabaseManager::init_tables() {
    if (!m_connected) return false;

    const char *createEmployees = R"(
        CREATE TABLE IF NOT EXISTS employees (
            id           INT AUTO_INCREMENT PRIMARY KEY,
            name         VARCHAR(100) NOT NULL,
            face_feature MEDIUMBLOB NULL,
            created_at   TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        ) ENGINE = InnoDB DEFAULT CHARSET = utf8mb4
    )";

    if (mysql_query(m_mysql, createEmployees) != 0) {
        qWarning() << "Failed to create employees table:" << mysql_error(m_mysql);
        return false;
    }

    const char *createAttendance = R"(
        CREATE TABLE IF NOT EXISTS attendance_records (
            id          INT AUTO_INCREMENT PRIMARY KEY,
            employee_id INT NOT NULL,
            check_time  DATETIME NOT NULL,
            type        ENUM('check_in', 'check_out') NOT NULL,
            FOREIGN KEY (employee_id) REFERENCES employees(id) ON DELETE CASCADE,
            INDEX idx_employee_time (employee_id, check_time)
        ) ENGINE = InnoDB DEFAULT CHARSET = utf8mb4
    )";

    if (mysql_query(m_mysql, createAttendance) != 0) {
        qWarning() << "Failed to create attendance_records table:" << mysql_error(m_mysql);
        return false;
    }

    qDebug() << "Tables initialized successfully";
    return true;
}

int DatabaseManager::add_employee(const QString &name, const std::vector<float> &feature) {
    if (!m_connected) return -1;

    std::string blob = feature_to_blob(feature);
    std::string escapedName(256, '\0');
    std::string escapedBlob(blob.size() * 2 + 1, '\0');

    auto nameLen = mysql_real_escape_string(m_mysql, escapedName.data(),
                                            name.toStdString().c_str(),
                                            static_cast<unsigned long>(name.toStdString().size()));
    auto blobLen = mysql_real_escape_string(m_mysql, escapedBlob.data(), blob.data(),
                                            static_cast<unsigned long>(blob.size()));

    std::string query = "INSERT INTO employees (name, face_feature) VALUES ('"
                        + std::string(escapedName.data(), nameLen) + "', '"
                        + std::string(escapedBlob.data(), blobLen) + "')";

    if (mysql_query(m_mysql, query.c_str()) != 0) {
        qWarning() << "Failed to add employee:" << mysql_error(m_mysql);
        return -1;
    }

    return static_cast<int>(mysql_insert_id(m_mysql));
}

std::vector<Employee> DatabaseManager::get_all_employees() {
    std::vector<Employee> employees;
    if (!m_connected) return employees;

    if (mysql_query(m_mysql, "SELECT id, name, face_feature, created_at FROM employees") != 0) {
        qWarning() << "Failed to query employees:" << mysql_error(m_mysql);
        return employees;
    }

    MYSQL_RES *result = mysql_store_result(m_mysql);
    if (!result) {
        qWarning() << "mysql_store_result failed:" << mysql_error(m_mysql);
        return employees;
    }

    int numFields = mysql_num_fields(result);
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) != nullptr) {
        unsigned long *lengths = mysql_fetch_lengths(result);
        Employee emp;
        emp.id = std::stoi(row[0]);
        emp.name = QString::fromUtf8(row[1]);

        if (row[2] && lengths[2] > 0) {
            emp.faceFeature = blob_to_feature(row[2], lengths[2]);
        }

        if (row[3]) {
            emp.createdAt = QDateTime::fromString(QString::fromUtf8(row[3]), "yyyy-MM-dd HH:mm:ss");
        }

        employees.push_back(std::move(emp));
    }

    mysql_free_result(result);
    return employees;
}

bool DatabaseManager::update_face_feature(int employeeId, const std::vector<float> &feature) {
    if (!m_connected) return false;

    std::string blob = feature_to_blob(feature);
    std::string escapedBlob(blob.size() * 2 + 1, '\0');
    auto blobLen = mysql_real_escape_string(m_mysql, escapedBlob.data(), blob.data(),
                                            static_cast<unsigned long>(blob.size()));

    std::string query = "UPDATE employees SET face_feature = '"
                        + std::string(escapedBlob.data(), blobLen)
                        + "' WHERE id = " + std::to_string(employeeId);

    if (mysql_query(m_mysql, query.c_str()) != 0) {
        qWarning() << "Failed to update face feature:" << mysql_error(m_mysql);
        return false;
    }
    return true;
}

bool DatabaseManager::record_attendance(int employeeId, const QString &type) {
    if (!m_connected) return false;

    std::string query = "INSERT INTO attendance_records (employee_id, check_time, type) VALUES ("
                        + std::to_string(employeeId) + ", NOW(), '"
                        + type.toStdString() + "')";

    if (mysql_query(m_mysql, query.c_str()) != 0) {
        qWarning() << "Failed to record attendance:" << mysql_error(m_mysql);
        return false;
    }
    return true;
}

AttendanceRecord DatabaseManager::get_last_record(int employeeId) {
    AttendanceRecord rec;
    if (!m_connected) return rec;

    std::string query = "SELECT id, employee_id, check_time, type FROM attendance_records "
                        "WHERE employee_id = " + std::to_string(employeeId) + " "
                        "ORDER BY check_time DESC LIMIT 1";

    if (mysql_query(m_mysql, query.c_str()) != 0) {
        return rec;
    }

    MYSQL_RES *result = mysql_store_result(m_mysql);
    if (!result) return rec;

    MYSQL_ROW row = mysql_fetch_row(result);
    if (row) {
        rec.id = std::stoi(row[0]);
        rec.employeeId = std::stoi(row[1]);
        rec.checkTime = QDateTime::fromString(QString::fromUtf8(row[2]), "yyyy-MM-dd HH:mm:ss");
        rec.type = QString::fromUtf8(row[3]);
    }

    mysql_free_result(result);
    return rec;
}

std::vector<AttendanceRecord> DatabaseManager::get_records_by_date(int employeeId, const QDate &date) {
    std::vector<AttendanceRecord> records;
    if (!m_connected) return records;

    std::string query = "SELECT id, employee_id, check_time, type FROM attendance_records "
                        "WHERE employee_id = " + std::to_string(employeeId) + " "
                        "AND DATE(check_time) = '" + date.toString("yyyy-MM-dd").toStdString() + "' "
                        "ORDER BY check_time";

    if (mysql_query(m_mysql, query.c_str()) != 0) return records;

    MYSQL_RES *result = mysql_store_result(m_mysql);
    if (!result) return records;

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) != nullptr) {
        AttendanceRecord rec;
        rec.id = std::stoi(row[0]);
        rec.employeeId = std::stoi(row[1]);
        rec.checkTime = QDateTime::fromString(QString::fromUtf8(row[2]), "yyyy-MM-dd HH:mm:ss");
        rec.type = QString::fromUtf8(row[3]);
        records.push_back(std::move(rec));
    }

    mysql_free_result(result);
    return records;
}

std::vector<float> DatabaseManager::blob_to_feature(const char *data, size_t len) {
    if (len % sizeof(float) != 0) return {};
    std::vector<float> feature(len / sizeof(float));
    std::memcpy(feature.data(), data, len);
    return feature;
}

std::string DatabaseManager::feature_to_blob(const std::vector<float> &feature) {
    return {reinterpret_cast<const char *>(feature.data()), feature.size() * sizeof(float)};
}
