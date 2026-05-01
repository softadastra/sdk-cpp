cmake_minimum_required(VERSION 3.20)

add_executable(sdk_cpp_tests
  ClientOptionsTests.cpp
  ResultTests.cpp
  ClientTests.cpp
)

target_link_libraries(sdk_cpp_tests
  PRIVATE
    softadastra::sdk_cpp
)

target_compile_features(sdk_cpp_tests
  PRIVATE
    cxx_std_20
)

target_compile_options(sdk_cpp_tests
  PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

add_test(
  NAME sdk_cpp_tests
  COMMAND sdk_cpp_tests
)
