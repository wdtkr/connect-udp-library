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
CMAKE_SOURCE_DIR = /Users/takeru/Documents/GitHub/uvgRTP

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/takeru/Documents/GitHub/uvgRTP/build

# Include any dependencies generated for this target.
include examples/CMakeFiles/sending.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include examples/CMakeFiles/sending.dir/compiler_depend.make

# Include the progress variables for this target.
include examples/CMakeFiles/sending.dir/progress.make

# Include the compile flags for this target's objects.
include examples/CMakeFiles/sending.dir/flags.make

examples/CMakeFiles/sending.dir/sending.cc.o: examples/CMakeFiles/sending.dir/flags.make
examples/CMakeFiles/sending.dir/sending.cc.o: /Users/takeru/Documents/GitHub/uvgRTP/examples/sending.cc
examples/CMakeFiles/sending.dir/sending.cc.o: examples/CMakeFiles/sending.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/takeru/Documents/GitHub/uvgRTP/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object examples/CMakeFiles/sending.dir/sending.cc.o"
	cd /Users/takeru/Documents/GitHub/uvgRTP/build/examples && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT examples/CMakeFiles/sending.dir/sending.cc.o -MF CMakeFiles/sending.dir/sending.cc.o.d -o CMakeFiles/sending.dir/sending.cc.o -c /Users/takeru/Documents/GitHub/uvgRTP/examples/sending.cc

examples/CMakeFiles/sending.dir/sending.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/sending.dir/sending.cc.i"
	cd /Users/takeru/Documents/GitHub/uvgRTP/build/examples && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/takeru/Documents/GitHub/uvgRTP/examples/sending.cc > CMakeFiles/sending.dir/sending.cc.i

examples/CMakeFiles/sending.dir/sending.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/sending.dir/sending.cc.s"
	cd /Users/takeru/Documents/GitHub/uvgRTP/build/examples && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/takeru/Documents/GitHub/uvgRTP/examples/sending.cc -o CMakeFiles/sending.dir/sending.cc.s

# Object files for target sending
sending_OBJECTS = \
"CMakeFiles/sending.dir/sending.cc.o"

# External object files for target sending
sending_EXTERNAL_OBJECTS =

examples/sending: examples/CMakeFiles/sending.dir/sending.cc.o
examples/sending: examples/CMakeFiles/sending.dir/build.make
examples/sending: libuvgrtp.a
examples/sending: examples/CMakeFiles/sending.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/Users/takeru/Documents/GitHub/uvgRTP/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable sending"
	cd /Users/takeru/Documents/GitHub/uvgRTP/build/examples && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/sending.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
examples/CMakeFiles/sending.dir/build: examples/sending
.PHONY : examples/CMakeFiles/sending.dir/build

examples/CMakeFiles/sending.dir/clean:
	cd /Users/takeru/Documents/GitHub/uvgRTP/build/examples && $(CMAKE_COMMAND) -P CMakeFiles/sending.dir/cmake_clean.cmake
.PHONY : examples/CMakeFiles/sending.dir/clean

examples/CMakeFiles/sending.dir/depend:
	cd /Users/takeru/Documents/GitHub/uvgRTP/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/takeru/Documents/GitHub/uvgRTP /Users/takeru/Documents/GitHub/uvgRTP/examples /Users/takeru/Documents/GitHub/uvgRTP/build /Users/takeru/Documents/GitHub/uvgRTP/build/examples /Users/takeru/Documents/GitHub/uvgRTP/build/examples/CMakeFiles/sending.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : examples/CMakeFiles/sending.dir/depend

