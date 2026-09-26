cmake_minimum_required(VERSION 3.10)
project(MyClangProject CXX)

set(CMAKE_CXX_STANDARD 17) # Or your desired C++ standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source and header directories
include_directories(include)
# add_subdirectory(src)
add_executable(my_executable src/main.cpp)
target_sources(my_executable PRIVATE 
    src/Schedule.cpp
    src/threading.cpp
    )

# # Integrate clang-format
# find_program(CLANG_FORMAT_EXECUTABLE clang-format)
# if(CLANG_FORMAT_EXECUTABLE)
#     add_custom_target(format
#         COMMAND ${CLANG_FORMAT_EXECUTABLE} -style=file -i ${CMAKE_SOURCE_DIR}/src/*.cpp ${CMAKE_SOURCE_DIR}/include/*.h
#         COMMENT "Formatting C++ files with clang-format"
#     )
# endif()

# # Integrate clang-tidy (optional)
# find_program(CLANG_TIDY_EXECUTABLE clang-tidy)
# if(CLANG_TIDY_EXECUTABLE)
#     set_target_properties(my_executable PROPERTIES CXX_CLANG_TIDY ${CLANG_TIDY_EXECUTABLE})
# endif()

# # Add tests (if using GTest)
# enable_testing()
# find_package(GTest CONFIG REQUIRED)
# add_executable(my_tests test/my_tests.cpp)
# target_link_libraries(my_tests GTest::gtest_main)