#include "interpreter/JeveInterpreter.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <source_file>" << std::endl;
        std::cerr << "Example: " << argv[0] << " examples/calculator.jeve" << std::endl;
        return 1;
    }

    try {
        // Read the source file
        std::ifstream file(argv[1]);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file '" << argv[1] << "'" << std::endl;
            return 1;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string code = buffer.str();

        // Create and run the interpreter
        jeve::JeveInterpreter interpreter;
        interpreter.interpret(code);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
} 