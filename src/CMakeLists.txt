file(GLOB
    SOURCES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
    ${PROJECT_SOURCE_DIR}/src/LexicalAnalysis/*.cpp
    ${PROJECT_SOURCE_DIR}/src/SyntaxAnalysis/*.cpp
    ${PROJECT_SOURCE_DIR}/src/Utility/*.cpp)

include_directories(
    ${PROJECT_SOURCE_DIR}/libs
    ${PROJECT_SOURCE_DIR}/include)

add_executable(cygni
    ${PROJECT_SOURCE_DIR}/Main.cpp
    ${SOURCES})