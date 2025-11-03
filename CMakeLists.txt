cmake_minimum_required(VERSION 3.10)
project(oj_problem_017)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Prefer g++-13 if available
if (NOT CMAKE_CXX_COMPILER)
  find_program(GXX_13 g++-13)
  if (GXX_13)
    set(CMAKE_CXX_COMPILER ${GXX_13})
  endif()
endif()

add_executable(code main.cpp)

# Optimize for speed and reasonable binary size
if (CMAKE_BUILD_TYPE STREQUAL "Release")
  target_compile_options(code PRIVATE -O2 -pipe -static-libstdc++ -static-libgcc)
else()
  target_compile_options(code PRIVATE -O2 -pipe)
endif()
