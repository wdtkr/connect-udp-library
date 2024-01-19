# cmake_minimum_required(VERSION 3.5)

# project(VideoStreaming)

# find_package(OpenCV REQUIRED)

# include_directories(${OpenCV_INCLUDE_DIRS})

# link_directories(/usr/local/lib)

# include_directories(
#     /Users/takeru/Documents/GitHub/connect-udp-library/uvgRTP/include/uvgrtp
#     /opt/homebrew/Cellar/portaudio/19.7.0/include
#     /opt/homebrew/Cellar/libsndfile/1.2.2/include
#     /opt/homebrew/Cellar/opus/1.4/include/opus
#     /opt/homebrew/Cellar/opusfile/0.12_1/include
#     /opt/homebrew/Cellar/libogg/1.3.5/include
#     /usr/local/include/jrtplib3/
# )

# find_library(PORTAUDIO_LIBRARY NAMES portaudio PATHS /opt/homebrew/Cellar/portaudio/19.7.0/lib)
# find_library(SNDFILE_LIBRARY NAMES sndfile PATHS /opt/homebrew/Cellar/libsndfile/1.2.2/lib)
# find_library(OPUS_LIBRARY NAMES opus PATHS /opt/homebrew/Cellar/opus/1.4/lib)
# find_library(OPUSFILE_LIBRARY NAMES opusfile PATHS /opt/homebrew/Cellar/opusfile/0.12_1/lib)
# find_library(OGG_LIBRARY NAMES ogg PATHS /opt/homebrew/Cellar/libogg/1.3.5/lib)
# find_library(JRTPLIB_LIBRARY NAMES jrtp PATHS /usr/local/lib)


# add_executable(testSender testSender.cpp)
# target_link_libraries(testSender ${OpenCV_LIBS} uvgrtp ${PORTAUDIO_LIBRARY} ${SNDFILE_LIBRARY} ${OPUS_LIBRARY} ${OPUSFILE_LIBRARY})

# add_executable(testReceiver testReceiver.cpp)
# target_link_libraries(testReceiver ${OpenCV_LIBS} uvgrtp ${PORTAUDIO_LIBRARY} ${SNDFILE_LIBRARY} ${OPUS_LIBRARY} ${OPUSFILE_LIBRARY})


# add_executable(testSenderJrtp testSenderJrtp.cpp)
# target_link_libraries(testSenderJrtp ${OpenCV_LIBS} ${PORTAUDIO_LIBRARY} ${SNDFILE_LIBRARY} ${OPUS_LIBRARY} ${OPUSFILE_LIBRARY} ${JRTPLIB_LIBRARY})

# add_executable(testReceiverJrtp testReceiverJrtp.cpp)
# target_link_libraries(testReceiverJrtp ${OpenCV_LIBS} ${PORTAUDIO_LIBRARY} ${SNDFILE_LIBRARY} ${OPUS_LIBRARY} ${OPUSFILE_LIBRARY} ${JRTPLIB_LIBRARY})


# add_executable(testEncoder testEncoder.cpp)
# target_link_libraries(testEncoder ${OpenCV_LIBS} uvgrtp ${PORTAUDIO_LIBRARY} ${SNDFILE_LIBRARY} ${OPUS_LIBRARY} ${OPUSFILE_LIBRARY})

# add_executable(testDecoder testDecoder.cpp)
# target_link_libraries(testDecoder ${OpenCV_LIBS} uvgrtp ${PORTAUDIO_LIBRARY} ${SNDFILE_LIBRARY} ${OPUS_LIBRARY} ${OPUSFILE_LIBRARY})




# --------- ios build --------------

cmake_minimum_required(VERSION 3.10)
project(UdpLibrary)

set(CMAKE_CXX_STANDARD 11)

# iOSビルドの設定
set(CMAKE_SYSTEM_NAME iOS)
set(CMAKE_OSX_ARCHITECTURES "arm64")
set(CMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH NO)
set(CMAKE_IOS_INSTALL_COMBINED YES)
set(CMAKE_XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "11.0")

# ソースファイルを指定
add_library(udp-lib-ios SHARED lib.cpp)
