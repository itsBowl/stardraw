//
// Created by Bowl on 06/02/2026.
//

#pragma once
#include "simple_window.hpp"
#include "vk_device.hpp"
#include "vk_swapchain.hpp"

#include <memory>
#include <vector>
#include <cassert>
#include <array>


namespace stardraw::vk13 {
    class vk_render {
    public:
        vk_render(simple_window&, vk_device&);
        ~vk_render();

        vk_render(const vk_render&) = delete;
        vk_render& operator=(const vk_render&) = delete;

        bool is_frame_in_progress() const {return is_frame_started;}
        VkCommandBuffer get_current_command_buffer() const;
        VkRenderPass get_current_render_pass() const {return swapchain->get_render_pass();}
        float get_aspect_ratio() const {return swapchain->extent_aspect_ratio();}

        int get_frame_index() const;

        VkCommandBuffer begin_frame();
        void end_frame();

        void begin_swapchain_render_pass(VkCommandBuffer);
        void end_swapchain_render_pass(VkCommandBuffer);

    private:
        simple_window& window;
        vk_device& device;
        std::unique_ptr<vk_swapchain> swapchain;
        std::vector<VkCommandBuffer> cmd_buffers;

        uint32_t image_idx;
        int frame_index{0};
        bool is_frame_started{false};

        void create_command_buffers();
        void free_command_buffers();
        void recreate_swapchain();
    };
}