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
CMAKE_SOURCE_DIR = /Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/cmake

# Include any dependencies generated for this target.
include examples/CMakeFiles/example4.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include examples/CMakeFiles/example4.dir/compiler_depend.make

# Include the progress variables for this target.
include examples/CMakeFiles/example4.dir/progress.make

# Include the compile flags for this target's objects.
include examples/CMakeFiles/example4.dir/flags.make

examples/CMakeFiles/example4.dir/example4.cpp.o: examples/CMakeFiles/example4.dir/flags.make
examples/CMakeFiles/example4.dir/example4.cpp.o: /Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/examples/example4.cpp
examples/CMakeFiles/example4.dir/example4.cpp.o: examples/CMakeFiles/example4.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/cmake/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object examples/CMakeFiles/example4.dir/example4.cpp.o"
	cd /Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/cmake/examples && /Applications/Xcode-15.0.0.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT examples/CMakeFiles/example4.dir/example4.cpp.o -MF CMakeFiles/example4.dir/example4.cpp.o.d -o CMakeFiles/example4.dir/example4.cpp.o -c /Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/examples/example4.cpp

examples/CMakeFiles/example4.dir/example4.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/example4.dir/example4.cpp.i"
	cd /Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/cmake/examples && /Applications/Xcode-15.0.0.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/examples/example4.cpp > CMakeFiles/example4.dir/example4.cpp.i

examples/CMakeFiles/example4.dir/example4.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/example4.dir/example4.cpp.s"
	cd /Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/cmake/examples && /Applications/Xcode-15.0.0.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/examples/example4.cpp -o CMakeFiles/example4.dir/example4.cpp.s

# Object files for target example4
example4_OBJECTS = \
"CMakeFiles/example4.dir/example4.cpp.o"

# External object files for target example4
example4_EXTERNAL_OBJECTS =

examples/example4: examples/CMakeFiles/example4.dir/example4.cpp.o
examples/example4: examples/CMakeFiles/example4.dir/build.make
examples/example4: src/libjrtp.a
examples/example4: /opt/homebrew/lib/libsrtp2.dylib
examples/example4: examples/CMakeFiles/example4.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/cmake/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable example4"
	cd /Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/cmake/examples && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/example4.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
examples/CMakeFiles/example4.dir/build: examples/example4
.PHONY : examples/CMakeFiles/example4.dir/build

examples/CMakeFiles/example4.dir/clean:
	cd /Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/cmake/examples && $(CMAKE_COMMAND) -P CMakeFiles/example4.dir/cmake_clean.cmake
.PHONY : examples/CMakeFiles/example4.dir/clean

examples/CMakeFiles/example4.dir/depend:
	cd /Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/cmake && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2 /Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/examples /Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/cmake /Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/cmake/examples /Users/takeru/Documents/GitHub/connect-udp-library/jrtplib-3.11.2/cmake/examples/CMakeFiles/example4.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : examples/CMakeFiles/example4.dir/depend

