# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/tcm/hardware

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/tcm/hardware/build

# Include any dependencies generated for this target.
include JetsonGPIO/tests/CMakeFiles/test_format.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include JetsonGPIO/tests/CMakeFiles/test_format.dir/compiler_depend.make

# Include the progress variables for this target.
include JetsonGPIO/tests/CMakeFiles/test_format.dir/progress.make

# Include the compile flags for this target's objects.
include JetsonGPIO/tests/CMakeFiles/test_format.dir/flags.make

JetsonGPIO/tests/CMakeFiles/test_format.dir/unit-tests/test_format.cpp.o: JetsonGPIO/tests/CMakeFiles/test_format.dir/flags.make
JetsonGPIO/tests/CMakeFiles/test_format.dir/unit-tests/test_format.cpp.o: ../JetsonGPIO/tests/unit-tests/test_format.cpp
JetsonGPIO/tests/CMakeFiles/test_format.dir/unit-tests/test_format.cpp.o: JetsonGPIO/tests/CMakeFiles/test_format.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/tcm/hardware/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object JetsonGPIO/tests/CMakeFiles/test_format.dir/unit-tests/test_format.cpp.o"
	cd /home/tcm/hardware/build/JetsonGPIO/tests && /usr/bin/aarch64-linux-gnu-g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT JetsonGPIO/tests/CMakeFiles/test_format.dir/unit-tests/test_format.cpp.o -MF CMakeFiles/test_format.dir/unit-tests/test_format.cpp.o.d -o CMakeFiles/test_format.dir/unit-tests/test_format.cpp.o -c /home/tcm/hardware/JetsonGPIO/tests/unit-tests/test_format.cpp

JetsonGPIO/tests/CMakeFiles/test_format.dir/unit-tests/test_format.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test_format.dir/unit-tests/test_format.cpp.i"
	cd /home/tcm/hardware/build/JetsonGPIO/tests && /usr/bin/aarch64-linux-gnu-g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/tcm/hardware/JetsonGPIO/tests/unit-tests/test_format.cpp > CMakeFiles/test_format.dir/unit-tests/test_format.cpp.i

JetsonGPIO/tests/CMakeFiles/test_format.dir/unit-tests/test_format.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test_format.dir/unit-tests/test_format.cpp.s"
	cd /home/tcm/hardware/build/JetsonGPIO/tests && /usr/bin/aarch64-linux-gnu-g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/tcm/hardware/JetsonGPIO/tests/unit-tests/test_format.cpp -o CMakeFiles/test_format.dir/unit-tests/test_format.cpp.s

JetsonGPIO/tests/CMakeFiles/test_format.dir/TestUtility.cpp.o: JetsonGPIO/tests/CMakeFiles/test_format.dir/flags.make
JetsonGPIO/tests/CMakeFiles/test_format.dir/TestUtility.cpp.o: ../JetsonGPIO/tests/TestUtility.cpp
JetsonGPIO/tests/CMakeFiles/test_format.dir/TestUtility.cpp.o: JetsonGPIO/tests/CMakeFiles/test_format.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/tcm/hardware/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object JetsonGPIO/tests/CMakeFiles/test_format.dir/TestUtility.cpp.o"
	cd /home/tcm/hardware/build/JetsonGPIO/tests && /usr/bin/aarch64-linux-gnu-g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT JetsonGPIO/tests/CMakeFiles/test_format.dir/TestUtility.cpp.o -MF CMakeFiles/test_format.dir/TestUtility.cpp.o.d -o CMakeFiles/test_format.dir/TestUtility.cpp.o -c /home/tcm/hardware/JetsonGPIO/tests/TestUtility.cpp

JetsonGPIO/tests/CMakeFiles/test_format.dir/TestUtility.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test_format.dir/TestUtility.cpp.i"
	cd /home/tcm/hardware/build/JetsonGPIO/tests && /usr/bin/aarch64-linux-gnu-g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/tcm/hardware/JetsonGPIO/tests/TestUtility.cpp > CMakeFiles/test_format.dir/TestUtility.cpp.i

JetsonGPIO/tests/CMakeFiles/test_format.dir/TestUtility.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test_format.dir/TestUtility.cpp.s"
	cd /home/tcm/hardware/build/JetsonGPIO/tests && /usr/bin/aarch64-linux-gnu-g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/tcm/hardware/JetsonGPIO/tests/TestUtility.cpp -o CMakeFiles/test_format.dir/TestUtility.cpp.s

# Object files for target test_format
test_format_OBJECTS = \
"CMakeFiles/test_format.dir/unit-tests/test_format.cpp.o" \
"CMakeFiles/test_format.dir/TestUtility.cpp.o"

# External object files for target test_format
test_format_EXTERNAL_OBJECTS =

JetsonGPIO/tests/test_format: JetsonGPIO/tests/CMakeFiles/test_format.dir/unit-tests/test_format.cpp.o
JetsonGPIO/tests/test_format: JetsonGPIO/tests/CMakeFiles/test_format.dir/TestUtility.cpp.o
JetsonGPIO/tests/test_format: JetsonGPIO/tests/CMakeFiles/test_format.dir/build.make
JetsonGPIO/tests/test_format: JetsonGPIO/libJetsonGPIO.so.1.2.5
JetsonGPIO/tests/test_format: JetsonGPIO/tests/CMakeFiles/test_format.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/tcm/hardware/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable test_format"
	cd /home/tcm/hardware/build/JetsonGPIO/tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test_format.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
JetsonGPIO/tests/CMakeFiles/test_format.dir/build: JetsonGPIO/tests/test_format
.PHONY : JetsonGPIO/tests/CMakeFiles/test_format.dir/build

JetsonGPIO/tests/CMakeFiles/test_format.dir/clean:
	cd /home/tcm/hardware/build/JetsonGPIO/tests && $(CMAKE_COMMAND) -P CMakeFiles/test_format.dir/cmake_clean.cmake
.PHONY : JetsonGPIO/tests/CMakeFiles/test_format.dir/clean

JetsonGPIO/tests/CMakeFiles/test_format.dir/depend:
	cd /home/tcm/hardware/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/tcm/hardware /home/tcm/hardware/JetsonGPIO/tests /home/tcm/hardware/build /home/tcm/hardware/build/JetsonGPIO/tests /home/tcm/hardware/build/JetsonGPIO/tests/CMakeFiles/test_format.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : JetsonGPIO/tests/CMakeFiles/test_format.dir/depend

