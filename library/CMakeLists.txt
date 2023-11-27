cmake_minimum_required(VERSION 3.5)

project(MediaStreaming)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

link_directories(/usr/local/lib)

include_directories(
    /Users/takeru/Documents/GitHub/connect-udp-library/library/uvgRTP/include/uvgrtp
    /Users/takeru/Documents/GitHub/connect-udp-library/library/opus/include
)

find_library(
    OPUS_LIBRARY NAMES opus PATHS  /Users/takeru/Documents/GitHub/connect-udp-library/library/opus/lib
)

add_executable(mediaReceiver mediaReceiver.cpp)
target_link_libraries(mediaReceiver ${OPUS_LIBRARY} uvgrtp)

