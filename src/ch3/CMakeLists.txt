set(APP_NAME ch3)
add_executable(${APP_NAME} main.cpp utils/mtlppimpl.cpp ../misc/glfw_adapter.mm
        utils/shader.cpp)
target_include_directories(${APP_NAME} PRIVATE ../misc)
target_link_libraries(${APP_NAME} PRIVATE glfw GLMHEADERONLY mtcpp "-framework Metal" "-framework QuartzCore")
