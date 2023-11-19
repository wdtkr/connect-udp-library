cmake_minimum_required(VERSION 3.0)

project(VideoStreaming)

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

link_directories(/usr/local/lib)
include_directories(/Users/takeru/Documents/GitHub/uvgRTP/include/uvgrtp)


# add_executable(testEncoder testEncoder.cpp)
# target_link_libraries(testEncoder ${OpenCV_LIBS} uvgrtp)

add_executable(testDecoder testDecoder.cpp)
target_link_libraries(testDecoder ${OpenCV_LIBS} uvgrtp)


add_executable(testSender testSender.cpp)
target_link_libraries(testSender ${OpenCV_LIBS} uvgrtp)

# add_executable(testReceiver testReceiver.cpp)
# target_link_libraries(testReceiver ${OpenCV_LIBS} uvgrtp)
