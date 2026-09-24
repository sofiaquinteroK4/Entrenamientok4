cmake_minimum_required(VERSION 3.16)
project(entrenamiento CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

add_compile_options(-Wall -Wextra -Wpedantic)

option(ENABLE_SANITIZERS "Build with -fsanitize=address,undefined" OFF)
if(ENABLE_SANITIZERS)
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address,undefined)
endif()

enable_testing()

add_subdirectory(semana1-raii-smartpointers/dia1-3)
add_subdirectory(semana1-raii-smartpointers/dia4-5)
add_subdirectory(semana2-regla-de-cinco/dia1-5)
