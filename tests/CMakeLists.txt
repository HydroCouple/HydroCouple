add_executable(hydrocouple_tests
    test_core_types.cpp
    test_hydrocouple.cpp
    test_spatial_wkb.cpp
)

target_link_libraries(hydrocouple_tests PRIVATE
    HydroCouple
    GTest::gtest
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(hydrocouple_tests)
