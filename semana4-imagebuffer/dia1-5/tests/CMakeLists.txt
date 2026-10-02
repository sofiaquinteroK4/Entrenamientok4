set(IMAGEBUFFER_TESTS construccion vacio copia movimiento)

foreach(name ${IMAGEBUFFER_TESTS})
  add_executable(test_imagebuffer_${name} test_${name}.cpp)
  target_link_libraries(test_imagebuffer_${name} PRIVATE imagebuffer_lib)
  add_test(NAME imagebuffer_${name} COMMAND test_imagebuffer_${name})
  set_tests_properties(imagebuffer_${name} PROPERTIES LABELS semana4)
endforeach()

# ---------------------------------------------------------------------
# Leak checks as CTest tests (label "memcheck").
#
# Each test executable, plus the demo, is run again under a leak
# checker, which exits with a non-zero code if it finds leaks:
# - valgrind, if it's installed (Linux);
# - otherwise, on macOS, the system `leaks` tool (valgrind doesn't run
#   on Apple Silicon, and Apple's ASan can't detect leaks).
# Skipped in the ENABLE_SANITIZERS build: ASan replaces malloc/free,
# which conflicts with both tools.
# ---------------------------------------------------------------------
if(NOT ENABLE_SANITIZERS)
  find_program(VALGRIND_EXECUTABLE valgrind)
  find_program(LEAKS_EXECUTABLE leaks)

  if(VALGRIND_EXECUTABLE)
    set(memcheck_command ${VALGRIND_EXECUTABLE} --leak-check=full
                         --error-exitcode=1 --quiet)
  elseif(APPLE AND LEAKS_EXECUTABLE)
    set(memcheck_command ${LEAKS_EXECUTABLE} --atExit --)
  endif()

  if(memcheck_command)
    set(memcheck_targets demo_imagebuffer)
    foreach(name ${IMAGEBUFFER_TESTS})
      list(APPEND memcheck_targets test_imagebuffer_${name})
    endforeach()

    foreach(target ${memcheck_targets})
      add_test(NAME ${target}_memcheck
               COMMAND ${memcheck_command} $<TARGET_FILE:${target}>)
      set_tests_properties(${target}_memcheck PROPERTIES LABELS "semana4;memcheck")
    endforeach()
  else()
    message(STATUS "semana4: neither valgrind nor leaks found, skipping memcheck tests")
  endif()
endif()
