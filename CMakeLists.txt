cmake_minimum_required(VERSION 3.20)
project(MyMetalApp LANGUAGES C CXX OBJC OBJCXX)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(3rd_party)

function(add_copy_assets TARGET_NAME SRC_DIR)
    add_custom_target(${TARGET_NAME}_copy_assets
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:${TARGET_NAME}>/../../assets
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${SRC_DIR}/shaders
            $<TARGET_FILE_DIR:${TARGET_NAME}>/shaders
            USES_TERMINAL
    )
    add_dependencies(${TARGET_NAME} ${TARGET_NAME}_copy_assets)
endfunction()

# demos
add_subdirectory(src/ch1)
add_subdirectory(src/ch2)
add_subdirectory(src/ch3)
add_copy_assets(ch3 ${CMAKE_SOURCE_DIR}/src/ch3)

add_subdirectory(src/ch4)
add_copy_assets(ch4 ${CMAKE_SOURCE_DIR}/src/ch4)

add_subdirectory(src/ch5)
add_copy_assets(ch5 ${CMAKE_SOURCE_DIR}/src/ch5)

add_subdirectory(src/ch6)
add_copy_assets(ch6 ${CMAKE_SOURCE_DIR}/src/ch6)

add_subdirectory(src/ch7)
add_copy_assets(ch7 ${CMAKE_SOURCE_DIR}/src/ch7)

add_subdirectory(src/ch8)
add_copy_assets(ch8 ${CMAKE_SOURCE_DIR}/src/ch8)

add_subdirectory(src/ch9)
add_copy_assets(ch9 ${CMAKE_SOURCE_DIR}/src/ch9)

add_subdirectory(src/ch10)
add_copy_assets(ch10 ${CMAKE_SOURCE_DIR}/src/ch10)

add_subdirectory(src/ch11)
add_copy_assets(ch11 ${CMAKE_SOURCE_DIR}/src/ch11)

add_subdirectory(src/ch12)
add_copy_assets(ch12 ${CMAKE_SOURCE_DIR}/src/ch12)

add_subdirectory(src/ch13)
add_copy_assets(ch13 ${CMAKE_SOURCE_DIR}/src/ch13)

add_subdirectory(src/ch14)
add_copy_assets(ch14 ${CMAKE_SOURCE_DIR}/src/ch14)

add_subdirectory(src/ch15)
add_copy_assets(ch15 ${CMAKE_SOURCE_DIR}/src/ch15)

add_subdirectory(src/ch16)
add_copy_assets(ch16 ${CMAKE_SOURCE_DIR}/src/ch16)

add_subdirectory(src/ch17)
add_copy_assets(ch17 ${CMAKE_SOURCE_DIR}/src/ch17)