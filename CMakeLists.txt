cmake_minimum_required(VERSION 3.10)
project(Predators)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/build_utils")

# Enable folder use in CMake
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Find SDL2
find_package(SDL2 REQUIRED COMPONENTS main)

# Find SDL2 Image
find_package(SDL2_image REQUIRED)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Find glm
set(GLM_INCLUDE_DIRS "lib/glm")

# Find json
set(JSON_INCLUDE_DIRS "lib/json")

# Find freetype
find_package(Freetype REQUIRED)

include_directories(source)
include_directories(${SDL2_INCLUDE_DIRS} ${SDL2main_INCLUDE_DIRS})
include_directories(${GLM_INCLUDE_DIRS})
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${SDL2_IMAGE_INCLUDE_DIRS})
include_directories(${JSON_INCLUDE_DIRS})
include_directories(${FREETYPE_INCLUDE_DIRS})

# Include other CmakeLists
add_subdirectory(source)
add_subdirectory(test)
add_subdirectory(lib/googletest)

# Copy DLLs to output folder on Windows
if(WIN32)
    foreach(DLL ${SDL2_DLLS})
        message("Copying ${DLL} to output folder")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND
            ${CMAKE_COMMAND} -E copy_if_different ${DLL} $<TARGET_FILE_DIR:${PROJECT_NAME}>)
    endforeach()
    
endif()

# Enable highest warning levels + treated as errors
if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /W4)
  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
else(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
endif(MSVC)

# Put these targets in the 'HiddenTargets' folder in the IDE.
set_target_properties(Predators_lib gmock gmock_main gtest gtest_main PROPERTIES FOLDER HiddenTargets)

# Put these targets in the 'ProjectTargets' folder in the IDE.
set_target_properties(Predators Predators_test PROPERTIES FOLDER ProjectTargets)

