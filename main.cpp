#include "Engine/Application.h"
#include <iostream>

int main() {
    try {
        DoEngine::Application app;
        app.Run();
    } catch (const std::exception& e) {
        std::cerr << "[CRITICAL ERROR] Application crashed: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
