# Install script for directory: /Users/takeru/Documents/GitHub/uvgRTP

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
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
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
  set(CMAKE_OBJDUMP "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/objdump")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/takeru/Documents/GitHub/uvgRTP/build/_deps/googletest-build/cmake_install.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/Users/takeru/Documents/GitHub/uvgRTP/build/uvgrtp.pc")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "uvgrtp_Runtime" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/Users/takeru/Documents/GitHub/uvgRTP/build/libuvgrtp.a")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libuvgrtp.a" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libuvgrtp.a")
    execute_process(COMMAND "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/ranlib" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libuvgrtp.a")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "uvgrtp_Develop" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/uvgrtp" TYPE FILE FILES
    "/Users/takeru/Documents/GitHub/uvgRTP/include/uvgrtp/clock.hh"
    "/Users/takeru/Documents/GitHub/uvgRTP/include/uvgrtp/context.hh"
    "/Users/takeru/Documents/GitHub/uvgRTP/include/uvgrtp/frame.hh"
    "/Users/takeru/Documents/GitHub/uvgRTP/include/uvgrtp/lib.hh"
    "/Users/takeru/Documents/GitHub/uvgRTP/include/uvgrtp/media_stream.hh"
    "/Users/takeru/Documents/GitHub/uvgRTP/include/uvgrtp/rtcp.hh"
    "/Users/takeru/Documents/GitHub/uvgRTP/include/uvgrtp/session.hh"
    "/Users/takeru/Documents/GitHub/uvgRTP/include/uvgrtp/util.hh"
    "/Users/takeru/Documents/GitHub/uvgRTP/include/uvgrtp/version.hh"
    "/Users/takeru/Documents/GitHub/uvgRTP/include/uvgrtp/wrapper_c.hh"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/uvgrtp/uvgrtpTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/uvgrtp/uvgrtpTargets.cmake"
         "/Users/takeru/Documents/GitHub/uvgRTP/build/CMakeFiles/Export/ec286d104ed254c2943602fc42ca539f/uvgrtpTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/uvgrtp/uvgrtpTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/uvgrtp/uvgrtpTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/uvgrtp" TYPE FILE FILES "/Users/takeru/Documents/GitHub/uvgRTP/build/CMakeFiles/Export/ec286d104ed254c2943602fc42ca539f/uvgrtpTargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/uvgrtp" TYPE FILE FILES "/Users/takeru/Documents/GitHub/uvgRTP/build/CMakeFiles/Export/ec286d104ed254c2943602fc42ca539f/uvgrtpTargets-release.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "uvgRTPMain" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/uvgrtp" TYPE FILE FILES
    "/Users/takeru/Documents/GitHub/uvgRTP/cmake/uvgrtpConfig.cmake"
    "/Users/takeru/Documents/GitHub/uvgRTP/cmake/uvgrtpMacros.cmake"
    "/Users/takeru/Documents/GitHub/uvgRTP/build/uvgrtp/uvgrtpConfigVersion.cmake"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/takeru/Documents/GitHub/uvgRTP/build/packaging/cmake_install.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/Users/takeru/Documents/GitHub/uvgRTP/build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
