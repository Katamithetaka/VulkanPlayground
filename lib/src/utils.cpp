
#include "common.hpp"
#include "command.hpp"
#include "module_loader.hpp"
#include <vulkan/vulkan.hpp>
#include <vulkan_app.hpp>
#include "utils.hpp"

namespace Vulkan::Utils {

    VulkanResult recreateSwapchainFromWindow(GLFWwindow* window, Device& device, Swapchain& swapchain)
    {

        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        while (width == 0 || height == 0)
        {
            glfwGetFramebufferSize(window, &width, &height);
            glfwWaitEvents();
        }

        LIB_QUICK_BAIL(device.querySwapchainSupportForDevice(swapchain.getSurface()));
        swapchain.getSwapchainConfig().extent = Utils::getExtentFromWindow(
            device.getPhysicalDevice().swapchainDetails.capabilities, 
            window
        );

        LIB_QUICK_BAIL(swapchain.recreateSwapchain());
        LIB_QUICK_BAIL(swapchain.recreateImageViews());
        LIB_QUICK_BAIL(swapchain.recreateFramebuffers());

        return VulkanResult::Success();
    }

}
