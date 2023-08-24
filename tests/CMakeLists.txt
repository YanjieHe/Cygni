file(GLOB
    SOURCES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
    ${PROJECT_SOURCE_DIR}/src/Expressions/*.cpp
    ${PROJECT_SOURCE_DIR}/src/LexicalAnalysis/*.cpp
    ${PROJECT_SOURCE_DIR}/src/SyntaxAnalysis/*.cpp
    ${PROJECT_SOURCE_DIR}/src/Utility/*.cpp)

file(GLOB TESTS
    ${PROJECT_SOURCE_DIR}/tests/*.cpp
    ${PROJECT_SOURCE_DIR}/tests/Expressions/*.cpp
    ${PROJECT_SOURCE_DIR}/tests/LexicalAnalysis/*.cpp
    ${PROJECT_SOURCE_DIR}/tests/SyntaxAnalysis/*.cpp)

include_directories(
    ${PROJECT_SOURCE_DIR}/libs/
    ${PROJECT_SOURCE_DIR}/include/)

add_executable(cygni-tests
    ${SOURCES}
    ${TESTS})