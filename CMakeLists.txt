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
if(APPLE)
  find_package(OpenGL REQUIRED)
endif(APPLE)

# Find glm
set(GLM_INCLUDE_DIRS "lib/glm")

# Find json
set(JSON_INCLUDE_DIRS "lib/json")

# Find freetype
find_package(Freetype REQUIRED)


include_directories(source_common)
include_directories(${SDL2_INCLUDE_DIRS} ${SDL2main_INCLUDE_DIRS})
include_directories(${GLM_INCLUDE_DIRS})
include_directories(${SDL2_IMAGE_INCLUDE_DIRS})
include_directories(${JSON_INCLUDE_DIRS})
include_directories(${FREETYPE_INCLUDE_DIRS})

if(APPLE)
  include_directories(${OPENGL_INCLUDE_DIR})
endif(APPLE)

if(NOT APPLE)
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

if(NOT IOS_PLATFORM)
  add_subdirectory(source_ios)
else()
  add_subdirectory(source_desktop)
endif()

add_subdirectory(source_common)
add_subdirectory(lib/googletest)
add_subdirectory(source_test)

# Enable highest warning levels + treated as errors
if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /W4)
  target_compile_options(${PROJECT_NAME}_platform PRIVATE /W4)
  target_compile_options(${PROJECT_NAME}_test PRIVATE /W4)
  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
else(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
  target_compile_options(${PROJECT_NAME}_platform PRIVATE -Wall -Wextra -pedantic -Werror)
  target_compile_options(${PROJECT_NAME}_test PRIVATE -Wall -Wextra -pedantic -Werror)
endif(MSVC)

# Put these targets in the 'HiddenTargets' folder in the IDE. 
set_target_properties(gmock gmock_main gtest gtest_main Predators_lib PROPERTIES FOLDER HiddenTargets)

# Put these targets in the 'ProjectTargets' folder in the IDE.
set_target_properties(Predators Predators_platform Predators_test Predators_platform PROPERTIES FOLDER ProjectTargets)


