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
    src/interpreter/ast/OperatorNodes.cpp
    src/interpreter/ast/ControlFlowNodes.cpp
    src/interpreter/ast/ArrayNodes.cpp
    src/interpreter/ast/FunctionNodes.cpp
    src/interpreter/ast/IONodes.cpp
    src/interpreter/ast/GCNodes.cpp
)

# Add header files
set(HEADERS
    src/interpreter/Forward.hpp
    src/interpreter/Object.hpp
    src/interpreter/Value.hpp
    src/interpreter/SymbolTable.hpp
    src/interpreter/GarbageCollector.hpp
    src/interpreter/JeveInterpreter.hpp
    src/interpreter/ast/ASTNode.hpp
    src/interpreter/ast/BasicNodes.hpp
    src/interpreter/ast/OperatorNodes.hpp
    src/interpreter/ast/ControlFlowNodes.hpp
    src/interpreter/ast/ArrayNodes.hpp
    src/interpreter/ast/FunctionNodes.hpp
    src/interpreter/ast/IONodes.hpp
    src/interpreter/ast/GCNodes.hpp
    src/interpreter/ast/AssignmentNode.hpp
    src/interpreter/ast/ConcatNode.hpp
    src/interpreter/ast/PropertyAccessNode.hpp
    src/interpreter/ast/SmartLoopNode.hpp
)

# Create executable
add_executable(jeve ${SOURCES})

# Include directories
target_include_directories(jeve PRIVATE src)

# Install target
install(TARGETS jeve DESTINATION bin) 

# Add AddressSanitizer flags
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -g")
set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fsanitize=address") 