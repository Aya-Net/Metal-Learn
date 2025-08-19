cmake_minimum_required(VERSION 3.20)
project(MyMetalApp LANGUAGES C CXX OBJC OBJCXX)

# 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_OBJCXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OBJCXX_STANDARD_REQUIRED ON)

# 可选：设置最低系统版本（与 Xcode 项目类似）
set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "Minimum OS X deployment version")

# 源文件
set(SRC
        src/main.mm
)

add_executable(${PROJECT_NAME} ${SRC})

# ---- Metal 编译（.metal -> .air -> .metallib） ----
# 1) 列出你的 metal 源（可多个）
set(METAL_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/basic.metal
)

# 2) 输出目录
set(METAL_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/metal)
file(MAKE_DIRECTORY ${METAL_BUILD_DIR})

# 3) 为每个 .metal 生成 .air
set(METAL_AIRS "")
foreach(METAL_SRC ${METAL_SOURCES})
    get_filename_component(METAL_NAME ${METAL_SRC} NAME_WE)
    set(AIR_FILE ${METAL_BUILD_DIR}/${METAL_NAME}.air)
    add_custom_command(
            OUTPUT ${AIR_FILE}
            COMMAND xcrun -sdk macosx metal -c ${METAL_SRC} -o ${AIR_FILE}
            DEPENDS ${METAL_SRC}
            COMMENT "Compiling Metal shader ${METAL_SRC} -> ${AIR_FILE}"
            VERBATIM
    )
    list(APPEND METAL_AIRS ${AIR_FILE})
endforeach()

# 4) 链接 .air -> .metallib
set(METAL_LIB ${METAL_BUILD_DIR}/default.metallib)
add_custom_command(
        OUTPUT ${METAL_LIB}
        COMMAND xcrun -sdk macosx metallib ${METAL_AIRS} -o ${METAL_LIB}
        DEPENDS ${METAL_AIRS}
        COMMENT "Linking Metal AIRs -> ${METAL_LIB}"
        VERBATIM
)

# 5) 生成一个自定义目标，确保构建 exe 前先构建 metallib
add_custom_target(metal_lib ALL DEPENDS ${METAL_LIB})
add_dependencies(${PROJECT_NAME} metal_lib)

# 6) 运行时把 .metallib 复制到可执行目录（与二进制同级，便于加载）
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${METAL_LIB} $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMENT "Copy metallib to runtime dir"
)

# ---- 链接系统框架 ----
# Metal + MetalKit + Cocoa + QuartzCore 基本够用
find_library(COCOA_FRAMEWORK Cocoa)
find_library(METAL_FRAMEWORK Metal)
find_library(METALKIT_FRAMEWORK MetalKit)
find_library(QUARTZCORE_FRAMEWORK QuartzCore)
find_library(FOUNDATION_FRAMEWORK Foundation)

target_link_libraries(${PROJECT_NAME} PRIVATE
        ${COCOA_FRAMEWORK}
        ${FOUNDATION_FRAMEWORK}
        ${QUARTZCORE_FRAMEWORK}
        ${METAL_FRAMEWORK}
        ${METALKIT_FRAMEWORK}
)

# 可选：把可执行输出到 bin/
set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# 可选：Debug 警告
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
    target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Debug>:-Wall -Wextra -Wpedantic>
    )
endif()
