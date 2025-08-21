cmake_minimum_required(VERSION 3.20)
project(MyMetalApp LANGUAGES C CXX OBJC OBJCXX)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(3rd_party)

# demos
add_subdirectory(src/ch1)
add_subdirectory(src/ch2)
add_subdirectory(src/ch3)
add_subdirectory(src/ch4)
add_subdirectory(src/ch5)
add_subdirectory(src/ch6)
add_subdirectory(src/ch7)
add_subdirectory(src/ch8)
add_subdirectory(src/ch9)
add_subdirectory(src/ch10)
add_subdirectory(src/ch11)