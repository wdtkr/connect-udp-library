# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.27

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /opt/homebrew/Cellar/cmake/3.27.7/bin/cmake

# The command to remove a file.
RM = /opt/homebrew/Cellar/cmake/3.27.7/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/takeru/Documents/GitHub/connect-udp-library/library

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/takeru/Documents/GitHub/connect-udp-library/library/build

# Include any dependencies generated for this target.
include CMakeFiles/media-lib.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/media-lib.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/media-lib.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/media-lib.dir/flags.make

CMakeFiles/media-lib.dir/mediaReceiver.cpp.o: CMakeFiles/media-lib.dir/flags.make
CMakeFiles/media-lib.dir/mediaReceiver.cpp.o: /Users/takeru/Documents/GitHub/connect-udp-library/library/mediaReceiver.cpp
CMakeFiles/media-lib.dir/mediaReceiver.cpp.o: CMakeFiles/media-lib.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/takeru/Documents/GitHub/connect-udp-library/library/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/media-lib.dir/mediaReceiver.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/media-lib.dir/mediaReceiver.cpp.o -MF CMakeFiles/media-lib.dir/mediaReceiver.cpp.o.d -o CMakeFiles/media-lib.dir/mediaReceiver.cpp.o -c /Users/takeru/Documents/GitHub/connect-udp-library/library/mediaReceiver.cpp

CMakeFiles/media-lib.dir/mediaReceiver.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/media-lib.dir/mediaReceiver.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/takeru/Documents/GitHub/connect-udp-library/library/mediaReceiver.cpp > CMakeFiles/media-lib.dir/mediaReceiver.cpp.i

CMakeFiles/media-lib.dir/mediaReceiver.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/media-lib.dir/mediaReceiver.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/takeru/Documents/GitHub/connect-udp-library/library/mediaReceiver.cpp -o CMakeFiles/media-lib.dir/mediaReceiver.cpp.s

# Object files for target media-lib
media__lib_OBJECTS = \
"CMakeFiles/media-lib.dir/mediaReceiver.cpp.o"

# External object files for target media-lib
media__lib_EXTERNAL_OBJECTS =

libmedia-lib.so: CMakeFiles/media-lib.dir/mediaReceiver.cpp.o
libmedia-lib.so: CMakeFiles/media-lib.dir/build.make
libmedia-lib.so: /opt/homebrew/lib/libopus.dylib
libmedia-lib.so: CMakeFiles/media-lib.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/Users/takeru/Documents/GitHub/connect-udp-library/library/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX shared module libmedia-lib.so"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/media-lib.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/media-lib.dir/build: libmedia-lib.so
.PHONY : CMakeFiles/media-lib.dir/build

CMakeFiles/media-lib.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/media-lib.dir/cmake_clean.cmake
.PHONY : CMakeFiles/media-lib.dir/clean

CMakeFiles/media-lib.dir/depend:
	cd /Users/takeru/Documents/GitHub/connect-udp-library/library/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/takeru/Documents/GitHub/connect-udp-library/library /Users/takeru/Documents/GitHub/connect-udp-library/library /Users/takeru/Documents/GitHub/connect-udp-library/library/build /Users/takeru/Documents/GitHub/connect-udp-library/library/build /Users/takeru/Documents/GitHub/connect-udp-library/library/build/CMakeFiles/media-lib.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/media-lib.dir/depend

