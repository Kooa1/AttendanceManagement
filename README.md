# AttendanceManagement

基于人脸识别的员工打卡考勤系统，使用 Qt6 + OpenCV + MySQL 构建。

## 功能特性

- **摄像头实时画面** — 开启/关闭摄像头，实时显示视频流
- **人脸检测** — 基于 OpenCV Haar Cascade 进行人脸检测，并在画面中标注
- **人脸注册** — 输入员工姓名，检测到人脸后提取特征存入数据库
- **人脸识别** — 实时匹配摄像头中的人脸与已注册员工，显示识别结果和匹配度
- **签到/签退** — 识别到已注册员工后，可进行签到和签退操作，记录考勤数据
- **操作日志** — 实时显示签到、签退、注册等操作记录

## 项目结构

```
AttendanceManagement/
├── CMakeLists.txt              # CMake 构建配置
├── main.cpp                    # 主程序入口（MainWindow 版本）
├── include/                    # 头文件
│   ├── MainWindow.h           # 主窗口声明
│   ├── FaceEngine.h           # 人脸引擎声明
│   ├── DatabaseManager.h      # 数据库管理器声明
│   ├── CameraThread.h         # 摄像头线程声明
│   ├── camera.h               # 旧版摄像头驱动声明
│   ├── mainpanel.h            # 旧版主面板声明
│   └── safequeue.h            # 线程安全队列模板
├── frontend/                   # 新版前端实现
│   ├── MainWindow.cpp         # 主窗口实现（UI 布局、信号槽、识别逻辑）
│   ├── FaceEngine.cpp         # 人脸引擎实现（检测、特征提取、匹配）
│   ├── DatabaseManager.cpp    # 数据库操作实现（MySQL 连接、CRUD）
│   └── CameraThread.cpp       # 摄像头线程实现（异步采集、检测）
└── src/                        # 旧版源码
    ├── main.cpp               # 旧版主入口（MainPanel 版本）
    ├── camera.cpp             # 旧版摄像头驱动实现
    └── mainpanel.cpp          # 旧版主面板实现
```

## 环境要求

| 依赖 | 版本 |
|---|---|
| CMake | >= 3.24 |
| Qt | >= 6.0 (Core, Gui, Widgets) |
| OpenCV | >= 4.0 (含 objdetect 模块) |
| MySQL / MariaDB | 任意支持 C API 的版本 |
| 编译器 | MSVC (Windows) |

## 构建步骤

### 1. 配置 CMake

编辑 `CMakeLists.txt` 中的路径变量，确保与本地环境一致：

```cmake
set(CMAKE_PREFIX_PATH "D:/application/Qt/6.9.0/msvc2022_64")   # Qt 安装路径
set(OpenCV_DIR "D:/src/opencv/build")                          # OpenCV 构建目录
set(MYSQL_DIR "D:/application/Mysql")                          # MySQL 安装目录
```

### 2. 构建

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

构建完成后会自动将所需的 Qt DLL、OpenCV DLL、MySQL DLL 以及 Haar Cascade XML 文件复制到可执行文件所在目录。

### 3. 运行

```bash
./build/AttendanceManagement.exe
```

## 数据库说明

系统会在运行时自动创建数据库和表，无需手动建库。

### 数据库表结构

**employees 表**

| 字段 | 类型 | 说明 |
|---|---|---|
| id | INT AUTO_INCREMENT | 员工 ID（主键） |
| name | VARCHAR(100) | 员工姓名 |
| face_feature | MEDIUMBLOB | 人脸特征向量 |
| created_at | TIMESTAMP | 创建时间 |

**attendance_records 表**

| 字段 | 类型 | 说明 |
|---|---|---|
| id | INT AUTO_INCREMENT | 记录 ID（主键） |
| employee_id | INT | 员工 ID（外键） |
| check_time | DATETIME | 打卡时间 |
| type | ENUM('check_in', 'check_out') | 打卡类型 |

### 数据库连接参数

默认连接参数可在 `DatabaseManager` 构造函数中修改：

| 参数 | 默认值 |
|---|---|
| Host | localhost |
| Port | 3306 |
| User | root |
| Password | 123456 |
| Database | attendance |

## 核心模块说明

### FaceEngine（人脸引擎）

- 使用 Haar Cascade 分类器检测人脸
- 基于像素特征提取 + L2 归一化生成特征向量
- 使用欧氏距离进行人脸匹配

### CameraThread（摄像头线程）

- 独立线程运行，不阻塞 UI
- 支持实时人脸检测和画面帧发射
- 通过信号与槽机制与主窗口通信

### DatabaseManager（数据库管理器）

- 封装 MySQL C API，提供员工和考勤记录的增删查改
- 特征向量以 BLOB 形式存储

## 命名规范

本项目统一使用 snake_case 风格命名自定义函数和变量，类名使用 PascalCase。

## 许可证

仅供学习参考使用。