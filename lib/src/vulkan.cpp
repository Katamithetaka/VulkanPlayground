#include "vulkan.hpp"
#include "utils.hpp"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace Vulkan::Utils {
    LIBRARY_DLL GLFWwindow * createWindow(uint32_t width, uint32_t height, const char* title, GLFWmonitor* monitor, GLFWwindow* share) {
        return glfwCreateWindow(width, height, title, monitor, share);
    }

    LIBRARY_DLL vk::Extent2D getExtentFromWindow(const vk::SurfaceCapabilitiesKHR& capabilities, GLFWwindow* window) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        } else {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
    
            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            std::cout << width << height << std::endl;
    
            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    
            return actualExtent;
        }
    }

}