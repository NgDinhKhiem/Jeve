cmake_minimum_required(VERSION 3.10)
project(Jeve VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add compiler flags
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Add source files
set(SOURCES
    src/main.cpp
    src/interpreter/JeveInterpreter.cpp
    src/interpreter/GarbageCollector.cpp
)

# Add header files
set(HEADERS
    src/interpreter/JeveInterpreter.hpp
    src/interpreter/Value.hpp
    src/interpreter/Object.hpp
    src/interpreter/SymbolTable.hpp
    src/interpreter/ASTNode.hpp
)

# Create executable
add_executable(jeve ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(jeve PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Install target
install(TARGETS jeve DESTINATION bin) 