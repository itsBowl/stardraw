//
// Created by Bowl on 05/02/2026.
//

#pragma once
#include "../vk13/simple_window.hpp"
#include "../vk13/vk_device.hpp"

class vk_test_app {
public:
    vk_test_app();
    ~vk_test_app();


    void run();
private:
    stardraw::vk13::simple_window window{800, 600, "StardrawTest"};
    stardraw::vk13::vk_device device{window};

};
