add_library(gl_helper STATIC framebuffer.cc shader.cc texture.cc)

target_compile_features(gl_helper PUBLIC cxx_std_11)
target_include_directories(gl_helper PUBLIC ../../include)
