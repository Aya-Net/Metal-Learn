set(APP_NAME ch2)
add_executable(${APP_NAME} main.cpp ../misc/glfw_adapter.mm)
target_include_directories(${APP_NAME} PRIVATE ../misc)
target_link_libraries(${APP_NAME} PRIVATE glfw mtcpp "-framework Metal" "-framework QuartzCore")