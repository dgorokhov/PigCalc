cmake_minimum_required(VERSION 3.25)
project(archie_tests LANGUAGES CXX)

# ---- 1. СБОРКА ОБЫЧНЫХ ТЕСТОВ (Google Test) ----
find_package(GTest REQUIRED)

add_executable(pgcalc_test_runner
    test_main.cpp
    test_common.cpp
)
target_link_libraries(pgcalc_test_runner PRIVATE GTest::gtest_main pgcalc::core)
target_compile_options(pgcalc_test_runner PRIVATE $<TARGET_PROPERTY:pgcalc_core,COMPILE_OPTIONS>)
target_link_options(pgcalc_test_runner PRIVATE $<TARGET_PROPERTY:pgcalc_core,LINK_OPTIONS>)
add_test(NAME pgcalc_unit_tests COMMAND pgcalc_test_runner)
