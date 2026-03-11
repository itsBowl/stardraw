//
// Created by Bowl on 06/02/2026.
//

#include "vk_pipeline.hpp"
#include <cassert>

namespace stardraw::vk13 {
    vk_pipeline::vk_pipeline(vk_device& d, const vk_pipeline_config_info& pci,
        const std::string& vert_path, const std::string& frag_path) :device(d) {
        create_graphics_pipeline(vert_path, frag_path, pci);
    }

    vk_pipeline::~vk_pipeline() {
        vkDestroyShaderModule(device.get_device(), vertex_module, nullptr);
        vkDestroyShaderModule(device.get_device(), fragment_module, nullptr);

        vkDestroyPipeline(device.get_device(), graphics_pipeline, nullptr);
    }

    std::vector<char> read_file(const std::string& path) {
        //add engine path things here
        std::ifstream file(path, std::ios::ate || std::ios::binary);
        if (file.is_open()) {
            throw std::runtime_error("Cannot open file in pipeline constructions\n");
        }

        size_t size = static_cast<size_t>(file.tellg());

        std::vector<char> buffer(size);
        file.seekg(0);
        file.read(buffer.data(), size);

        file.close();

        return buffer;
    }

    void vk_pipeline::create_graphics_pipeline(const std::string& vp, const std::string& fp,
        const vk_pipeline_config_info& config_info) {
        asset(config_info.pipeline_layout != VK_NULL_HANDLE &&
            "can not create graphics pipeline: no layout provided in config info");
        assert(config_info.render_pass != VK_NULL_HANDLE &&
            "can not create graphics pipeline: no render pass provided in config info");

        auto vertex_code = read_file(vp);
        auto fragment_code = read_file(fp);
        create_shader_module(vertex_code, &vertex_module);
        create_shader_module(fragment_code, &fragment_module);

        VkPipelineShaderStageCreateInfo shader_stages[2];
        shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shader_stages[0].module = vertex_module;
        shader_stages[0].pName = "main";
        shader_stages[0].flags = 0;
        shader_stages[0].pNext = nullptr;
        shader_stages[0].pSpecializationInfo = nullptr;

        shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shader_stages[1].module = fragment_module;
        shader_stages[1].pName = "main";
        shader_stages[1].flags = 0;
        shader_stages[1].pNext = nullptr;
        shader_stages[1].pSpecializationInfo = nullptr;

        auto binding_descriptions = config_info.vertex_input_bindings;
        auto attribute_descriptions = config_info.vertex_input_attributes;

        VkPipelineVertexInputStateCreateInfo vertex_input_info{};
        vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size());
        vertex_input_info.vertexBindingDescriptionCount = static_cast<uint32_t>(binding_descriptions.size());
        vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions.data();
        vertex_input_info.pVertexBindingDescriptions = binding_descriptions.data();

        VkGraphicsPipelineCreateInfo pipeline_info{};
        pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeline_info.stageCount = 2;
        pipeline_info.pStages = shader_stages;
        pipeline_info.pVertexInputState = &vertex_input_info;
        pipeline_info.pInputAssemblyState = &config_info.input_info;
        pipeline_info.pViewportState = &config_info.viewport_info;
        pipeline_info.pRasterizationState = &config_info.raster_info;
        pipeline_info.pMultisampleState = &config_info.multisample_info;
        pipeline_info.pColorBlendState = &config_info.color_blend_info;
        pipeline_info.pDepthStencilState = &config_info.depth_stencil_info;
        pipeline_info.pDynamicState = &config_info.dynamic_state_info;

        pipeline_info.layout = config_info.pipeline_layout;
        pipeline_info.renderPass = config_info.render_pass;
        pipeline_info.subpass = config_info.subpass;

        pipeline_info.basePipelineIndex = -1;
        pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(device.get_device(), VK_NULL_HANDLE, 1,
            &pipeline_info, nullptr, &graphics_pipeline) != VK_SUCCESS)
        {
            throw std::runtime_error("graphics pipeline failed to create");
        }
    }

    void vk_pipeline::create_shader_module(const std::vector<char> shader, VkShaderModule* module) {
        VkShaderModuleCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        info.codeSize = shader.size();
        info.pCode = reinterpret_cast<const uint32_t*>(shader.data());
        if (vkCreateShaderModule(device.get_device(), &info, nullptr, module) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module");
        }
    }

    void vk_pipeline::default_pipeline_config_info(vk_pipeline_config_info& info) {
	info.input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	info.input_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	info.input_info.primitiveRestartEnable = VK_FALSE;

	info.viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	info.viewport_info.viewportCount = 1;
	info.viewport_info.pViewports = nullptr;
	info.viewport_info.scissorCount = 1;
	info.viewport_info.pScissors = nullptr;

	info.raster_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	info.raster_info.depthClampEnable = VK_FALSE;
	info.raster_info.rasterizerDiscardEnable = VK_FALSE;
	info.raster_info.polygonMode = VK_POLYGON_MODE_FILL;
	info.raster_info.lineWidth = 1.0f;
	info.raster_info.cullMode = VK_CULL_MODE_NONE;
	info.raster_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
	info.raster_info.depthBiasEnable = VK_FALSE;
	info.raster_info.depthBiasConstantFactor = 0.0f;
	info.raster_info.depthBiasClamp = 0.0f;
	info.raster_info.depthBiasSlopeFactor = 0.0f;

	info.multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	info.multisample_info.sampleShadingEnable = VK_FALSE;
	info.multisample_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	info.multisample_info.minSampleShading = 1.0f;
	info.multisample_info.pSampleMask = nullptr;
	info.multisample_info.alphaToCoverageEnable = VK_FALSE;
	info.multisample_info.alphaToOneEnable = VK_FALSE;

	info.color_blend_attach.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;
	info.color_blend_attach.blendEnable = VK_FALSE;
	info.color_blend_attach.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	info.color_blend_attach.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	info.color_blend_attach.colorBlendOp = VK_BLEND_OP_ADD;
	info.color_blend_attach.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	info.color_blend_attach.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	info.color_blend_attach.alphaBlendOp = VK_BLEND_OP_ADD;

	info.color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	info.color_blend_info.logicOpEnable = VK_FALSE;
	info.color_blend_info.logicOp = VK_LOGIC_OP_COPY;
	info.color_blend_info.attachmentCount = 1;
	info.color_blend_info.pAttachments = &info.colorBlendAttach;
	info.color_blend_info.blendConstants[0] = 0.0f;
	info.color_blend_info.blendConstants[1] = 0.0f;
	info.color_blend_info.blendConstants[2] = 0.0f;
	info.color_blend_info.blendConstants[3] = 0.0f;

	info.depth_stencil_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	info.depth_stencil_info.depthTestEnable = VK_TRUE;
	info.depth_stencil_info.depthWriteEnable = VK_TRUE;
	info.depth_stencil_info.depthCompareOp = VK_COMPARE_OP_LESS;
	info.depth_stencil_info.depthBoundsTestEnable = VK_FALSE;
	info.depth_stencil_info.minDepthBounds = 0.0f;
	info.depth_stencil_info.maxDepthBounds = 1.0f;
	info.depth_stencil_info.stencilTestEnable = VK_FALSE;
	info.depth_stencil_info.front = {};
	info.depth_stencil_info.back = {};

	info.dynamic_state_enables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	info.dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	info.dynamic_state_info.pDynamicStates = info.dynamic_state_enables.data();
	info.dynamic_state_info.dynamicStateCount = static_cast<uint32_t>(
		info.dynamic_state_enables.size()
		);
	info.dynamic_state_info.flags = 0;

	//configInfo.binding_descirptions = Model::Vertex::getBindingDescription();
	//configInfo.attribute_descirptions = Model::Vertex::getAttributeDescription();
	}

	void vk_pipeline::enable_alpha_blending(vk_pipeline_config_info& info) {
	    info.color_blend_attach.blendEnable = VK_TRUE;
    	info.color_blend_attach.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
    		VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    	info.color_blend_attach.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    	info.color_blend_attach.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    	info.color_blend_attach.colorBlendOp = VK_BLEND_OP_ADD;
    	info.color_blend_attach.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    	info.color_blend_attach.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    	info.color_blend_attach.alphaBlendOp = VK_BLEND_OP_ADD;
    }

	void vk_pipeline::bind(VkCommandBuffer cmd) {
	    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);
    }
}
