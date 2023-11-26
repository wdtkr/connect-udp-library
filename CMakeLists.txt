cmake_minimum_required(VERSION 3.0)

project(VideoStreaming)

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

link_directories(/usr/local/lib)

include_directories(
    /Users/takeru/Documents/GitHub/connect-udp-library/uvgRTP/include/uvgrtp
    /opt/homebrew/Cellar/portaudio/19.7.0/include
    /opt/homebrew/Cellar/libsndfile/1.2.2/include
    /opt/homebrew/Cellar/opus/1.4/include/opus
    /opt/homebrew/Cellar/opusfile/0.12_1/include
    /opt/homebrew/Cellar/libogg/1.3.5/include
)

find_library(
    PORTAUDIO_LIBRARY NAMES portaudio PATHS /opt/homebrew/Cellar/portaudio/19.7.0/lib
    SNDFILE_LIBRARY NAMES sndfile PATHS /opt/homebrew/Cellar/libsndfile/1.2.2/lib
    OPUS_LIBRARY NAMES opus PATHS /opt/homebrew/Cellar/opus/1.4/lib
    OPUSFILE_LIBRARY NAMES opusfile PATHS /opt/homebrew/Cellar/opusfile/0.12_1/lib
    OGG_LIBRARY NAMES ogg PATHS /opt/homebrew/Cellar/libogg/1.3.5/lib
)

add_executable(mediaSender mediaSender.cpp)
target_link_libraries(mediaSender ${OpenCV_LIBS} uvgrtp ${PORTAUDIO_LIBRARY} ${SNDFILE_LIBRARY} ${OPUS_LIBRARY} ${OPUSFILE_LIBRARY})

add_executable(mediaReceiver mediaReceiver.cpp)
target_link_libraries(mediaReceiver ${OpenCV_LIBS} uvgrtp ${PORTAUDIO_LIBRARY} ${SNDFILE_LIBRARY} ${OPUS_LIBRARY} ${OPUSFILE_LIBRARY})

# add_executable(testEncoder testEncoder.cpp)
# target_link_libraries(testEncoder ${OpenCV_LIBS} uvgrtp ${PORTAUDIO_LIBRARY} ${SNDFILE_LIBRARY} ${OPUS_LIBRARY} ${OPUSFILE_LIBRARY})

# add_executable(testDecoder testDecoder.cpp)
# target_link_libraries(testDecoder ${OpenCV_LIBS} uvgrtp ${PORTAUDIO_LIBRARY} ${SNDFILE_LIBRARY} ${OPUS_LIBRARY} ${OPUSFILE_LIBRARY})

# add_executable(testSender testSender.cpp)
# target_link_libraries(testSender ${OpenCV_LIBS} uvgrtp)

# add_executable(testReceiver testReceiver.cpp)
# target_link_libraries(testReceiver ${OpenCV_LIBS} uvgrtp)
