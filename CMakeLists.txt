cmake_minimum_required(VERSION 3.24)
project(AttendanceManagement)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (WIN32)
    set(CMAKE_PREFIX_PATH "D:/application/Qt/6.9.0/msvc2022_64")
    set(OpenCV_DIR "D:/src/opencv/build")
    set(MYSQL_DIR "D:/application/Mysql")
endif ()

# 查找 Qt6
find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED
)

# 查找 OpenCV
find_package(OpenCV REQUIRED
        PATHS "${OpenCV_DIR}"
        NO_DEFAULT_PATH
)

message(STATUS "OpenCV version: ${OpenCV_VERSION}")
message(STATUS "OpenCV include: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "OpenCV libraries: ${OpenCV_LIBRARIES}")

# 自动查找所有头文件
file(GLOB_RECURSE HEADER_FILES
        "include/*.h"
        "include/*.hpp"
)

# 自动查找所有源文件
file(GLOB_RECURSE SOURCE_FILES
        "frontend/*.cpp"
)

# 添加可执行文件
add_executable(AttendanceManagement
        ${SOURCE_FILES}
        ${HEADER_FILES}
        main.cpp
)

target_compile_definitions(AttendanceManagement PRIVATE
        NOMINMAX
)

# 设置包含目录
target_include_directories(AttendanceManagement PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/frontend
        ${OpenCV_INCLUDE_DIRS}
        "${MYSQL_DIR}/include"
)

# 链接库
target_link_libraries(AttendanceManagement
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        ${OpenCV_LIBRARIES}
        "${MYSQL_DIR}/lib/libmysql.lib"
)

# MSVC 编译器选项
if (MSVC)
    add_compile_options(/Zc:__cplusplus)
    add_compile_options(/utf-8)
endif ()

# Windows 平台部署 - 复制 Qt DLLs
if (WIN32)
    set(QT_INSTALL_PATH "D:/application/Qt/6.9.0/msvc2022_64")

    set(DEBUG_SUFFIX "")
    if (CMAKE_BUILD_TYPE MATCHES "Debug")
        set(DEBUG_SUFFIX "d")
    endif ()

    foreach (QT_LIB Core Gui Widgets)
        add_custom_command(TARGET AttendanceManagement POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:AttendanceManagement>"
                COMMENT "Copying Qt6${QT_LIB}${DEBUG_SUFFIX}.dll"
        )
    endforeach ()

    # 复制 OpenCV DLL
    add_custom_command(TARGET AttendanceManagement POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${OpenCV_DIR}/x64/vc16/bin/opencv_world4120${DEBUG_SUFFIX}.dll"
            "$<TARGET_FILE_DIR:AttendanceManagement>"
            COMMENT "Copying OpenCV DLL"
    )

    # 复制 MySQL DLL
    add_custom_command(TARGET AttendanceManagement POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${MYSQL_DIR}/lib/libmysql.dll"
            "$<TARGET_FILE_DIR:AttendanceManagement>"
            COMMENT "Copying MySQL DLL"
    )

    # 复制 Haar Cascade
    add_custom_command(TARGET AttendanceManagement POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${OpenCV_DIR}/etc/haarcascades/haarcascade_frontalface_default.xml"
            "$<TARGET_FILE_DIR:AttendanceManagement>"
            COMMENT "Copying Haar cascade XML"
    )

    # 复制平台插件
    if (EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
        add_custom_command(TARGET AttendanceManagement POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:AttendanceManagement>/plugins/platforms/"
                COMMENT "Creating platforms directory"
        )
        add_custom_command(TARGET AttendanceManagement POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:AttendanceManagement>/plugins/platforms/"
                COMMENT "Copying qwindows${DEBUG_SUFFIX}.dll"
        )
    endif ()
endif ()

message(STATUS "Found header files: ${HEADER_FILES}")
message(STATUS "Found source files: ${SOURCE_FILES}")
