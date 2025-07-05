#ifndef LIB_VULKAN_SWAPCHAIN_HPP
#define LIB_VULKAN_SWAPCHAIN_HPP

#include "GLFW/glfw3.h"
#include "vulkan.hpp"
#include "instance.hpp"
#include "device.hpp"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_handles.hpp"
#include "vulkan/vulkan_structs.hpp"

namespace Vulkan {

    struct SharingConfig {
        vk::SharingMode sharingMode;
        std::vector<uint32_t> queueIndices;
    };

    struct SwapchainConfig {
        vk::PresentModeKHR presentMode;
        vk::SurfaceFormatKHR surfaceFormat;
        SharingConfig sharingConfig;
        vk::Extent2D extent;
        Instance* instance;
        Device* device;
        vk::ImageUsageFlags imageUsage;
        bool clipped = true;
    };
    
    struct ImageViewConfig {
        vk::ImageViewType viewType = vk::ImageViewType::e2D;
        vk::ComponentMapping components = {
            vk::ComponentSwizzle::eIdentity,
            vk::ComponentSwizzle::eIdentity,
            vk::ComponentSwizzle::eIdentity,
            vk::ComponentSwizzle::eIdentity,
        };
        vk::ImageSubresourceRange subResourceRange = {
            vk::ImageAspectFlagBits::eColor,
            0, 1,
            0, 1
        };

    };

    struct FramebuffersConfig{

        vk::RenderPass renderPass;
        uint32_t layers = 1;
    };

    class LIBRARY_DLL Swapchain {

        public:
            VulkanResult createSurface(Instance& instance, GLFWwindow* window);

            VulkanResult createSwapchain(const SwapchainConfig& config);
            VulkanResult createImageViews(const ImageViewConfig& config);
            VulkanResult createFramebuffers(const FramebuffersConfig& config);

            VulkanResult recreateSwapchain();
            VulkanResult recreateImageViews();
            VulkanResult recreateFramebuffers();

            SwapchainConfig& getSwapchainConfig() { return swapchainConfig; }
            ImageViewConfig& getImageViewConfig() { return imageViewConfig; }
            FramebuffersConfig& getFramebufferConfig() { return framebufferConfig; }

            vk::SurfaceKHR getSurface() { return surface.get(); }
            vk::SwapchainKHR& getSwapchain() { return swapChain.get(); }
            std::vector<vk::Image>& getImages() { return swapchainImages; }
            std::vector<vk::UniqueImageView>& getImageViews() { return imageViews; }
            std::vector<vk::UniqueFramebuffer>& getFramebuffers() { return framebuffers; }


        private:
            
            VulkanResult createSwapchain();
            VulkanResult createImageViews();
            VulkanResult createFramebuffers();

            vk::UniqueSwapchainKHR swapChain;
            vk::UniqueSurfaceKHR surface;
            std::vector<vk::UniqueImageView> imageViews;
            std::vector<vk::UniqueFramebuffer> framebuffers;
       
            SwapchainConfig swapchainConfig;
            ImageViewConfig imageViewConfig;
            FramebuffersConfig framebufferConfig;



            std::vector<vk::Image> swapchainImages;
    };
}

#endif