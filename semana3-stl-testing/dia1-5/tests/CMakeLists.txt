# One executable per test file, one add_test() per executable. The
# loop avoids repeating the same three lines for every file (compare
# with the week 1-2 tests/CMakeLists.txt).
#
# Every test gets the "semana3" label, so this week's tests can be run
# on their own with `ctest -L semana3`.
foreach(name iteradores algoritmos regresion)
  add_executable(test_stl_${name} test_${name}.cpp)
  target_link_libraries(test_stl_${name} PRIVATE stl_buffer_lib)
  add_test(NAME stl_${name} COMMAND test_stl_${name})
  set_tests_properties(stl_${name} PROPERTIES LABELS semana3)
endforeach()
