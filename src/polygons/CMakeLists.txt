add_library(polygons STATIC polygon.cc)

target_compile_features(polygons PUBLIC cxx_std_11)
target_include_directories(polygons PUBLIC ../../include)
