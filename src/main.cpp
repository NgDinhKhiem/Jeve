#include "interpreter/JeveInterpreter.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

void printUsage(const std::string& programName) {
    std::cout << "Usage: " << programName << " [options] <file>" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -Xms<size>  Set initial heap size (e.g., -Xms1m for 1MB)" << std::endl;
    std::cout << "  -Xmx<size>  Set maximum heap size (e.g., -Xmx64m for 64MB)" << std::endl;
    std::cout << "  -h, --help  Show this help message" << std::endl;
}

size_t parseMemorySize(const std::string& sizeStr) {
    size_t value = 0;
    size_t multiplier = 1;
    
    if (sizeStr.empty()) {
        throw std::runtime_error("Empty size string");
    }
    
    size_t i = 0;
    while (i < sizeStr.size() && std::isdigit(sizeStr[i])) {
        value = value * 10 + (sizeStr[i] - '0');
        i++;
    }
    
    if (i < sizeStr.size()) {
        char unit = sizeStr[i];
        switch (tolower(unit)) {
            case 'k': multiplier = 1024; break;
            case 'm': multiplier = 1024 * 1024; break;
            case 'g': multiplier = 1024 * 1024 * 1024; break;
            default:
                throw std::runtime_error("Unknown unit: " + std::string(1, unit));
        }
    }
    
    return value * multiplier;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Error: No input file specified." << std::endl;
        printUsage(argv[0]);
        return 1;
    }

    // Increased default memory settings
    size_t initialHeap = 4 * 1024 * 1024;    // 4MB
    size_t maxHeap = 128 * 1024 * 1024;      // 128MB
    std::string filename;
    
    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (arg.substr(0, 4) == "-Xms") {
            try {
                initialHeap = parseMemorySize(arg.substr(4));
            } catch (const std::exception& e) {
                std::cerr << "Error: Invalid initial heap size format. " << e.what() << std::endl;
                return 1;
            }
        } else if (arg.substr(0, 4) == "-Xmx") {
            try {
                maxHeap = parseMemorySize(arg.substr(4));
            } catch (const std::exception& e) {
                std::cerr << "Error: Invalid maximum heap size format. " << e.what() << std::endl;
                return 1;
            }
        } else {
            // Assume it's the filename
            filename = arg;
        }
    }
    
    if (filename.empty()) {
        std::cerr << "Error: No input file specified." << std::endl;
        printUsage(argv[0]);
        return 1;
    }

    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file: " << filename << std::endl;
            return 1;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string code = buffer.str();
        
        jeve::JeveInterpreter interpreter(initialHeap, maxHeap);
        interpreter.interpret(code);
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
} 