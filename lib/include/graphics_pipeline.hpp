#ifndef LIB_VULKAN_GRAPHICS_PIPELINE_HPP
#define LIB_VULKAN_GRAPHICS_PIPELINE_HPP

#include "vulkan.hpp"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_handles.hpp"
#include "vulkan/vulkan_structs.hpp"
#include "device.hpp"

namespace Vulkan {

    struct ViewportConfig {
        bool usesDynamicViewport = false;
        uint32_t dynamicViewportCount = 0;

        std::vector<vk::Viewport> viewports = {};
    };

    struct ScissorsConfig {
        bool usesDynamicScissors = false;
        uint32_t dynamicScissorsCount = 0;

        std::vector<vk::Rect2D> scissors = {};
    };

    struct ColorBlendConfig {
        std::vector<vk::PipelineColorBlendAttachmentState> attachments = {{
		    false,
		    vk::BlendFactor::eZero,
		    vk::BlendFactor::eZero,
		    vk::BlendOp::eAdd,
		    vk::BlendFactor::eZero,
		    vk::BlendFactor::eZero,
		    vk::BlendOp::eAdd,
		    vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
		        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
        }};

        std::array<float, 4> constants = {};

        bool enableLogicOp = false;
        vk::LogicOp logicOp = vk::LogicOp::eCopy;
    };

    struct GraphicsPipelineConfig {
        Device* device;
        vk::RenderPass renderPass;
        uint32_t subpass = 0;
        std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = {};

        std::vector<vk::DynamicState> dynamicStates = {};
        
        std::vector<vk::VertexInputBindingDescription> vertexBindingDescriptions = {};
        std::vector<vk::VertexInputAttributeDescription> vertexAttributeDescriptions = {};

        vk::PrimitiveTopology topology = vk::PrimitiveTopology::eTriangleList;
        bool primitiveRestart;

        ViewportConfig viewportConfig = {};
        ScissorsConfig scissorConfig = {};

        vk::PipelineRasterizationStateCreateInfo razterizationInfo = {
		    vk::PipelineRasterizationStateCreateFlags{},
		    false,
		    false,
		    vk::PolygonMode::eFill,
		    vk::CullModeFlagBits::eNone,
		    vk::FrontFace::eClockwise,
		    false,
		    0.0f,
		    0.0f,
		    0.0f,
		    1.0f,
		    nullptr
        };

        vk::PipelineMultisampleStateCreateInfo multiSampling = {
		    vk::PipelineMultisampleStateCreateFlags{},
		    vk::SampleCountFlagBits::e1,
		    false,
		    1.0f,
		    nullptr,
		    false,
		    false,
		    nullptr
        };

        ColorBlendConfig colorBlendConfig = {};

        std::vector<vk::DescriptorSetLayout> descriptorSetLayouts = {};
        std::vector<vk::PushConstantRange> pushConstants = {};

    };

    class LIBRARY_DLL GraphicsPipeline {
        public:

        VulkanResult createGraphicsPipeline(GraphicsPipelineConfig pipelineConfig);
        VulkanResult recreateGraphicsPipeline();

        vk::Pipeline getPipeline() { return pipeline.get(); }
        vk::PipelineLayout getPipelineLayout() { return pipelineLayout.get(); }
        GraphicsPipelineConfig& getPipelineConfig() { return config; }


        private:
        VulkanResult createGraphicsPipeline();

        GraphicsPipelineConfig config;
        vk::UniquePipelineLayout pipelineLayout;
        vk::UniquePipeline pipeline;

    };
}

#endif