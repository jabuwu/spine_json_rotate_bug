cmake_minimum_required(VERSION 3.0)
project(transpile_spine_bug)
file(GLOB SOURCES "spine-runtimes/spine-c/spine-c/src/**/*.c" "spine-runtimes/spine-sfml/c/src/spine/*.cpp")

#
# First download and extract SFML 2.5.1 for the respective OS we are on
#
set(DEPS_DIR "${CMAKE_CURRENT_LIST_DIR}/dependencies/")
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set(CMAKE_OSX_ARCHITECTURES x86_64)
	set(ONLY_ACTIVE_ARCH NO)
	set(SFML_URL "https://www.sfml-dev.org/files/SFML-2.5.1-macOS-clang.tar.gz")
	set(SFML_DIR ${DEPS_DIR}/SFML-2.5.1-macos-clang)
	if (NOT EXISTS "${SFML_DIR}")
		message("Downloading SFML for Mac OS X")
		file(DOWNLOAD "${SFML_URL}" "${DEPS_DIR}/sfml.tar.gz")
		execute_process(
				COMMAND ${CMAKE_COMMAND} -E tar xzf  ${DEPS_DIR}/sfml.tar.gz
				WORKING_DIRECTORY ${DEPS_DIR}
		)
		# copy freetype over to Frameworks/ so rpath resoultion works
		execute_process(
				COMMAND ${CMAKE_COMMAND} -E copy_directory ${SFML_DIR}/extlibs/freetype.framework ${SFML_DIR}/Frameworks/freetype.framework
				WORKING_DIRECTORY ${SFML_DIR}
		)
	endif()
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set(SFML_URL "https://www.sfml-dev.org/files/SFML-2.5.1-linux-gcc-64-bit.tar.gz")
	set(SFML_DIR ${DEPS_DIR}/SFML-2.5.1)
	if (NOT EXISTS ${SFML_DIR})
		message("Downloading SFML for Linux 64-bit")
		file(DOWNLOAD "${SFML_URL}" "${DEPS_DIR}/sfml.tar.gz")
		execute_process(
				COMMAND ${CMAKE_COMMAND} -E tar xzf  ${DEPS_DIR}/sfml.tar.gz
				WORKING_DIRECTORY ${DEPS_DIR}
		)
	endif()
else()
	set(SFML_URL "https://www.sfml-dev.org/files/SFML-2.5.1-windows-vc14-32-bit.zip")
	set(SFML_DIR ${DEPS_DIR}/SFML-2.5.1)
	if (NOT EXISTS ${SFML_DIR})
		message("Downloading SFML for Windows 32-bit")
		file(DOWNLOAD "${SFML_URL}" "${DEPS_DIR}/sfml.zip")
		execute_process(
				COMMAND ${CMAKE_COMMAND} -E tar x  ${DEPS_DIR}/sfml.zip
				WORKING_DIRECTORY ${DEPS_DIR}
		)
	endif()
endif()


# Define spine-sfml example executable
add_executable(spine src/main.cpp ${SOURCES})
target_include_directories(spine PUBLIC spine-runtimes/spine-c/spine-c/include)
target_include_directories(spine PUBLIC spine-runtimes/spine-sfml/c/src)

# Link in SFML libraries and OS dependencies like OpenGL
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	find_library(SFML SFML PATHS ${SFML_DIR}/Frameworks)
	find_library(SFML_SYSTEM "sfml-system" PATHS ${SFML_DIR}/Frameworks)
	find_library(SFML_WINDOW sfml-window PATHS ${SFML_DIR}/Frameworks)
	find_library(SFML_GRAPHICS sfml-graphics PATHS ${SFML_DIR}/Frameworks)
	target_link_libraries(spine ${SFML} ${SFML_SYSTEM} ${SFML_WINDOW} ${SFML_GRAPHICS})
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries(spine sfml-graphics sfml-window sfml-system)
else()
	set(SFML_LIBS ${SFML_DIR}/lib)
	target_link_libraries(spine ${SFML_LIBS}/sfml-main-d.lib)
	target_link_libraries(spine ${SFML_LIBS}/sfml-graphics-s-d.lib)
	target_link_libraries(spine ${SFML_LIBS}/sfml-window-s-d.lib)
	target_link_libraries(spine ${SFML_LIBS}/sfml-system-s-d.lib)
	target_link_libraries(spine ${SFML_LIBS}/freetype.lib)
	target_link_libraries(spine ${SFML_LIBS}/jpeg.lib)
	target_link_libraries(spine opengl32)
	target_link_libraries(spine gdi32)
	target_link_libraries(spine winmm)
	add_definitions(-DSFML_STATIC)
endif()
