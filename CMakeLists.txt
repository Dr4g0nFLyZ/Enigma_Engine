# Minimum required version of CMake
cmake_minimum_required(VERSION 3.16)

# Define the project
project(HelloWorldQt6 LANGUAGES CXX C)

# Find the Qt6 package. We specifically need the 'Widgets' module for QPushButton.
find_package(Qt6 COMPONENTS Widgets REQUIRED)

# This looks for glfw3Config.cmake on your system
find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGLWidgets Sql)
find_package(glfw3 3.3 REQUIRED)
find_package(Freetype REQUIRED)

# Instead of find_package, look for the files directly
find_path(WASMTIME_INCLUDE_DIR NAMES wasmtime.h)
find_library(WASMTIME_LIBRARY NAMES wasmtime)

if(WASMTIME_INCLUDE_DIR AND WASMTIME_LIBRARY)
    # Create the target manually to match what your code expects
    add_library(wasmtime::wasmtime UNKNOWN IMPORTED)
    set_target_properties(wasmtime::wasmtime PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${WASMTIME_INCLUDE_DIR}"
        IMPORTED_LOCATION "${WASMTIME_LIBRARY}"
    )
    message(STATUS "Found Wasmtime: ${WASMTIME_LIBRARY}")
else()
    message(FATAL_ERROR "Wasmtime headers or library not found. Check your nix-shell.")
endif()

# Specify the C++ standard to use
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 1. Point to where your GLAD headers are located
include_directories(include) # Adjust this if your headers are in a different folder

add_executable(hello_world 
    main.cpp
)

# Add the executable
#add_executable(hello_world main.cpp)

# Optional: Find and link OpenGL (usually needed with GLFW)
find_package(OpenGL REQUIRED)

# Link the executable against the necessary Qt6 module with glfw.
target_link_libraries(hello_world PRIVATE Qt6::Widgets wasmtime::wasmtime glfw OpenGL::GL Qt6::OpenGLWidgets Qt6::Sql Freetype::Freetype)

# Set up the Qt properties for the executable (crucial for linking and deploying)
# This uses the settings defined by the find_package() command.
qt_standard_project_setup()
