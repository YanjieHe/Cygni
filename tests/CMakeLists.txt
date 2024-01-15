file(GLOB
    SOURCES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
    ${PROJECT_SOURCE_DIR}/src/Expressions/*.cpp
    ${PROJECT_SOURCE_DIR}/src/LexicalAnalysis/*.cpp
    ${PROJECT_SOURCE_DIR}/src/SyntaxAnalysis/*.cpp
    ${PROJECT_SOURCE_DIR}/src/Utility/*.cpp
    ${PROJECT_SOURCE_DIR}/src/Visitors/*.cpp
    ${PROJECT_SOURCE_DIR}/src/FlintByteCode/*.cpp)

file(GLOB TESTS
    ${PROJECT_SOURCE_DIR}/tests/*.cpp
    ${PROJECT_SOURCE_DIR}/tests/Expressions/*.cpp
    ${PROJECT_SOURCE_DIR}/tests/LexicalAnalysis/*.cpp
    ${PROJECT_SOURCE_DIR}/tests/SyntaxAnalysis/*.cpp
    ${PROJECT_SOURCE_DIR}/tests/Visitors/*.cpp)

include_directories(
    ${PROJECT_SOURCE_DIR}/libs/
    ${PROJECT_SOURCE_DIR}/include/
    ${PROJECT_SOURCE_DIR}/include/FlintByteCode)

add_executable(cygni-tests
    ${SOURCES}
    ${TESTS})