add_executable(ch1 main.cpp ../misc/glfw_adapter.mm)
target_include_directories(ch1 PRIVATE ../misc)
target_link_libraries(ch1 PRIVATE glfw mtcpp "-framework Metal" "-framework QuartzCore")