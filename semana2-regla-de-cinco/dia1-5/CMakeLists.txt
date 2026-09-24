add_library(buffer_lib INTERFACE)
target_include_directories(buffer_lib INTERFACE include)

add_executable(demo_semana2 examples/demo_semana2.cpp)
target_link_libraries(demo_semana2 PRIVATE buffer_lib)
target_compile_definitions(demo_semana2 PRIVATE BUFFER_TRACE)

add_subdirectory(tests)
