cmake_minimum_required(VERSION 3.17)
project(Onitama)

set(CMAKE_CXX_STANDARD 17)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions("-march=native -O0")
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions("-march=native -Ofast")
else ()
    message(FATAL_ERROR "CMAKE_BUILD_TYPE was not 'Debug' or 'Release'")
endif ()

if (WIN32)
    link_libraries(ws2_32)
endif ()

set(JSON_BuildTests OFF CACHE INTERNAL "")


add_subdirectory(src/)
add_subdirectory(extern/CLI11)
add_subdirectory(extern/json)
add_subdirectory(extern/doctest)


add_executable(
        Onitama

        src/cli/main.cpp
        src/cli/modes/Mode.h
        src/cli/modes/PerftMode.cpp
        src/cli/modes/PerftMode.h
        src/cli/modes/SelfPlayMode.cpp
        src/cli/modes/SelfPlayMode.h
        src/cli/modes/ServerPlayMode.cpp
        src/cli/modes/ServerPlayMode.h
        src/cli/modes/HumanPlayMode.cpp
        src/cli/modes/HumanPlayMode.h

        test/perfttest.cpp

        extern/easywsclient/easywsclient.cpp
        extern/easywsclient/easywsclient.hpp
)

target_link_libraries(Onitama OnitamaCore)
target_link_libraries(Onitama CLI11::CLI11)
target_link_libraries(Onitama nlohmann_json::nlohmann_json)
target_include_directories(Onitama PRIVATE extern/easywsclient)
target_link_libraries(Onitama doctest)


add_executable(
        OnitamaTests
        test/perfttest.cpp
)

target_compile_definitions(OnitamaTests PRIVATE STANDALONE_TESTS)
target_link_libraries(OnitamaTests OnitamaCore)
target_link_libraries(OnitamaTests doctest)