set(APP_NAME ch7)
add_executable(${APP_NAME} main.cpp utils/mtlppimpl.cpp ../misc/glfw_adapter.mm
        utils/shader.cpp
        utils/camera.h)
target_include_directories(${APP_NAME} PRIVATE ../misc)
target_link_libraries(${APP_NAME} PRIVATE glfw GLMHEADERONLY mtcpp stb "-framework Metal" "-framework QuartzCore")

add_custom_target(${APP_NAME}_copy_assets
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:${APP_NAME}>/../../assets
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:${APP_NAME}>/shaders
        USES_TERMINAL
)

add_dependencies(${APP_NAME} ${APP_NAME}_copy_assets)