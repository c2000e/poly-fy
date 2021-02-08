
find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)

add_custom_target(
    copy_shader_files
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR})

add_executable(poly_fy glad.c main.cc polygon.cc)
target_compile_features(poly_fy PRIVATE cxx_std_11)
target_include_directories(poly_fy PRIVATE ../include ${OPENGL_INCLUDE_DIRS} ${SDL2_INCLUDE_DIRS})
target_link_libraries(poly_fy PRIVATE ${OPENGL_LIBRARIES} ${SDL2_LIBRARIES} ${CMAKE_DL_LIBS})

add_dependencies(poly_fy copy_shader_files)
