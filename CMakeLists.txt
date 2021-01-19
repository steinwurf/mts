cmake_minimum_required (VERSION 3.10)
project(mts)

if(NOT DEFINED STEINWURF_RESOLVE)
    message(STATUS "Resolving dependencies...")
    execute_process(
        COMMAND python waf resolve
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    set(STEINWURF_RESOLVE "${CMAKE_CURRENT_SOURCE_DIR}/resolve_symlinks")
endif()

file(GLOB mts_headers ./src/mts/*.hpp)

add_library(mts INTERFACE)
add_library(steinwurf::mts ALIAS mts)

if (NOT TARGET bnb)
add_subdirectory("${STEINWURF_RESOLVE}/bnb" bnb)
endif()

if (NOT TARGET recycle)
add_subdirectory("${STEINWURF_RESOLVE}/recycle" recycle)
endif()

if (NOT TARGET boost)
# Add to include path to the resolved boost dependency to enable cmake
# to use the same header files as the waf build system.
add_library(boost INTERFACE)
add_library(steinwurf::boost ALIAS boost)
target_include_directories(boost INTERFACE "${STEINWURF_RESOLVE}/boost/")
endif()

target_link_libraries(mts
    INTERFACE steinwurf::bnb
    INTERFACE steinwurf::recycle
    INTERFACE steinwurf::boost)

target_include_directories(mts INTERFACE src)

target_compile_features(mts INTERFACE cxx_std_11)

install(FILES ${mts_headers} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/mts)
