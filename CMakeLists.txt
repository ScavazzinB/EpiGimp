cmake_minimum_required(VERSION 3.16)
project(EpiGimp VERSION 1.0.0 LANGUAGES CXX)

# C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find required packages
find_package(PkgConfig REQUIRED)

# GTKmm 3.0 (C++ bindings for GTK)
pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)

# SDL2
pkg_check_modules(SDL2 REQUIRED sdl2)

# libpng
pkg_check_modules(PNG REQUIRED libpng)

# libjpeg
pkg_check_modules(JPEG REQUIRED libjpeg)

# Source files
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/CommandStack.cpp
    src/ImageCommand.cpp
)

# Header files
set(HEADERS
    include/epigimp/MainWindow.hpp
    include/epigimp/Command.hpp
    include/epigimp/CommandStack.hpp
    include/epigimp/ImageCommand.hpp
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${GTKMM_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
    ${PNG_INCLUDE_DIRS}
    ${JPEG_INCLUDE_DIRS}
)

# Link directories
target_link_directories(${PROJECT_NAME} PRIVATE
    ${GTKMM_LIBRARY_DIRS}
    ${SDL2_LIBRARY_DIRS}
    ${PNG_LIBRARY_DIRS}
    ${JPEG_LIBRARY_DIRS}
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${GTKMM_LIBRARIES}
    ${SDL2_LIBRARIES}
    ${PNG_LIBRARIES}
    ${JPEG_LIBRARIES}
)

# Compiler flags
target_compile_options(${PROJECT_NAME} PRIVATE
    ${GTKMM_CFLAGS_OTHER}
    -Wall
    -Wextra
    -Wpedantic
)
