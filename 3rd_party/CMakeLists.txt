
# glfw
option(GLFW_BUILD_DOCS OFF)
add_subdirectory(glfw-3.4)

# glm
add_subdirectory(glm)

# metal-cpp
add_library(mtcpp INTERFACE)
target_include_directories(mtcpp INTERFACE metal-cpp)
