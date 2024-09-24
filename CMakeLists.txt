cmake_minimum_required(VERSION 3.28)
project(GLCollection)

set(CMAKE_CXX_STANDARD 17)

include(CheckCXXCompilerFlag)

message(STATUS "Compiler")
message(STATUS ">> ID      : ${CMAKE_CXX_COMPILER_ID}")
message(STATUS ">> Version : ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS ">> Path    : ${CMAKE_CXX_COMPILER}")

if (${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
    check_cxx_compiler_flag(-std=c++17 cxx_17)
    check_cxx_compiler_flag(-Wall high_warning_level)
elseif (${CMAKE_CXX_COMPILER_ID} MATCHES GNU)
    check_cxx_compiler_flag(-std=gnu++17 cxx_17)
    check_cxx_compiler_flag(-Wextra high_warning_level)
endif ()

configure_file(
        ${CMAKE_SOURCE_DIR}/common/include/project_constants.h.in
        ${CMAKE_SOURCE_DIR}/project_constants.h @ONLY
)

include_directories("${CMAKE_SOURCE_DIR}/common/include")
include_directories("${CMAKE_SOURCE_DIR}/3rd_party/glad/include")
include_directories("${CMAKE_SOURCE_DIR}/3rd_party/glfw3/include")
include_directories("${CMAKE_SOURCE_DIR}/3rd_party/glm")
include_directories("${CMAKE_SOURCE_DIR}/3rd_party/freeimage/include")
include_directories("${CMAKE_SOURCE_DIR}/3rd_party/ffmpeg/include")
link_directories("${CMAKE_SOURCE_DIR}/3rd_party/glad/lib/linux")
link_directories("${CMAKE_SOURCE_DIR}/3rd_party/glfw3/lib/linux")
link_directories("${CMAKE_SOURCE_DIR}/3rd_party/freeimage/lib/linux")
link_directories("${CMAKE_SOURCE_DIR}/3rd_party/ffmpeg/lib/linux")

set(
        ALL_LIBS
        glad
        glfw3
        pthread
        dl
        X11
        freeimage

        avdevice
        avfilter
        avformat
        avcodec
        swresample
        swscale
        avutil
        x264
)

set(
        COMMON_FILES
        common/source/light.cpp
        common/source/camera.cpp
        common/source/object.cpp
        common/source/shader.cpp
        common/source/file_decoder.cpp
        common/source/video_reader.cpp
)

# 01 Lighting
add_executable(01_lighting 01_lighting/01_lighting.cpp ${COMMON_FILES})
target_link_libraries(01_lighting ${ALL_LIBS})

# 02 Projector
add_executable(02_projector 02_projector/02_projector.cpp ${COMMON_FILES})
target_link_libraries(02_projector ${ALL_LIBS})

# 03 Gimbal Lock
add_executable(03_gimbal_lock 03_gimbal_lock/03_gimbal_lock.cpp ${COMMON_FILES})
target_link_libraries(03_gimbal_lock ${ALL_LIBS})