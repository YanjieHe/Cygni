file(GLOB CYGNI_CORE_SOURCES
    ${PROJECT_SOURCE_DIR}/src/Expressions/*.cpp
    ${PROJECT_SOURCE_DIR}/src/LexicalAnalysis/*.cpp
    ${PROJECT_SOURCE_DIR}/src/SyntaxAnalysis/*.cpp
    ${PROJECT_SOURCE_DIR}/src/Utility/*.cpp
    ${PROJECT_SOURCE_DIR}/src/Visitors/*.cpp)

add_library(cygni_core STATIC
    ${CYGNI_CORE_SOURCES})

target_include_directories(cygni_core
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
    PRIVATE
        ${PROJECT_SOURCE_DIR}/libs)

target_link_libraries(cygni_core PUBLIC flint_bytecode)

add_executable(cygni
    ${PROJECT_SOURCE_DIR}/Main.cpp)

target_link_libraries(cygni PRIVATE cygni_core)

target_include_directories(cygni
    PRIVATE
        ${PROJECT_SOURCE_DIR}/libs)