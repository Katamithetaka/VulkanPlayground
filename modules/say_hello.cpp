#include "vulkan.hpp"

#include "allocator.hpp"
#include "common.hpp"
#include "device.hpp"
#include "glslang/Public/ShaderLang.h"
#include "graphics_pipeline.hpp"
#include "instance.hpp"
#include "shared.hpp"
#include "swapchain.hpp"
#include "thread"
#include "utils.hpp"
#include "vulkan/vulkan_core.h"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_handles.hpp"
#include "vulkan_app.hpp"
#include <filesystem>
#include <functional>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp> // after <glm/glm.hpp>
#include "glm/ext/matrix_clip_space.hpp"
#include <iostream>
#include <memory>
#include <ostream>
#include "GLFW/glfw3.h"

#ifdef _WIN32
#define MODULES_DLL __declspec(dllexport)
#else
#define MODULES_DLL
#endif

struct Vertex
{
	glm::vec2 position;
	glm::vec3 color;
	glm::vec2 uv;


	static vk::VertexInputBindingDescription getBindingDescription()
	{
		vk::VertexInputBindingDescription bindingDescription{};
		bindingDescription.setBinding(0);
		bindingDescription.setStride(sizeof(Vertex));
		bindingDescription.setInputRate(vk::VertexInputRate::eVertex);

		return bindingDescription;
	}

	static std::vector<vk::VertexInputAttributeDescription> getAttributeDescription()
	{
		std::vector<vk::VertexInputAttributeDescription> attributeDescription(3);

		attributeDescription[0].setBinding(0);
		attributeDescription[0].setLocation(0);
		attributeDescription[0].setFormat(vk::Format::eR32G32Sfloat);
		attributeDescription[0].setOffset(offsetof(Vertex, position));

		attributeDescription[1].setBinding(0);
		attributeDescription[1].setLocation(1);
		attributeDescription[1].setFormat(vk::Format::eR32G32B32Sfloat);
		attributeDescription[1].setOffset(offsetof(Vertex, color));

		attributeDescription[2].setBinding(0);
		attributeDescription[2].setLocation(2);
		attributeDescription[2].setFormat(vk::Format::eR32G32Sfloat);
		attributeDescription[2].setOffset(offsetof(Vertex, uv));

		return attributeDescription;
	}
};

struct UniformBuffer {
	glm::mat4 model{}, view{}, projection{};

	static std::vector<vk::DescriptorSetLayoutBinding>  getBindingDescription() {
		vk::DescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.setBinding(0);
		uboLayoutBinding.setDescriptorType(vk::DescriptorType::eUniformBuffer);
		uboLayoutBinding.setDescriptorCount(1);
		uboLayoutBinding.setStageFlags(vk::ShaderStageFlagBits::eVertex);


		return {uboLayoutBinding};
	}
};

std::array<Vertex, 4> vertices = {
    Vertex{
		.position = glm::vec2(-0.5, -0.5), 
		.color = glm::vec3(0.0, 0.0, 0.0),
		.uv = glm::vec2(-1,-1),
	},
    {
		.position = glm::vec2(-0.5, 0.5),
     	.color = glm::vec3(0.6392156863, 0.6392156863, 0.6392156863),
		.uv = glm::vec2(-1, 1),
	},
    {
		.position = glm::vec2(0.5, 0.5),
     	.color = glm::vec3(1.0, 1.0, 1.0),
		.uv = glm::vec2(1, 1),
	},
    {
		.position = glm::vec2(0.5, -0.5), 
		.color = glm::vec3(0.5019607843, 0, 0.5019607843),
		.uv = glm::vec2(1, -1),
	},
};

std::array<uint32_t, 6> indices{
    0, 1, 2, 
	2, 3, 0
};

using namespace Vulkan;

struct FrameData
{
	vk::UniqueSemaphore imageAvailableSemaphore;
	vk::UniqueSemaphore renderFinishedSemaphore;
	vk::UniqueFence inFlightFence;
	vk::CommandBuffer commandBuffer;
	Buffer uniformBuffer;
	vk::UniqueDescriptorSet descriptorSet;
};


uint32_t WIDTH = 1280;
uint32_t HEIGHT = 800;
std::string title = "Vulkan Application";
const uint32_t MAX_FRAMES_IN_FLIGHT = 1;

struct VkApp : VulkanApplication
{
	void onDebugMessage(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	                    vk::DebugUtilsMessageTypeFlagsEXT messageTypes,
	                    const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData)
	{

		std::cout << vk::to_string(messageSeverity) << " " << vk::to_string(messageTypes) << " " << pCallbackData->pMessage << std::endl;
	}

	static void GLFWframebuffersize(GLFWwindow *window, int, int)
	{
		VkApp *app = (VkApp *)glfwGetWindowUserPointer(window);
		app->framebufferResized = true;
	}

	virtual VulkanResult OnInit() override
	{
		std::cout << "Running OnInit()! " << std::endl;

		using vk::DebugUtilsMessageSeverityFlagBitsEXT::eError, vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo, vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose, vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning;
		using vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral, vk::DebugUtilsMessageTypeFlagBitsEXT::eDeviceAddressBinding, vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance, vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;

		window = glfwCreateWindow(WIDTH, HEIGHT, title.c_str(), nullptr, nullptr);
		glfwSetFramebufferSizeCallback(window, GLFWframebuffersize);
		glfwSetWindowUserPointer(window, this);
		if (!window)
		{
			auto result = VulkanResult::GLFWError();
			return result;
		}

		std::cout << "Setup window!" << std::endl;

		LIB_QUICK_BAIL(instance.createInstance({
		    .appName = "Vulkan Triangle",
		    .appVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
		    .engineName = "Vulkan Engine",
		    .engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
		    .vulkanVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),

		    .requiredInstanceExtensions = {VK_EXT_DEBUG_UTILS_EXTENSION_NAME},

		    .enableLayers = true,
		    .requiredLayers = {"VK_LAYER_KHRONOS_validation"},

		    .usesWindow = true,

		    .createDebugCallbackMessenger = true,

		    .messageSeverity = eError | eWarning | eInfo | eVerbose,
		    .messageTypes = eGeneral | eValidation | ePerformance | eDeviceAddressBinding,

		    .debugCallback = {
		        std::bind(&VkApp::onDebugMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)},
		}));

		std::cout << "Created instance!" << std::endl;

		LIB_QUICK_BAIL(device.createDevice({
		    .instance = &instance,
		    .queueRequirements = {
		        QueueInformation{
		            .requiredFlags = vk::QueueFlagBits::eGraphics,
		            .queuePriority = 1.0,
		            .name = "graphicsQueue"},
		        QueueInformation{
		            .requiredFlags = {},
		            .requirePresentSupport = true,
		            .queuePriority = 1.0,
		            .name = "presentQueue"},
		        QueueInformation{
		            .requiredFlags = vk::QueueFlagBits::eCompute,
		            .requirePresentSupport = false,
		            .queuePriority = 1.0,
		            .name = "computeQueue"},
		        QueueInformation{
		            .requiredFlags = vk::QueueFlagBits::eTransfer,
		            .requirePresentSupport = false,
		            .queuePriority = 1.0,
		            .name = "transferQueue"},
		    },
		    .features = vk::PhysicalDeviceFeatures{},

		    .checkSuitability = Utils::defaultCheckSuitability,
		    .pickBestPhysicalDevice = Utils::defaultPickBestPhysicalDevice,

		    .deviceExtensions = {},

		    // if this is set to true, the swapchain KHR extension will automatically be added to the device
		    .requiresSwapchainSupport = true,
		}));

		std::cout << "Created device!" << std::endl;

		graphicsQueue = &device.getQueue(0);
		presentQueue = &device.getQueue(1);
		computeQueue = &device.getQueue(2);
		transferQueue = &device.getQueue(3);

		std::vector<uint32_t> queueIndices{};

		if (graphicsQueue->queueIndex != presentQueue->queueIndex)
		{
			queueIndices = std::vector<uint32_t>{
			    graphicsQueue->queueIndex.value(), presentQueue->queueIndex.value()};
		}

		std::cout << "Got queues!" << std::endl;

		LIB_QUICK_BAIL(swapchain.createSurface(instance, window));

		std::cout << "Created surface!" << std::endl;

		LIB_QUICK_BAIL(device.querySwapchainSupportForDevice(swapchain.getSurface()));
		std::cout << "Got swapchain capabilities!" << std::endl;

		LIB_QUICK_BAIL(swapchain.createSwapchain({
		    .presentMode = Utils::chooseSwapPresentMode(device.getPhysicalDevice().swapchainDetails.presentModes),
		    .surfaceFormat = Utils::defaultChooseSwapSurfaceFormat(device.getPhysicalDevice().swapchainDetails.formats),
		    .sharingConfig = {
		        .sharingMode = graphicsQueue->queueIndex == presentQueue->queueIndex ? vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent,
		        .queueIndices = queueIndices},
		    .extent = Vulkan::Utils::getExtentFromWindow(device.getPhysicalDevice().swapchainDetails.capabilities, window),
		    .instance = &instance,
		    .device = &device,
		    .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
		    .clipped = true,
		}));

		std::cout << "Running in " << vk::to_string(swapchain.getSwapchainConfig().presentMode) << " mode" << std::endl;

		LIB_QUICK_BAIL(swapchain.createImageViews({.viewType = vk::ImageViewType::e2D,
		                                           .components = {
		                                               vk::ComponentSwizzle::eIdentity,
		                                               vk::ComponentSwizzle::eIdentity,
		                                               vk::ComponentSwizzle::eIdentity,
		                                               vk::ComponentSwizzle::eIdentity,
		                                           },
		                                           .subResourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}}));

		std::cout << "Created image views!" << std::endl;

		LIB_SET_AND_BAIL_RESULT_VALUE_UNSCOPPED(Utils::compileShaders({
		                                            {std::filesystem::path("shaders") / "simple_triangle.frag.glsl", EShLanguage::EShLangFragment},
		                                            {std::filesystem::path("shaders") / "simple_triangle.vert.glsl", EShLanguage::EShLangVertex},
		                                        }),
		                                        auto shaders);

		std::cout << "Created shaders!" << std::endl;

		vk::ShaderModuleCreateInfo fragmentShaderInfo = {};
		fragmentShaderInfo.setCode(shaders[0]);

		VULKAN_SET_AND_BAIL_RESULT_VALUE_UNSCOPPED(
		    device.getDevice().createShaderModuleUnique(fragmentShaderInfo),
		    auto fragmentShaderModule, "Couldn't create fragment shader");

		vk::ShaderModuleCreateInfo vertexShaderInfo = {};
		vertexShaderInfo.setCode(shaders[1]);

		VULKAN_SET_AND_BAIL_RESULT_VALUE_UNSCOPPED(
		    device.getDevice().createShaderModuleUnique(vertexShaderInfo),
		    auto vertexShaderModule, "Couldn't create vertex shader");

		std::cout << "Created shader modules" << std::endl;

		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = {
		    {}, {}};

		shaderStages[0].setPName("main");
		shaderStages[0].setModule(fragmentShaderModule.get());
		shaderStages[0].setStage(vk::ShaderStageFlagBits::eFragment);

		shaderStages[1].setPName("main");
		shaderStages[1].setModule(vertexShaderModule.get());
		shaderStages[1].setStage(vk::ShaderStageFlagBits::eVertex);

		LIB_QUICK_BAIL(createRenderPass());



		std::cout << "Created render pass!" << std::endl;

		LIB_QUICK_BAIL(createDescriptorSetLayout());

		LIB_QUICK_BAIL(pipeline.createGraphicsPipeline({
		    .device = &device,
		    .renderPass = renderPass.get(),
		    .subpass = 0,
		    .shaderStages = shaderStages,
		    .dynamicStates = {
		        vk::DynamicState::eViewport,
		        vk::DynamicState::eScissor},

		    .vertexBindingDescriptions = {Vertex::getBindingDescription()},
		    .vertexAttributeDescriptions = Vertex::getAttributeDescription(),

		    .topology = vk::PrimitiveTopology::eTriangleList,
		    .primitiveRestart = false,

		    .viewportConfig = {
		        .usesDynamicViewport = true,
		        .dynamicViewportCount = 1,
		    },
		    .scissorConfig = {
		        .usesDynamicScissors = true,
		        .dynamicScissorsCount = 1,
		    },
		    .razterizationInfo = {vk::PipelineRasterizationStateCreateFlags{}, false, false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone, vk::FrontFace::eClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f, nullptr},

		    .multiSampling = {vk::PipelineMultisampleStateCreateFlags{}, vk::SampleCountFlagBits::e1, false, 1.0f, nullptr, false, false, nullptr},

		    .colorBlendConfig = {.attachments = {{false, vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd, vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd, vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA}},

		                         .constants = {},

		                         .enableLogicOp = false,
		                         .logicOp = vk::LogicOp::eCopy},

		    .descriptorSetLayouts = {descriptorSetLayout.get()},
		    .pushConstants = {},
		}));

		std::cout << "Created graphics pipeline!" << std::endl;

		LIB_QUICK_BAIL(
		    allocator.createAllocator({
		        .device = &device,
		        .instance = instance.getInstance(),
		    }));

		LIB_SET_AND_BAIL_RESULT_VALUE_UNSCOPPED(allocator.createBuffer(
		                                            sizeof(Vertex) * 6, vk::BufferUsageFlagBits::eVertexBuffer,
		                                            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
		                                            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
		                                            VMA_MEMORY_USAGE_AUTO_PREFER_HOST),
		                                        vertexBuffer);

		std::cout << "Created vertex buffer" << std::endl;

		LIB_SET_AND_BAIL_RESULT_VALUE_UNSCOPPED(allocator.createBuffer(
		                                            sizeof(Vertex) * 6, vk::BufferUsageFlagBits::eIndexBuffer,
		                                            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
		                                            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
		                                            VMA_MEMORY_USAGE_AUTO_PREFER_HOST),
		                                        indexBuffer);

		std::cout << "Created Vertex & Index Buffer" << std::endl;

		LIB_QUICK_BAIL(fillVertexBuffer());

		LIB_QUICK_BAIL(swapchain.createFramebuffers({.renderPass = renderPass.get(),
		                                             .layers = 1}));

		std::cout << "Created framebuffers" << std::endl;

		LIB_QUICK_BAIL(createFrameDatas());

		std::cout << "Created frame data" << std::endl;

		LIB_QUICK_BAIL(createDescriptorPool());
		std::cout << "Created descriptor pool!" << std::endl;

		LIB_QUICK_BAIL(createDescriptorSets());
		std::cout << "Created descriptor sets!" << std::endl;


		return VulkanResult::Success();
	};

	VulkanResult MainLoop() override
	{
		static float previousTime = glfwGetTime();
		static float currentTime = previousTime;
		static float totalTime = 0;
		static int fps = 0;

		std::thread render_thread([this]()
		                          {

		while (!glfwWindowShouldClose(window))
		{
            auto result = drawFrame();
            if(result.type() != VulkanResultVariants::Success) {
                std::cout << result.description() << std::endl;
                break;
            };

			currentTime = glfwGetTime();
			auto diff = currentTime - previousTime;
			totalTime += diff;
			++rendered_frames;
			if(totalTime > 1) {
				std::string newTitle = title + " - " + std::to_string(fps) + "fps - " + std::to_string(rendered_frames) + " rendered fps";
				totalTime = 0;
				fps = 0;
				rendered_frames = 0;
				glfwSetWindowTitle(window, newTitle.c_str());
			}

			previousTime = glfwGetTime();
		} });

		while (!glfwWindowShouldClose(window))
		{
			glfwPollEvents();
			++fps;
		}

		render_thread.join();
		auto _ = device.getDevice().waitIdle();
		return VulkanResult::Success();
	}

	VulkanResult drawFrame()
	{
		VULKAN_QUICK_BAIL(device.getDevice().waitForFences(frameData[currentFrame].inFlightFence.get(), true, UINT32_MAX), "Coudln't wait for inflight fence");

		uint32_t imageIndex;
		auto image = device.getDevice().acquireNextImageKHR(swapchain.getSwapchain(), UINT64_MAX, frameData[currentFrame].imageAvailableSemaphore.get());

		if (image.result == vk::Result::eErrorOutOfDateKHR)
		{
			Utils::recreateSwapchainFromWindow(window, device, swapchain);
			return VulkanResult::Success();
		}
		else if (image.result != vk::Result::eSuccess && image.result != vk::Result::eSuboptimalKHR)
		{
			return VulkanResult::VulkanError(image.result, "Couldn't acquire image for rendering!");
		}
		
		updateUniformBuffer(currentFrame);

		VULKAN_QUICK_BAIL(device.getDevice().resetFences(frameData[currentFrame].inFlightFence.get()), "Couldn't reset inflightfence!");

		imageIndex = image.value;


		frameData[currentFrame].commandBuffer.reset();
		LIB_QUICK_BAIL(recordCommand(frameData[currentFrame].commandBuffer, imageIndex));

		auto waitSemaphores = {frameData[currentFrame].imageAvailableSemaphore.get()};
		std::vector<vk::PipelineStageFlags> waitStages = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

		vk::SubmitInfo submit{
		    waitSemaphores,
		    waitStages,
		    frameData[currentFrame].commandBuffer,
		    {frameData[currentFrame].renderFinishedSemaphore.get()}};

		VULKAN_QUICK_BAIL(graphicsQueue->queue.submit(submit, frameData[currentFrame].inFlightFence.get()), "Couldn't submit to graphics queue");

		vk::PresentInfoKHR presentInfo{
		    {frameData[currentFrame].renderFinishedSemaphore.get()},
		    swapchain.getSwapchain(),
		    imageIndex,
		    {}};

		auto presentResult = presentQueue->queue.presentKHR(presentInfo);

		if (presentResult == vk::Result::eErrorOutOfDateKHR || presentResult == vk::Result::eSuboptimalKHR || framebufferResized)
		{
			framebufferResized = false;
			Utils::recreateSwapchainFromWindow(window, device, swapchain);
		}
		else if (presentResult != vk::Result::eSuccess)
		{
			return VulkanResult::VulkanError(presentResult, "Couldn't present to screen!");
		}

		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

		return VulkanResult::Success();
	}

	VulkanResult createRenderPass()
	{
		std::vector<vk::AttachmentDescription> colorAttachments = {{}};

		colorAttachments[0].setFormat(swapchain.getSwapchainConfig().surfaceFormat.format);
		colorAttachments[0].setSamples(vk::SampleCountFlagBits::e1);
		colorAttachments[0].setLoadOp(vk::AttachmentLoadOp::eClear);
		colorAttachments[0].setStoreOp(vk::AttachmentStoreOp::eStore);
		colorAttachments[0].setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
		colorAttachments[0].setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
		colorAttachments[0].setInitialLayout(vk::ImageLayout::eUndefined);
		colorAttachments[0].setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

		std::vector<vk::AttachmentReference> colorAttachmentReferences = {{}};

		colorAttachmentReferences[0].setAttachment(0);
		colorAttachmentReferences[0].setLayout(vk::ImageLayout::eColorAttachmentOptimal);

		std::vector<vk::SubpassDescription> subpasses = {
		    vk::SubpassDescription{}};

		subpasses[0].setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
		subpasses[0].setColorAttachments(colorAttachmentReferences);

		vk::SubpassDependency dependency = {};

		dependency.setSrcSubpass(vk::SubpassExternal);
		dependency.setDstSubpass(0);
		dependency.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		dependency.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		dependency.setSrcAccessMask({});
		dependency.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

		vk::RenderPassCreateInfo renderPassInfo{};

		renderPassInfo.setAttachments(colorAttachments);
		renderPassInfo.setSubpasses(subpasses);
		renderPassInfo.setDependencies(dependency);

		auto result = device.getDevice().createRenderPassUnique(renderPassInfo);

		VULKAN_QUICK_BAIL((result.result), "Couldn't create render pass!");
		result.value.swap(renderPass);

		return VulkanResult::Success();
	}

	VulkanResult createFrameDatas()
	{
		frameData.resize(MAX_FRAMES_IN_FLIGHT);

		LIB_QUICK_BAIL(createCommandBuffers());

		for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			vk::SemaphoreCreateInfo sempahoreInfo{
			    vk::SemaphoreCreateFlags{},
			    nullptr};

			vk::FenceCreateInfo fenceInfo{
			    vk::FenceCreateFlagBits::eSignaled,
			    nullptr};
			auto result1 = device.getDevice().createSemaphoreUnique(sempahoreInfo);
			VULKAN_QUICK_BAIL(result1.result, "Couldn't create imageAvailableSemaphore");
			auto result2 = device.getDevice().createSemaphoreUnique(sempahoreInfo);
			VULKAN_QUICK_BAIL(result2.result, "Couldn't create renderFinishedSemaphore");
			auto result3 = device.getDevice().createFenceUnique(fenceInfo);
			VULKAN_QUICK_BAIL(result3.result, "Couldn't create inFlightFence");

			result1.value.swap(frameData[i].imageAvailableSemaphore);
			result2.value.swap(frameData[i].renderFinishedSemaphore);
			result3.value.swap(frameData[i].inFlightFence);
			
			
			LIB_SET_AND_BAIL_RESULT_VALUE_UNSCOPPED(allocator.createBuffer(
				sizeof(UniformBuffer), vk::BufferUsageFlagBits::eUniformBuffer,
				VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
				VMA_MEMORY_USAGE_AUTO_PREFER_HOST),
				frameData[i].uniformBuffer);
				
			}
		std::cout << "Created sync objects" << std::endl;
		std::cout << "Created Uniform Buffers" << std::endl;
			

		return VulkanResult();
	}

	ResultValue<std::vector<vk::UniqueCommandBuffer>> allocateCommandBuffers(vk::CommandPool commandPool, size_t count, vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary)
	{
		// allocate main frame command buffers
		vk::CommandBufferAllocateInfo allocateInfo{};
		allocateInfo.setCommandPool(commandPool);
		allocateInfo.setLevel(level);
		allocateInfo.setCommandBufferCount(count);

		auto commandBuffersResult = device.getDevice().allocateCommandBuffersUnique(allocateInfo);

		VULKAN_QUICK_BAIL(commandBuffersResult.result, "Couldn't allocate command buffers!");

		return std::move(commandBuffersResult.value);
	}

	VulkanResult createCommandBuffers()
	{

		vk::CommandPoolCreateInfo commandPoolInfo{};
		commandPoolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
		commandPoolInfo.setQueueFamilyIndex(graphicsQueue->queueIndex.value());

		VULKAN_SET_AND_BAIL_RESULT_VALUE(
		    device.getDevice().createCommandPoolUnique(commandPoolInfo),
		    commandPool,
		    "Couldn't create command pool");
		std::cout << "Created main command pool" << std::endl;

		commandPoolInfo.setFlags(vk::CommandPoolCreateFlagBits::eTransient);
		commandPoolInfo.setQueueFamilyIndex(transferQueue->queueIndex.value());

		VULKAN_SET_AND_BAIL_RESULT_VALUE(
		    device.getDevice().createCommandPoolUnique(commandPoolInfo),
		    transferCommandPool,
		    "Couldn't create transfer command pool");
		std::cout << "Created transfer command pool" << std::endl;

		LIB_SET_AND_BAIL_RESULT_VALUE(
		    allocateCommandBuffers(commandPool.get(), MAX_FRAMES_IN_FLIGHT),
		    commandBuffers);

		for (size_t i = 0; i < commandBuffers.size(); ++i)
		{
			frameData[i].commandBuffer = commandBuffers[i].get();
		}

		return VulkanResult();
	}

	VulkanResult recordCommand(vk::CommandBuffer buffer, uint32_t imageIndex)
	{

		vk::CommandBufferBeginInfo commandBegin = {
		    vk::CommandBufferUsageFlags{},
		    nullptr,
		    nullptr};

		VULKAN_QUICK_BAIL(buffer.begin(commandBegin), "Couldn't begin command buffer!");

		auto clearColors = {vk::ClearValue{vk::ClearColorValue{0.0f, 0.0f, 0.0f, 1.0f}}};

		vk::Extent2D swapchainExtent = swapchain.getSwapchainConfig().extent;

		vk::RenderPassBeginInfo passBegin = {
		    renderPass.get(),
		    swapchain.getFramebuffers()[imageIndex].get(),
		    {
		        vk::Offset2D{0, 0},
		        swapchainExtent,
		    },
		    clearColors,
		    nullptr};

		vk::Buffer vertexBuffers[] = {vertexBuffer.buffer};
		VmaAllocationInfo vertexAlloc;
		vmaGetAllocationInfo(allocator.getAllocator(), vertexBuffer.allocation, &vertexAlloc);
		vk::DeviceSize vertexBufferOffsets = {0};

		buffer.bindVertexBuffers(0, vertexBuffers, vertexBufferOffsets);

		VmaAllocationInfo indexAlloc;
		vmaGetAllocationInfo(allocator.getAllocator(), indexBuffer.allocation, &indexAlloc);

		buffer.bindIndexBuffer(indexBuffer.buffer, 0, vk::IndexType::eUint32);

		buffer.beginRenderPass(passBegin, vk::SubpassContents::eInline);
		buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.getPipeline());


		vk::Viewport viewport{
		    0.0f,
		    0.0f,
		    static_cast<float>(swapchainExtent.width),
		    static_cast<float>(swapchainExtent.height),
		    0.0f,
		    1.0f,
		};

		buffer.setViewport(0, viewport);

		vk::Rect2D scissor{
		    vk::Offset2D{0, 0},
		    swapchainExtent};

		buffer.setScissor(0, scissor);

		buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.getPipelineLayout(), 0, {frameData[currentFrame].descriptorSet.get()}, {});

		buffer.drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

		buffer.endRenderPass();

		VULKAN_QUICK_BAIL(buffer.end(), "Couldn't end recording of command buffer!");

		return VulkanResult::Success();
	}

	VulkanResult fillVertexBuffer()
	{

		void *data;
		VULKAN_QUICK_BAIL((vk::Result)vmaMapMemory(allocator.getAllocator(), vertexBuffer.allocation, (void **)&data), "Couldn't map vertex buffer memory");

		memcpy(data, vertices.data(), vertices.size() * sizeof(Vertex));
		vmaUnmapMemory(allocator.getAllocator(), vertexBuffer.allocation);

		std::cout << "Filled vertex buffer !" << std::endl;

		void *indexData;
		VULKAN_QUICK_BAIL((vk::Result)vmaMapMemory(allocator.getAllocator(), indexBuffer.allocation, (void **)&indexData), "Couldn't map index buffer memory");

		memcpy(indexData, indices.data(), indices.size() * sizeof(uint32_t));
		vmaUnmapMemory(allocator.getAllocator(), indexBuffer.allocation);

		std::cout << "Filled index buffer !" << std::endl;
		return VulkanResult::Success();
	}

	VulkanResult createDescriptorSetLayout() {
		auto binding = UniformBuffer::getBindingDescription();
		vk::DescriptorSetLayoutCreateInfo createInfo{};
		createInfo.setBindings(binding);

		VULKAN_SET_AND_BAIL_RESULT_VALUE(device.getDevice().createDescriptorSetLayoutUnique(createInfo), descriptorSetLayout, "Couldn't create descriptor set layout!");

		return VulkanResult::Success();
	}

	void updateUniformBuffer(uint32_t currentImage) {
		auto& buffer = frameData[currentImage].uniformBuffer;
		auto ubo = UniformBuffer{};
		ubo.model = glm::translate(glm::vec3(1.0, 0, 0));
		vk::Extent2D swapchainExtent = swapchain.getSwapchainConfig().extent;

		ubo.projection = glm::perspective(glm::radians(45.0f), (float)swapchainExtent.width / (float) swapchainExtent.height, 0.1f, 10.0f);
		ubo.projection[1][1] *= -1;

		ubo.view = glm::lookAt(
			glm::vec3(0.0f, 0.0f, 3.0f),  // Camera position (in front of the object)
			glm::vec3(0.0f, 0.0f, 0.0f),  // Look at the origin
			glm::vec3(0.0f, 1.0f, 0.0f)   // Up direction
		);

		void* uniformDataMap;
		vmaMapMemory(allocator.getAllocator(), buffer.allocation, &uniformDataMap);

		memcpy(uniformDataMap, &ubo, sizeof(ubo));

		vmaUnmapMemory(allocator.getAllocator(), buffer.allocation);



	}

	VulkanResult createDescriptorPool() 
	{
		vk::DescriptorPoolSize descriptorPoolSize{};
		descriptorPoolSize.setType(vk::DescriptorType::eUniformBuffer);
		descriptorPoolSize.setDescriptorCount(MAX_FRAMES_IN_FLIGHT);

		vk::DescriptorPoolCreateInfo createInfo{};
		createInfo.setPoolSizes({descriptorPoolSize});
		createInfo.setMaxSets(static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT));

		VULKAN_SET_AND_BAIL_RESULT_VALUE(device.getDevice().createDescriptorPoolUnique(createInfo), descriptorPool, "Coudln't create descriptor pool");

		return VulkanResult::Success();
	}

	VulkanResult createDescriptorSets()
	{
		std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout.get());
		vk::DescriptorSetAllocateInfo allocInfo{};
		allocInfo.setDescriptorPool(descriptorPool.get());
		allocInfo.setSetLayouts(layouts);

		std::vector<vk::UniqueDescriptorSet> descriptorsets;
		VULKAN_SET_AND_BAIL_RESULT_VALUE_UNSCOPPED(device.getDevice().allocateDescriptorSetsUnique(allocInfo), descriptorsets, "Couldn't create descriptor sets.");

		for(size_t i = 0; i < frameData.size(); ++i) {
			descriptorsets[i].swap(frameData[i].descriptorSet);
		}

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vk::DescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = frameData[i].uniformBuffer.buffer;
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBuffer);

            vk::WriteDescriptorSet descriptorWrite{};
            descriptorWrite.dstSet = frameData[i].descriptorSet.get();
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfo;

            device.getDevice().updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
        }


		return VulkanResult::Success();
	}

private:
	GLFWwindow *window;
	Instance instance;
	Device device;
	Swapchain swapchain;
	vk::UniqueRenderPass renderPass;
	
	vk::UniqueDescriptorSetLayout descriptorSetLayout;
	vk::UniqueDescriptorPool descriptorPool;


	GraphicsPipeline pipeline;
	Allocator allocator;
	vk::UniqueCommandPool commandPool, transferCommandPool;
	std::vector<vk::UniqueCommandBuffer> commandBuffers;

	QueueInformation *graphicsQueue;
	QueueInformation *presentQueue;
	QueueInformation *computeQueue;
	QueueInformation *transferQueue;

	Buffer vertexBuffer;
	Buffer indexBuffer;

	std::vector<FrameData> frameData;
	size_t currentFrame = 0;
	bool framebufferResized = false;
	size_t rendered_frames = 0;
};

extern "C"
{

	MODULES_DLL VulkanApplication *create_application()
	{

		return new VkApp();
	}
}
