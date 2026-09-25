add_library(stl_buffer_lib INTERFACE)
target_include_directories(stl_buffer_lib INTERFACE include)

add_executable(demo_semana3 examples/demo_semana3.cpp)
target_link_libraries(demo_semana3 PRIVATE stl_buffer_lib)

add_subdirectory(tests)
