#include "graphics_pipeline.hpp"
#include "vulkan.hpp"
#include "vulkan/vulkan_core.h"
#include "vulkan/vulkan_structs.hpp"


namespace Vulkan {

    VulkanResult GraphicsPipeline::createGraphicsPipeline(GraphicsPipelineConfig pipelineConfig) {
        config = pipelineConfig;

        return createGraphicsPipeline();
    }

    VulkanResult GraphicsPipeline::createGraphicsPipeline() {
		vk::PipelineVertexInputStateCreateInfo vertexInfo{};

        vertexInfo.setVertexAttributeDescriptions(config.vertexAttributeDescriptions);
        vertexInfo.setVertexBindingDescriptions(config.vertexBindingDescriptions);

        vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
        inputAssemblyInfo.setTopology(config.topology);
        inputAssemblyInfo.setPrimitiveRestartEnable(config.primitiveRestart);

		vk::PipelineViewportStateCreateInfo viewportStateInfo{};
        
        if(config.viewportConfig.usesDynamicViewport) {
            viewportStateInfo.viewportCount = config.viewportConfig.dynamicViewportCount;
        }
        else {
            viewportStateInfo.setViewports(config.viewportConfig.viewports);
        }

        if(config.scissorConfig.usesDynamicScissors) {
            viewportStateInfo.scissorCount = config.scissorConfig.dynamicScissorsCount;
        }
        else {
            viewportStateInfo.setScissors(config.scissorConfig.scissors);
        }

        vk::PipelineDynamicStateCreateInfo dynamicStateInfo {};

        dynamicStateInfo.setDynamicStates(config.dynamicStates);

		vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.setSetLayouts(config.descriptorSetLayouts);
        pipelineLayoutInfo.setPushConstantRanges(config.pushConstants);

		VULKAN_SET_AND_BAIL_RESULT_VALUE(
            config.device->getDevice().createPipelineLayoutUnique(
                pipelineLayoutInfo,
                nullptr, 
                config.device->getDispatcher()),
            pipelineLayout, 
            "Couldn't create pipeline layout!");

		std::cout << "Created pipeline layout" << std::endl;

		vk::PipelineColorBlendStateCreateInfo colorBlend{};

        colorBlend.setBlendConstants(config.colorBlendConfig.constants);
        colorBlend.setLogicOpEnable(config.colorBlendConfig.enableLogicOp);
        colorBlend.setLogicOp(config.colorBlendConfig.logicOp);
        colorBlend.setAttachments(config.colorBlendConfig.attachments);



		vk::GraphicsPipelineCreateInfo pipelineInfo{};

        pipelineInfo.setStages(config.shaderStages);
        pipelineInfo.setPVertexInputState(&vertexInfo);
        pipelineInfo.setPInputAssemblyState(&inputAssemblyInfo);
        pipelineInfo.setPViewportState(&viewportStateInfo);
        pipelineInfo.setPRasterizationState(&config.razterizationInfo);
        pipelineInfo.setPMultisampleState(&config.multiSampling);

        pipelineInfo.setPColorBlendState(&colorBlend);
        pipelineInfo.setPDynamicState(&dynamicStateInfo);
        pipelineInfo.setLayout(pipelineLayout.get());
        pipelineInfo.setRenderPass(config.renderPass);
        pipelineInfo.setSubpass(config.subpass);
        pipelineInfo.setBasePipelineHandle(pipeline.get() == VK_NULL_HANDLE ? pipeline.get() : VK_NULL_HANDLE);


		VULKAN_SET_AND_BAIL_RESULT_VALUE(
            config.device->getDevice().createGraphicsPipelineUnique(
                nullptr,
                pipelineInfo,
                nullptr, 
                config.device->getDispatcher()),
            pipeline, 
            "Couldn't create pipeline!");

		std::cout << "Created graphics pipeline !" << std::endl;


		return VulkanResult();
        
    }

    VulkanResult GraphicsPipeline::recreateGraphicsPipeline() {
        return createGraphicsPipeline();
    }
}