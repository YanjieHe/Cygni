cmake_minimum_required(VERSION 3.5)

project(Cygni)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    FlintByteCode
    GIT_REPOSITORY https://github.com/YanjieHe/FlintByteCode.git
    GIT_TAG master
)
FetchContent_MakeAvailable(FlintByteCode)

add_subdirectory(src)

enable_testing()
add_subdirectory(tests)