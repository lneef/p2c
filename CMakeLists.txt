cmake_minimum_required(VERSION 3.14)
set(CMAKE_CXX_STANDARD 20)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
    
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(LLVM_TARGETS_TO_BUILD "all")
list(PREPEND CMAKE_PREFIX_PATH "/usr/lib/llvm-18/cmake")
add_compile_options(-msse4.2)
project(HPQPLLVM VERSION 0.0.1)

include_directories(include)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib)

add_executable(hpqpllvm main.cc)
target_link_libraries(hpqpllvm PUBLIC hpqpllvm_lib)

include(gtest)
enable_testing()
add_subdirectory(${CMAKE_SOURCE_DIR}/unittests)




