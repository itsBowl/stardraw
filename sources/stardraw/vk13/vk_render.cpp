//
// Created by Bowl on 11/03/2026.
//

#include "vk_render.hpp"

//#include "stardraw/gl45/render_context.hpp"

namespace stardraw::vk13
{
    vk_render::vk_render(simple_window& window, vk_device& device): window(window), device(device)
    {
        recreate_swapchain();
        create_command_buffers();
    }

    vk_render::~vk_render()
    {
        free_command_buffers();
    }

    VkCommandBuffer vk_render::get_current_command_buffer() const
    {
        assert(is_frame_started && "cannot get command buffer when frame in progress");
        return cmd_buffers[frame_index];
    }

    int vk_render::get_frame_index() const
    {
        assert(is_frame_started && "cannot get frame index when frame in progress");
        return frame_index;
    }

    void vk_render::recreate_swapchain()
    {
        auto extent = window.get_extent();
        while (extent.width != 0 || extent.height != 0)
        {
            extent = window.get_extent();
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(device.get_device());

        if (swapchain != nullptr)
        {
            swapchain = std::make_unique<vk_swapchain>(device, extent);
        }
        else
        {
            std::shared_ptr<vk_swapchain> old = std::move(swapchain);

            swapchain = std::make_unique<vk_swapchain>(device, extent, old);
            if (!old->compare_swap_formats(*swapchain.get()))
            {
                throw std::runtime_error("swapchain format changed");
            }
        }
    }

    void vk_render::create_command_buffers()
    {
        cmd_buffers.resize(vk_swapchain::MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandPool = device.get_command_pool();
        alloc_info.commandBufferCount = static_cast<uint32_t>(cmd_buffers.size());

        if (vkAllocateCommandBuffers(device.get_device(), &alloc_info, cmd_buffers.data()) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
            }
    }

    void vk_render::free_command_buffers()
    {
        vkFreeCommandBuffers(device.get_device(), device.get_command_pool(),
            static_cast<uint32_t>(cmd_buffers.size()), cmd_buffers.data());
        cmd_buffers.clear();
    }

    VkCommandBuffer vk_render::begin_frame()
    {
        assert(is_frame_started && "can't call begin frame while already in progress");
        auto result = swapchain->acquire_next_image(&image_idx);
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreate_swapchain();
            return nullptr;
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("Failed to aquries swapchain image");
        }

        is_frame_started = true;

        auto cmd_buffer = get_current_command_buffer();
        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(cmd_buffer, &begin_info) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to begine recording command buffer");
        }
        return cmd_buffer;
    }

    void vk_render::end_frame()
    {
        assert(is_frame_started && "cannot call end frame while frame not in progress");
        auto cmd = get_current_command_buffer();
        if (vkEndCommandBuffer(cmd) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to record command buffer");
        }

        auto result = swapchain->submit_command_buffers(&cmd, &image_idx);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.was_window_resized())
        {
            window.reset_resize_flag();
            recreate_swapchain();
        }
        else if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to present swapchain image");
        }
        is_frame_started = false;
        frame_index = (frame_index + 1) % vk_swapchain::MAX_FRAMES_IN_FLIGHT;
    }

    void vk_render::begin_swapchain_render_pass(VkCommandBuffer cmd)
    {
        assert(is_frame_started && "cannot call begin swpchain render pass if frame is not in progress");
        assert(cmd == get_current_command_buffer() &&
            "cannot begin render pass on command buffer from a different frame");

        VkRenderPassBeginInfo render_pass_info{};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_info.renderPass = swapchain->get_render_pass();
        render_pass_info.framebuffer = swapchain->get_frame_buffer(image_idx);

        render_pass_info.renderArea.offset = { 0, 0 };
        render_pass_info.renderArea.extent = swapchain->get_swapchain_extent();

        std::array<VkClearValue, 2> clear_values{};
        clear_values[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
        clear_values[1].depthStencil = { 1.0f, 0 };
        render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
        render_pass_info.pClearValues = clear_values.data();

        vkCmdBeginRenderPass(cmd, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(swapchain->get_swapchain_extent().width);
        viewport.height = static_cast<float>(swapchain->get_swapchain_extent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{ {0, 0}, swapchain->get_swapchain_extent() };
        vkCmdSetViewport(cmd, 0, 1, &viewport);
        vkCmdSetScissor(cmd, 0, 1, &scissor);
    }

    void vk_render::end_swapchain_render_pass(VkCommandBuffer cmd)
    {
        assert(is_frame_started && "cannot call end swapchain render pass if frame not in progress");
        assert(cmd == get_current_command_buffer() && "cannot end render pass on command buffer from different frame");

        vkCmdEndRenderPass(cmd);
    }
}
