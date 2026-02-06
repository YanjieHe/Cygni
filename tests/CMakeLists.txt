file(GLOB CYGNI_TEST_SOURCES
    ${PROJECT_SOURCE_DIR}/tests/*.cpp
    ${PROJECT_SOURCE_DIR}/tests/Expressions/*.cpp
    ${PROJECT_SOURCE_DIR}/tests/LexicalAnalysis/*.cpp
    ${PROJECT_SOURCE_DIR}/tests/SyntaxAnalysis/*.cpp
    ${PROJECT_SOURCE_DIR}/tests/Utility/*.cpp
    ${PROJECT_SOURCE_DIR}/tests/Visitors/*.cpp)

add_executable(cygni-tests
    ${CYGNI_TEST_SOURCES})

target_include_directories(cygni-tests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/libs)

target_link_libraries(cygni-tests PRIVATE cygni_core)