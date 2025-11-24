add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/IR)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/runtime)

file(GLOB_RECURSE HPQPLLVM_HEADERS
    ${CMAKE_SOURCE_DIR}/include/*.h)

set(HPQPLLVM_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/Driver.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/IU.cc
    )

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")


include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

llvm_map_components_to_libnames(llvm_libs support core irreader)

add_library(hpqpllvm_lib STATIC ${HPQPLLVM_SOURCES} ${HPQPLLVM_HEADERS})
target_link_libraries(hpqpllvm_lib PUBLIC LLVM)
target_link_libraries(hpqpllvm_lib PUBLIC ir)
target_link_libraries(runtime PRIVATE ir)
target_link_libraries(hpqpllvm_lib PUBLIC runtime)


