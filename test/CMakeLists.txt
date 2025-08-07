
add_executable(tests test.cpp )

target_link_libraries(tests gtest)

include(GoogleTest)
gtest_discover_tests(tests)

