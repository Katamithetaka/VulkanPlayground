cmake_minimum_required(VERSION 3.9)

project(Modules
        VERSION 0.0.1
        DESCRIPTION "Shared library modules")
include(CTest)

set(CMAKE_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/build)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_OUTPUT_DIR}/modules/)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_OUTPUT_DIR}/modules/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_OUTPUT_DIR}/modules/)

if(UNIX) 
    set (CMAKE_CXX_FLAGS "-W -Wall -Wextra")
endif()

file(
    GLOB_RECURSE 
    ModulesSrc
    ${PROJECT_SOURCE_DIR}/modules/*.cpp
)

foreach(lib_loc IN LISTS ModulesSrc) 
    set(CMAKE_CXX_STANDARD 20)

    include_directories(
        lib/include
        modules/
    )

    get_filename_component(lib_we ${lib_loc} NAME_WE)

    add_library(${lib_we} SHARED
        ./modules/shared.hpp
        ${lib_loc}
    )

    target_link_libraries(${lib_we} Library)

endforeach()


project(Library
        VERSION 0.0.1
        DESCRIPTION "Common library for executables & modules")

include(CTest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/build)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_OUTPUT_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_OUTPUT_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_OUTPUT_DIR}/lib)

set(SOURCES ./lib/include/common.hpp ./lib/include/macro_map.hpp ./lib/include/module_loader.hpp lib/src/file.cpp)

add_library(Library ${SOURCES})
target_link_libraries(Library ${CMAKE_DL_LIBS})

project(Executable
        VERSION 0.0.1
        DESCRIPTION "Global executable using library")

include(CTest)


if(UNIX) 
    set (CMAKE_CXX_FLAGS "-W -Wall -Wextra")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/build)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_OUTPUT_DIR})

set(SOURCES ./executable/main.cpp)

add_executable(Executable ${SOURCES})

foreach(lib_loc IN LISTS ModulesSrc) 


    get_filename_component(lib_we ${lib_loc} NAME_WE)


    target_link_libraries(Executable ${lib_we})

endforeach()

add_test (NAME executableTest COMMAND Executable reload reload quit)

set (passRegex "Hello World 170" "Goodbye Pizza 170")

set_property (TEST executableTest
              PROPERTY PASS_REGULAR_EXPRESSION "${passRegex}")
