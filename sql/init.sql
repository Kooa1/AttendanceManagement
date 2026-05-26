CREATE DATABASE IF NOT EXISTS attendance
    DEFAULT CHARACTER SET utf8mb4
    DEFAULT COLLATE utf8mb4_unicode_ci;

USE attendance;

CREATE TABLE IF NOT EXISTS employees (
    id          INT AUTO_INCREMENT PRIMARY KEY,
    name        VARCHAR(100) NOT NULL,
    face_feature MEDIUMBLOB   NULL,
    created_at  TIMESTAMP    DEFAULT CURRENT_TIMESTAMP
) ENGINE = InnoDB
  DEFAULT CHARSET = utf8mb4;

CREATE TABLE IF NOT EXISTS attendance_records (
    id          INT AUTO_INCREMENT PRIMARY KEY,
    employee_id INT          NOT NULL,
    check_time  DATETIME     NOT NULL,
    type        ENUM ('check_in', 'check_out') NOT NULL,
    FOREIGN KEY (employee_id) REFERENCES employees (id) ON DELETE CASCADE,
    INDEX idx_employee_time (employee_id, check_time)
) ENGINE = InnoDB
  DEFAULT CHARSET = utf8mb4;
