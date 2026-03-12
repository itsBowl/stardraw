//
// Created by Bowl on 05/02/2026.
//

#include "main.hpp"
#include <iostream>

vk_test_app::vk_test_app() {

}

vk_test_app::~vk_test_app() {

}

void vk_test_app::run() {
    while (!window.shouldClose()) {
        glfwPollEvents();
        //std::cout << "Test run\n";
    }

    std::cout << "Ending test run\n";
}

int main() {
    using namespace stardraw::vk13;

    vk_test_app app;

    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;

}
