cmake_minimum_required(VERSION 3.30)
project(Dependencies)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_BUILD_SHARED_LIBS ON)
set(BUILD_SHARED_LIBS ON)
OPTION(VULKAN_HPP_DISPATCH_LOADER_DYNAMIC ON)
OPTION(UPDATE_DEPS OFF)
SET(GLFW_LIBRARY_TYPE "SHARED")
OPTION(UPDATE_DEPS ON)
OPTION(BUILD_WERROR ON)
option(VMA_STATIC_VULKAN_FUNCTIONS "Link statically with Vulkan API" OFF)
option(VMA_DYNAMIC_VULKAN_FUNCTIONS "Fetch pointers to Vulkan functions internally (no static linking)" ON)
option(GLM_BUILD_LIBRARY ON)
set(CMAKE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/build)
message(STATUS "CMake command is executed in: ${CMAKE_CURRENT_BINARY_DIR}")


if(WIN32) 
    add_definitions(-DWIN32)
endif()


if (NOT CMAKE_BUILD_TYPE)
message(WARNING "CMAKE_BUILD_TYPE not set. Using Debug for dependency build type")
set(CMAKE_BUILD_TYPE Debug)
endif()

set(DepDir Debug)


if (CMAKE_CROSSCOMPILING)
set(DepDir "${CMAKE_SYSTEM_NAME}/${DepDir}/${CMAKE_SYSTEM_PROCESSOR}")
else()
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    message (STATUS "CMake think we are compiling for 64-bit")
else ()
    message (STATUS "CMake think we are compiling for something not 64-bit")
endif()
math(EXPR bitness "8 * ${CMAKE_SIZEOF_VOID_P}")
set(DepDir "${DepDir}/${bitness}")
endif()    



add_definitions(-DVULKAN_HPP_DISPATCH_LOADER_DYNAMIC)
add_definitions(-DVULKAN_HPP_NO_EXCEPTIONS)
add_definitions(-DVK_NO_PROTOTYPES)

if(UPDATE_DEPS)
    add_subdirectory(vendor/Vulkan-ValidationLayers/)
    return()
endif()

set(RUNTIME_OUTPUT_DIRECTORY "${CMAKE_OUTPUT_DIRECTORY}/")
set(LIBRARY_OUTPUT_DIRECTORY "${CMAKE_OUTPUT_DIRECTORY}/")
set(ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_OUTPUT_DIRECTORY}/")
add_subdirectory(vendor/Vulkan-ValidationLayers/external/${DepDir}/Vulkan-Headers)
add_subdirectory(vendor/Vulkan-ValidationLayers/external/${DepDir}/SPIRV-Headers)
add_subdirectory(vendor/Vulkan-ValidationLayers/external/${DepDir}/SPIRV-Tools) 
add_subdirectory(vendor/Vulkan-ValidationLayers/external/${DepDir}/Vulkan-Utility-Libraries)
add_subdirectory(vendor/Vulkan-ValidationLayers)
add_subdirectory(vendor/Vulkan-ValidationLayers/external/${DepDir}/glslang)
add_subdirectory(vendor/glfw)
add_subdirectory(vendor/glm)


add_subdirectory(vendor/VulkanMemoryAllocator)


set(REQUIRED_LIBRARIES SPIRV-Tools-shared glslang glslang-default-resource-limits SPIRV Vulkan::Headers GPUOpen::VulkanMemoryAllocator SPIRV-Headers glfw glm)
set(INCLUDE_DIRECTORIES )

project(Modules
        VERSION 0.0.1
        DESCRIPTION "Shared library modules")
include(CTest)

if(UNIX) 
    set (CMAKE_CXX_FLAGS "-W -Wall -Wextra")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set (CMAKE_CXX_FLAGS "-W -Wall -Wextra")

endif()


if(UNIX) 
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

    target_include_directories(${lib_we} PUBLIC ${INCLUDE_DIRECTORIES})

    set_target_properties(${lib_we} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_OUTPUT_DIRECTORY}/$<CONFIG>/modules/"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_OUTPUT_DIRECTORY}/$<CONFIG>/modules/"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_OUTPUT_DIRECTORY}/$<CONFIG>/modules/"
    )

    target_link_libraries(${lib_we} Library ${REQUIRED_LIBRARIES})
    set_target_properties(${lib_we} PROPERTIES FOLDER Main/Modules)


endforeach()




project(Library
        VERSION 0.0.1
        DESCRIPTION "Common library for executables & modules")

if(UNIX) 
    set (CMAKE_CXX_FLAGS "-W -Wall -Wextra")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set (CMAKE_CXX_FLAGS "-W -Wall -Wextra")

endif()

include(CTest)

set(CMAKE_CXX_STANDARD 20)

set(SOURCES ./lib/include/common.hpp ./lib/include/vulkan_app.hpp
            ./lib/include/module_loader.hpp lib/src/utils.cpp
            ./lib/include/swapchain.hpp ./lib/include/device.hpp
            ./lib/include/instance.hpp ./lib/include/allocator.hpp
            ./lib/src/device.cpp ./lib/src/graphics_pipeline.cpp
            ./lib/include/graphics_pipeline.hpp
            ./lib/src/swapchain.cpp
            lib/src/vulkan_app.cpp lib/src/vma.cpp lib/src/vulkan.cpp)
set(MODULES )

add_library(Library SHARED ${SOURCES})

set_target_properties(Library PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_OUTPUT_DIRECTORY}/$<CONFIG>"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_OUTPUT_DIRECTORY}/$<CONFIG>"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_OUTPUT_DIRECTORY}/$<CONFIG>"
)

target_link_libraries(Library ${CMAKE_DL_LIBS} ${REQUIRED_LIBRARIES})
target_include_directories(Library PUBLIC ${INCLUDE_DIRECTORIES})
target_compile_definitions(Library PRIVATE -DLIBRARY_EXPORT=1)
set_target_properties(Library PROPERTIES FOLDER Main)

project(Executable
        VERSION 0.0.1
        DESCRIPTION "Global executable using library")

include(CTest)


if(UNIX) 
    set (CMAKE_CXX_FLAGS "-W -Wall -Wextra")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set (CMAKE_CXX_FLAGS "-W -Wall -Wextra")

endif()

set(CMAKE_CXX_STANDARD 20)

set(SOURCES ./executable/main.cpp ./executable/rendering.hpp )

add_executable(Executable ${SOURCES})



set_target_properties(Executable PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_OUTPUT_DIRECTORY}/$<CONFIG>"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_OUTPUT_DIRECTORY}/$<CONFIG>"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_OUTPUT_DIRECTORY}/$<CONFIG>"
)

target_link_libraries(Executable ${REQUIRED_LIBRARIES} Library)

target_include_directories(Executable PUBLIC ${INCLUDE_DIRECTORIES})
set_target_properties(Executable PROPERTIES FOLDER Main)
set_property(TARGET Executable PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}")
set_property(TARGET Executable PROPERTY VS_STARTUP_PROJECT)
add_custom_command(TARGET Executable POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:Executable> $<TARGET_RUNTIME_DLLS:Executable>
  COMMAND_EXPAND_LISTS
)



add_custom_command(
         TARGET Executable POST_BUILD
         COMMAND ${CMAKE_COMMAND} -E copy_directory              
             ${CMAKE_CURRENT_LIST_DIR}/shaders $<TARGET_FILE_DIR:Executable>/shaders
         COMMENT "Copying shaders" VERBATIM
)

