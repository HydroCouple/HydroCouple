add_executable(hydrocouple_tests
    test_hydrocouple.cpp
    test_spatial_wkb.cpp
)

target_include_directories(hydrocouple_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(hydrocouple_tests PRIVATE
    GTest::gtest
    GTest::gtest_main
)

target_compile_features(hydrocouple_tests PRIVATE cxx_std_17)

include(GoogleTest)
gtest_discover_tests(hydrocouple_tests)
