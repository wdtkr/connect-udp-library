cmake_minimum_required(VERSION 3.0)
project(MyVideoEncoder)

# OpenCVを見つけてリンク
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# OpenH264のライブラリパスを追加
set(OPENH264_LIBRARY_PATH "${CMAKE_SOURCE_DIR}/openh264-2.3.1/libopenh264-2.3.1-mac-arm64.dylib")

# ソースファイルを定義
set(SOURCE_FILES test.cpp)

# 実行可能ファイルを追加
add_executable(MyVideoEncoder ${SOURCE_FILES})

# OpenCVとOpenH264のライブラリをターゲットにリンク
target_link_libraries(MyVideoEncoder ${OpenCV_LIBS} ${OPENH264_LIBRARY_PATH})
