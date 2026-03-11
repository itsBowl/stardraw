//
// Created by Bowl on 06/02/2026.
//

#pragma once
#include "./vk_device.hpp"

#include <string>
#include <vector>
#include <fstream>
#include <stdexcept>

namespace stardraw::vk13 {

    struct vk_pipeline_config_info {
        vk_pipeline_config_info();
        vk_pipeline_config_info(const vk_pipeline_config_info&) = delete;
        vk_pipeline_config_info& operator=(const vk_pipeline_config_info&) = delete;

        std::vector<VkVertexInputBindingDescription> vertex_input_bindings;
        std::vector<VkVertexInputAttributeDescription> vertex_input_attributes;
        VkPipelineViewportStateCreateInfo viewport_info{};
        VkPipelineInputAssemblyStateCreateInfo input_info{};
        VkPipelineRasterizationStateCreateInfo raster_info{};
        VkPipelineMultisampleStateCreateInfo multisample_info{};
        VkPipelineColorBlendAttachmentState color_blend_attach{};
        VkPipelineColorBlendStateCreateInfo color_blend_info{};
        VkPipelineDepthStencilStateCreateInfo depth_stencil_info{};

        std::vector<VkDynamicState> dynamic_state_enables{};
        VkPipelineDynamicStateCreateInfo dynamic_state_info{};

        VkPipelineLayout pipeline_layout = nullptr;
        VkRenderPass render_pass = nullptr;
        uint32_t subpass = 0;

    };
    class vk_pipeline {
    public:
        vk_pipeline(vk_device&, const vk_pipeline_config_info&,
            const std::string&, const std::string&);
        ~vk_pipeline();

        vk_pipeline(const vk_pipeline&) = delete;
        vk_pipeline& operator=(const vk_pipeline&) = delete;

        static void default_pipeline_config_info(vk_pipeline_config_info&);
        static void enable_alpha_blending(vk_pipeline_config_info&);

        void bind(VkCommandBuffer);

    private:

        static std::vector<char> read_file(const std::string&);

        void create_graphics_pipeline(const std::string&, const std::string&, const vk_pipeline_config_info&);

        void create_shader_module(const std::vector<char>, VkShaderModule*);

        vk_device& device;
        VkPipeline graphics_pipeline;
        VkShaderModule vertex_module;
        VkShaderModule fragment_module;
    };
}
