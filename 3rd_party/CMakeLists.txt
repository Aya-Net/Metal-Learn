
# glfw
option(GLFW_BUILD_DOCS OFF)
add_subdirectory(glfw-3.4)

# glm
add_library(GLMHEADERONLY INTERFACE)
target_include_directories(GLMHEADERONLY INTERFACE glm)

# metal-cpp
add_library(mtcpp INTERFACE)
target_include_directories(mtcpp INTERFACE metal-cpp)

# stb
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE stb)

# assimp
add_subdirectory(assimp)