# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
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
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/student/Desktop/studia/pscr

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/student/Desktop/studia/pscr/build

# Include any dependencies generated for this target.
include CMakeFiles/udp_client.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/udp_client.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/udp_client.dir/flags.make

CMakeFiles/udp_client.dir/udp_client.c.o: CMakeFiles/udp_client.dir/flags.make
CMakeFiles/udp_client.dir/udp_client.c.o: ../udp_client.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/student/Desktop/studia/pscr/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/udp_client.dir/udp_client.c.o"
	/usr/bin/clang $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/udp_client.dir/udp_client.c.o   -c /home/student/Desktop/studia/pscr/udp_client.c

CMakeFiles/udp_client.dir/udp_client.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/udp_client.dir/udp_client.c.i"
	/usr/bin/clang $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/student/Desktop/studia/pscr/udp_client.c > CMakeFiles/udp_client.dir/udp_client.c.i

CMakeFiles/udp_client.dir/udp_client.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/udp_client.dir/udp_client.c.s"
	/usr/bin/clang $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/student/Desktop/studia/pscr/udp_client.c -o CMakeFiles/udp_client.dir/udp_client.c.s

# Object files for target udp_client
udp_client_OBJECTS = \
"CMakeFiles/udp_client.dir/udp_client.c.o"

# External object files for target udp_client
udp_client_EXTERNAL_OBJECTS =

udp_client: CMakeFiles/udp_client.dir/udp_client.c.o
udp_client: CMakeFiles/udp_client.dir/build.make
udp_client: CMakeFiles/udp_client.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/student/Desktop/studia/pscr/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable udp_client"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/udp_client.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/udp_client.dir/build: udp_client

.PHONY : CMakeFiles/udp_client.dir/build

CMakeFiles/udp_client.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/udp_client.dir/cmake_clean.cmake
.PHONY : CMakeFiles/udp_client.dir/clean

CMakeFiles/udp_client.dir/depend:
	cd /home/student/Desktop/studia/pscr/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/student/Desktop/studia/pscr /home/student/Desktop/studia/pscr /home/student/Desktop/studia/pscr/build /home/student/Desktop/studia/pscr/build /home/student/Desktop/studia/pscr/build/CMakeFiles/udp_client.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/udp_client.dir/depend

