cmake_minimum_required(VERSION 3.10.2)

project("ffmpegplay")


include_directories(include)

#添加一个系统查找目录
#添加系统环境变量
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}")

add_library( # Sets the name of the library.
        ffmpegplay
        SHARED
        native-lib.cpp)

find_library( # Sets the name of the path variable.
        log-lib
        log)

target_link_libraries( # Specifies the target library.
        ffmpegplay
        avcodec
        avfilter
        avformat
        avutil
        swresample
        swscale
        android
        ${log-lib})