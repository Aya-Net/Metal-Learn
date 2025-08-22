set(APP_NAME ch11)
add_executable(${APP_NAME} main.cpp utils/mtlppimpl.cpp ../misc/glfw_adapter.mm
        utils/shader.cpp
        utils/camera.h)
target_include_directories(${APP_NAME} PRIVATE ../misc)
target_link_libraries(${APP_NAME} PRIVATE glfw GLMHEADERONLY mtcpp stb "-framework Metal" "-framework QuartzCore")
