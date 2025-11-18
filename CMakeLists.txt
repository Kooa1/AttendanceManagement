cmake_minimum_required(VERSION 3.24)
project(FaceDetection)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (WIN32)
    set(CMAKE_PREFIX_PATH "D:/application/Qt/6.9.0/msvc2022_64")
elseif (APPLE)
    set(CMAKE_PREFIX_PATH "/Users/wuwenze/Qt/6.9.0/macos/lib/cmake/Qt6")
endif ()

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED
)

find_package(OpenCV REQUIRED)

# 自动查找所有头文件
file(GLOB_RECURSE HEADER_FILES
        "include/*.h"
        "include/*.hpp"
        "include/*.hxx"
        "src/*.h"
        "src/*.hpp"
        "src/*.hxx"
)

# 自动查找所有源文件
file(GLOB_RECURSE SOURCE_FILES
        "src/*.cpp"
        "src/*.cxx"
        "src/*.cc"
        "src/*.c"
)

# 添加可执行文件
add_executable(FaceDetection
        ${SOURCE_FILES}
        ${HEADER_FILES}
)

# 设置包含目录
target_include_directories(FaceDetection PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${OpenCV_INCLUDE_DIRS}
)

# 链接库 - 所有平台都使用OpenCV目标
target_link_libraries(FaceDetection
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        opencv_core
        opencv_imgproc
        opencv_videoio
        opencv_highgui
        opencv_imgcodecs
        opencv_video
)

# MSVC 编译器选项
if (MSVC)
    add_compile_options(/Zc:__cplusplus)
endif ()

# Windows 平台部署
if (WIN32)
    # 设置 Qt 安装路径
    set(QT_INSTALL_PATH "D:/application/Qt/6.9.0/msvc2022_64")
    
    # 调试版本后缀
    set(DEBUG_SUFFIX "")
    if (CMAKE_BUILD_TYPE MATCHES "Debug")
        set(DEBUG_SUFFIX "d")
    endif ()
    
    # 复制主要的 Qt DLLs
    foreach (QT_LIB Core Gui Widgets)
        add_custom_command(TARGET FaceDetection POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:FaceDetection>"
                COMMENT "Copying Qt6${QT_LIB}${DEBUG_SUFFIX}.dll"
        )
    endforeach ()
    
    # 复制平台插件
    if (EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
        add_custom_command(TARGET FaceDetection POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:FaceDetection>/plugins/platforms/"
                COMMENT "Creating platforms directory"
        )
        
        add_custom_command(TARGET FaceDetection POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:FaceDetection>/plugins/platforms/"
                COMMENT "Copying qwindows${DEBUG_SUFFIX}.dll"
        )
    endif ()
    
    # 复制OpenCV DLLs
    if (OpenCV_DIR)
        # 查找OpenCV DLL文件
        file(GLOB OPENCV_DLLS "${OpenCV_DIR}/*.dll")
        foreach(DLL_FILE ${OPENCV_DLLS})
            add_custom_command(TARGET FaceDetection POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${DLL_FILE}"
                    "$<TARGET_FILE_DIR:FaceDetection>"
                    COMMENT "Copying OpenCV DLL: ${DLL_FILE}"
            )
        endforeach()
    endif()

endif ()

# 可选：打印找到的文件列表用于调试
message(STATUS "Found header files: ${HEADER_FILES}")
message(STATUS "Found source files: ${SOURCE_FILES}")