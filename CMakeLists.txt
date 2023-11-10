# # videoReceiver.cppとvideoSender.cppのCMake
# cmake_minimum_required(VERSION 3.0)

# project(VideoStreaming)

# find_package(OpenCV REQUIRED)
# set(FFMPEG_LIBRARIES "/opt/homebrew/Cellar/ffmpeg/6.0_1/lib")

# include_directories(${OpenCV_INCLUDE_DIRS})

# # FFmpeg includePathの設定
# include_directories("/opt/homebrew/Cellar/ffmpeg/6.0_1/include")

# add_executable(videoSender videoSender.cpp)
# target_link_libraries(videoSender ${OpenCV_LIBS} ${FFMPEG_LIBRARIES}/libavformat.dylib ${FFMPEG_LIBRARIES}/libavcodec.dylib ${FFMPEG_LIBRARIES}/libavutil.dylib ${FFMPEG_LIBRARIES}/libswscale.dylib)

# add_executable(videoReceiver videoReceiver.cpp)
# target_link_libraries(videoReceiver ${OpenCV_LIBS} ${FFMPEG_LIBRARIES}/libavformat.dylib ${FFMPEG_LIBRARIES}/libavcodec.dylib ${FFMPEG_LIBRARIES}/libavutil.dylib ${FFMPEG_LIBRARIES}/libswscale.dylib)



# testReceive.cppとtestSend.cppのCMake
cmake_minimum_required(VERSION 3.0)

project(VideoStreaming)

find_package(OpenCV REQUIRED)
set(FFMPEG_LIBRARIES "/opt/homebrew/Cellar/ffmpeg/6.0_1/lib")

include_directories(${OpenCV_INCLUDE_DIRS})

# FFmpeg includePathの設定
include_directories("/opt/homebrew/Cellar/ffmpeg/6.0_1/include")

add_executable(testSender testSender.cpp)
target_link_libraries(testSender ${OpenCV_LIBS} ${FFMPEG_LIBRARIES}/libavformat.dylib ${FFMPEG_LIBRARIES}/libavcodec.dylib ${FFMPEG_LIBRARIES}/libavutil.dylib ${FFMPEG_LIBRARIES}/libswscale.dylib)

add_executable(testReceiver testReceiver.cpp)
target_link_libraries(testReceiver ${OpenCV_LIBS} ${FFMPEG_LIBRARIES}/libavformat.dylib ${FFMPEG_LIBRARIES}/libavcodec.dylib ${FFMPEG_LIBRARIES}/libavutil.dylib ${FFMPEG_LIBRARIES}/libswscale.dylib)

