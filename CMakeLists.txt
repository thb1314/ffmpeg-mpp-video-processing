cmake_minimum_required(VERSION 3.10)

# 项目名称
project(ffmpeg_demo)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(FFMPEG_DIR /userdata/ffmpeg)
# 指定 FFmpeg 的头文件目录
set(FFMPEG_INCLUDE_DIR ${FFMPEG_DIR}/binary/include) # 修改为你的 FFmpeg 头文件路径
message("FFMPEG_INCLUDE_DIR: " ${FFMPEG_INCLUDE_DIR})
include_directories(${FFMPEG_INCLUDE_DIR})
# 指定 FFmpeg 的库目录
set(FFMPEG_LIB_DIR ${FFMPEG_DIR}/binary/lib) # 修改为你的 FFmpeg 库文件路径
link_directories(${FFMPEG_LIB_DIR})

# 使用 find 查找所有ffmpeg依赖的 .so 文件所在的目录
execute_process(
    COMMAND find ${FFMPEG_DIR}/other_binary -name "lib" -type d
    OUTPUT_VARIABLE FFMPEG_DEP_LIB_DIRS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
string(REPLACE "\n" ";" FOUND_PATHS_LIST "${FFMPEG_DEP_LIB_DIRS}")

foreach(PATH ${FOUND_PATHS_LIST})
    link_directories(${PATH})
    message("add ffmpeg dep lib ${PATH}")
endforeach()

message("FFMPEG_DEP_LIB_DIRS: " ${FFMPEG_LINK_DEP_LIB_DIRS})
string(REPLACE "\n" ":" FFMPEG_LD_DEP_LIB_DIRS "${FFMPEG_DEP_LIB_DIRS}")
set(FFMPEG_LD_DEP_LIB_DIRS ${FFMPEG_LD_DEP_LIB_DIRS}:${FFMPEG_LIB_DIR})

# 使用 file(GLOB ...) 查找 src/ 文件夹下的所有 .cpp 文件
set(CORE_DIR ${CMAKE_SOURCE_DIR}/core)
file(GLOB CORE_SOURCES "${CORE_DIR}/*.cpp")
add_library(core SHARED ${CORE_SOURCES})
target_link_libraries(core PRIVATE avutil)
target_include_directories(core 
    PRIVATE 
    ${CORE_DIR} 
    ${FFMPEG_INCLUDE_DIR}
)


set(ENCODERS_DIR ${CMAKE_SOURCE_DIR}/encoders)
file(GLOB ENCODER_SOURCES "${ENCODERS_DIR}/*.cpp")
add_library(encoders SHARED ${ENCODER_SOURCES})
target_link_libraries(encoders PRIVATE avutil avformat avcodec swscale swresample)
target_include_directories(encoders 
    PRIVATE
    ${ENCODERS_DIR}
    ${CORE_DIR}
)


set(PROVIDERS_DIR ${CMAKE_SOURCE_DIR}/providers)
file(GLOB PROVIDERS_SOURCES "${PROVIDERS_DIR}/*.cpp")
add_library(providers SHARED ${PROVIDERS_SOURCES})
target_include_directories(providers PRIVATE ${PROVIDERS_DIR} ${CORE_DIR})



if(UNIX)
    message("use pthread")
    target_link_libraries(providers PRIVATE pthread avutil avformat)
else()
    target_link_libraries(providers PRIVATE avutil avformat)
endif()



# 定义目标
add_executable(ffmpeg_demo src/main.cpp)
target_include_directories(ffmpeg_demo PRIVATE ${PROVIDERS_DIR} ${ENCODERS_DIR} ${CORE_DIR} )
# 链接共享库和 FFmpeg 库到可执行文件
target_link_libraries(ffmpeg_demo PRIVATE core encoders providers avutil avformat avcodec)


# 创建运行脚本
set(RUN_SCRIPT ${CMAKE_BINARY_DIR}/run_ffmpeg_demo.sh)
file(WRITE ${RUN_SCRIPT} "#!/bin/bash\n")
file(WRITE ${RUN_SCRIPT} "\nCURDIR=\$(dirname \$(realpath $0))\n")
file(APPEND ${RUN_SCRIPT} "export LD_LIBRARY_PATH=${FFMPEG_LD_DEP_LIB_DIRS}:\$LD_LIBRARY_PATH\n")
file(APPEND ${RUN_SCRIPT} "\$CURDIR/ffmpeg_demo \"\$@\"\n")
file(COPY ${RUN_SCRIPT} DESTINATION ${CMAKE_BINARY_DIR})

add_custom_target(run
    COMMAND ${CMAKE_COMMAND} -E echo "Running the ffmpeg_demo script..."
    COMMAND bash ${RUN_SCRIPT}
    DEPENDS ffmpeg_demo
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running ffmpeg_demo with auto-detected library paths"
)