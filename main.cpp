#include "Engine/Application.h"
#include <iostream>

int main() {
    std::cout << "[MAIN] Starting Application..." << std::endl;
    try {
        DoEngine::Application app;
        std::cout << "[MAIN] Entering app.Run()..." << std::endl;
        app.Run();
        std::cout << "[MAIN] app.Run() finished." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[CRITICAL ERROR] Application crashed: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    std::cout << "[MAIN] Process exiting gracefully." << std::endl;
    return EXIT_SUCCESS;
}
