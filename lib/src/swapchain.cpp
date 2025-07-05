#include "vulkan.hpp"
#include "swapchain.hpp"


namespace Vulkan
{
	VulkanResult Swapchain::createSurface(Instance& instance, GLFWwindow *window)
	{
		VkSurfaceKHR surface;
		auto result = vk::Result(glfwCreateWindowSurface(
		    (VkInstance)instance.getInstance(),
		    window, nullptr,
		    (VkSurfaceKHR *)&surface));
		VULKAN_QUICK_BAIL(result, "Couldn't create surface!");

		vk::detail::ObjectDestroy<vk::Instance, VULKAN_HPP_DEFAULT_DISPATCHER_TYPE>
		    _deleter(instance.getInstance(), nullptr, instance.getDispatcher());
		this->surface = vk::UniqueSurfaceKHR(vk::SurfaceKHR(surface), _deleter);

		return VulkanResult::Success();
	}

	VulkanResult Swapchain::createSwapchain(const SwapchainConfig &config)
	{
		swapchainConfig = config;
		return createSwapchain();
	}

	VulkanResult Swapchain::createSwapchain()
	{
		auto &swapChainDetails = swapchainConfig.device->getPhysicalDevice().swapchainDetails;

		uint32_t imageCount = swapChainDetails.capabilities.minImageCount;

		if (swapChainDetails.capabilities.maxImageCount > 0 &&
		    imageCount > swapChainDetails.capabilities.maxImageCount)
		{
			imageCount = swapChainDetails.capabilities.maxImageCount;
		}


		vk::SwapchainCreateInfoKHR swapChainInfo{};

		swapChainInfo.setFlags(vk::SwapchainCreateFlagsKHR{});
		swapChainInfo.setSurface(surface.get());
		swapChainInfo.setMinImageCount(imageCount);
		swapChainInfo.setImageFormat(swapchainConfig.surfaceFormat.format);
		swapChainInfo.setImageColorSpace(swapchainConfig.surfaceFormat.colorSpace);
		swapChainInfo.setImageArrayLayers(1);
		swapChainInfo.setImageExtent(swapchainConfig.extent);
		swapChainInfo.setImageUsage(swapchainConfig.imageUsage);
		swapChainInfo.setImageSharingMode(swapchainConfig.sharingConfig.sharingMode);
		swapChainInfo.setQueueFamilyIndices(swapchainConfig.sharingConfig.queueIndices);
		swapChainInfo.setPreTransform(swapChainDetails.capabilities.currentTransform);
		swapChainInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
		swapChainInfo.setClipped(swapchainConfig.clipped);
		swapChainInfo.setOldSwapchain(swapChain ? swapChain.get() : VK_NULL_HANDLE);

		VULKAN_SET_AND_BAIL_RESULT_VALUE(
		    swapchainConfig.device->getDevice().createSwapchainKHRUnique(
		        swapChainInfo,
		        nullptr,
		        swapchainConfig.device->getDispatcher()),
		    swapChain,
		    "Couldn't create swapchain!");

		std::cout << "Created swapchain" << std::endl;


		VULKAN_SET_AND_BAIL_RESULT_VALUE(
		    swapchainConfig.device->getDevice().getSwapchainImagesKHR(
		        swapChain.get(),
		        swapchainConfig.device->getDispatcher()),
		    swapchainImages,
		    "Couldn't get swapchain images!");


		std::cout << "Created Swap Chain with " << swapchainImages.size() << " images"
		          << std::endl;

		return VulkanResult();
	}

	VulkanResult Swapchain::createImageViews(const ImageViewConfig &_imageViewConfig)
	{
		imageViewConfig = _imageViewConfig;

		return createImageViews();
	}

	VulkanResult Swapchain::createImageViews()
	{
		vk::ImageViewCreateInfo imageViewInfo{};

		imageViewInfo.setComponents(imageViewConfig.components);
		imageViewInfo.setViewType(imageViewConfig.viewType);
		imageViewInfo.setFormat(swapchainConfig.surfaceFormat.format);
		imageViewInfo.setSubresourceRange(imageViewConfig.subResourceRange);

		imageViews.resize(swapchainImages.size());
		for (size_t i = 0; i < swapchainImages.size(); ++i)
		{
			imageViewInfo.setImage(swapchainImages[i]);

			VULKAN_SET_AND_BAIL_RESULT_VALUE(
			    swapchainConfig.device->getDevice().createImageViewUnique(
			        imageViewInfo,
			        nullptr,
			        swapchainConfig.device->getDispatcher()),
			    imageViews[i],
			    "Couldn't create image views!");
		}

		return VulkanResult::Success();
	}

	VulkanResult Swapchain::createFramebuffers(const FramebuffersConfig &_framebufferConfig)
	{
		framebufferConfig = _framebufferConfig;

		return createFramebuffers();
	}

	VulkanResult Swapchain::createFramebuffers()
	{
		vk::FramebufferCreateInfo framebufferInfo{};

		framebufferInfo.setRenderPass(framebufferConfig.renderPass);
		framebufferInfo.setWidth(swapchainConfig.extent.width);
		framebufferInfo.setHeight(swapchainConfig.extent.height);
		framebufferInfo.setLayers(framebufferConfig.layers);

		framebuffers.resize(imageViews.size());

		for (size_t i = 0; i < imageViews.size(); ++i)
		{
			vk::ImageView attachments[] = {imageViews[i].get()};
			framebufferInfo.setAttachments(attachments);

			VULKAN_SET_AND_BAIL_RESULT_VALUE(
			    swapchainConfig.device->getDevice().createFramebufferUnique(
			        framebufferInfo,
			        nullptr,
			        swapchainConfig.device->getDispatcher()),
				framebuffers[i],
			    "Couldn't create image views!");
		}

		return VulkanResult::Success();
	}

	VulkanResult Swapchain::recreateSwapchain()
	{
		return createSwapchain();
	}

	VulkanResult Swapchain::recreateFramebuffers()
	{
		return createFramebuffers();
	}

	VulkanResult Swapchain::recreateImageViews()
	{
		return createImageViews();
	}
}