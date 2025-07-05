#ifndef LIB_VULKAN_DEVICE_HPP
#define LIB_VULKAN_DEVICE_HPP

#include "vulkan.hpp"
#include "GLFW/glfw3.h"
#include "instance.hpp"

namespace Vulkan {
	struct QueueInformation
	{
		vk::QueueFlags requiredFlags;
		bool requirePresentSupport = false;
        float queuePriority = 1.0;
        std::string name = "";


        // once created this object will be filled
        
        std::optional<uint32_t> queueIndex = 0;
        vk::QueueFamilyProperties properties{};
        vk::Queue queue{};
	};

    struct SwapChainSupportDetails
	{
		vk::SurfaceCapabilitiesKHR capabilities;
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR> presentModes;
	};

    
    struct PhysicalDevice {
        vk::PhysicalDevice physicalDevice;
        vk::PhysicalDeviceProperties deviceProperties;
        std::vector<QueueInformation> queues;
        SwapChainSupportDetails swapchainDetails;
    };

    struct DeviceConfig {
        Instance* instance;
        std::vector<QueueInformation> queueRequirements = {};
        vk::PhysicalDeviceFeatures features;

        std::function<bool(vk::PhysicalDevice)> checkSuitability = {};
        std::function<ResultValue<uint32_t>(const std::vector<PhysicalDevice> &physicalDevices)> pickBestPhysicalDevice = {};

		std::vector<const char *> deviceExtensions = {};
		std::vector<const char *> layers = {};
        
        // if this is set to true, the swapchain KHR extension will automatically be added to the device
        bool requiresSwapchainSupport = false;

        vk::detail::DispatchLoaderDynamic* loader = &VULKAN_HPP_DEFAULT_DISPATCHER; 

    };

    struct FoundQueues {
        bool allHaveValue = false;
        std::vector<QueueInformation> queueRequirements = {};
    };


    class LIBRARY_DLL Device {
        public:
            VulkanResult createDevice(DeviceConfig);
            
            VulkanResult recreateDevice();

            VulkanResult changeDevice(uint32_t deviceIndex);

            QueueInformation& getQueue(uint32_t index) {
                return physicalDevice.queues[index];
            }

            std::optional<vk::Queue> findQueue(std::string_view name) {
                auto queue = std::find_if(physicalDevice.queues.begin(), physicalDevice.queues.end(), [name](auto& queue){
                    return queue.name == name;
                });

                if(queue != physicalDevice.queues.end()) {
                    return queue->queue;
                }

                return std::nullopt;
            }

            std::vector<QueueInformation>& getQueues() {
                return physicalDevice.queues;
            }

            vk::Device& getDevice() {
                return device.get();
            }

            PhysicalDevice& getPhysicalDevice() {
                return physicalDevice;
            }

            uint32_t getSelectedDeviceIndex() {
                return selectedPhysicalDevice;
            }

            std::vector<PhysicalDevice>& getSuitableDevices() {
                return suitableDevices;
            }

            VulkanResult querySwapchainSupportForDevice(vk::SurfaceKHR surface);
            
            vk::detail::DispatchLoaderDynamic& getDispatcher() {
                return *config.loader;
            }

        protected:

            VulkanResult pickPhysicalDevice();
            ResultValue<bool> checkDeviceExtension(vk::PhysicalDevice physicalDevice);
            ResultValue<SwapChainSupportDetails> querySwapchainSupport(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface);


            ResultValue<std::vector<QueueInformation>> getQueueFamilies(vk::PhysicalDevice physicalDevice, const QueueInformation& match, vk::SurfaceKHR surface = nullptr);
            ResultValue<FoundQueues> selectQueueFamilies(vk::PhysicalDevice);

        private:


        vk::UniqueDevice device;
        
        std::vector<PhysicalDevice> suitableDevices;
        PhysicalDevice physicalDevice;
        uint32_t selectedPhysicalDevice;


        DeviceConfig config;
    };
}

#endif