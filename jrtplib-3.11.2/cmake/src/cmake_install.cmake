# Install script for directory: /Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/Applications/Xcode-15.0.0.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/jrtplib3" TYPE FILE FILES
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtcpapppacket.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtcpbyepacket.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtcpcompoundpacket.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtcpcompoundpacketbuilder.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtcppacket.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtcppacketbuilder.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtcprrpacket.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtcpscheduler.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtcpsdesinfo.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtcpsdespacket.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtcpsrpacket.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtcpunknownpacket.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtpaddress.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtpcollisionlist.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/cmake/src/rtpconfig.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtpdebug.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtpdefines.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtperrors.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtphashtable.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtpinternalsourcedata.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtpipv4address.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtpipv4destination.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtpipv6address.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtpipv6destination.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtpkeyhashtable.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtplibraryversion.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtpmemorymanager.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtpmemoryobject.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtppacket.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtppacketbuilder.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtppollthread.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtprandom.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtprandomrand48.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtprandomrands.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtprandomurandom.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtprawpacket.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtpsession.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtpsessionparams.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtpsessionsources.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtpsourcedata.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtpsources.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtpstructs.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtptimeutilities.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtptransmitter.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtptypes_win.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/cmake/src/rtptypes.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtpudpv4transmitter.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtpudpv6transmitter.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtpbyteaddress.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtpexternaltransmitter.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtpsecuresession.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtpsocketutil.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtpabortdescriptors.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtpselect.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtptcpaddress.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/rtptcptransmitter.h"
    "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/src/extratransmitters/rtpfaketransmitter.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/lib/libjrtp.a")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/usr/local/lib" TYPE STATIC_LIBRARY FILES "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/cmake/src/libjrtp.a")
  if(EXISTS "$ENV{DESTDIR}/usr/local/lib/libjrtp.a" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/usr/local/lib/libjrtp.a")
    execute_process(COMMAND "/Applications/Xcode-15.0.0.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/ranlib" "$ENV{DESTDIR}/usr/local/lib/libjrtp.a")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/lib/libjrtp.3.11.2.dylib")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/usr/local/lib" TYPE SHARED_LIBRARY FILES "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/cmake/src/libjrtp.3.11.2.dylib")
  if(EXISTS "$ENV{DESTDIR}/usr/local/lib/libjrtp.3.11.2.dylib" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/usr/local/lib/libjrtp.3.11.2.dylib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/Applications/Xcode-15.0.0.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/strip" -x "$ENV{DESTDIR}/usr/local/lib/libjrtp.3.11.2.dylib")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/lib/libjrtp.dylib")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/usr/local/lib" TYPE SHARED_LIBRARY FILES "/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/cmake/src/libjrtp.dylib")
endif()

