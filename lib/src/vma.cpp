#define VMA_IMPLEMENTATION 1
#include "vulkan.hpp"
#include "vk_mem_alloc.h"

#include "allocator.hpp"

namespace Vulkan {
    VulkanResult Allocator::createAllocator(VulkanAllocatorConfig allocConfig) {
        config = allocConfig;
        
        VmaAllocatorCreateInfo allocatorInfo{};
        allocatorInfo.device = config.device->getDevice();
        allocatorInfo.instance = config.instance;
        allocatorInfo.physicalDevice = config.device->getPhysicalDevice().physicalDevice;
        
        VmaVulkanFunctions vulkanFunctions = {};
        vulkanFunctions.vkGetInstanceProcAddr = config.device->getDispatcher().vkGetInstanceProcAddr;
        vulkanFunctions.vkGetDeviceProcAddr = config.device->getDispatcher().vkGetDeviceProcAddr;

        allocatorInfo.pVulkanFunctions = &vulkanFunctions;
    
        VULKAN_QUICK_BAIL((vk::Result)vmaCreateAllocator(&allocatorInfo, &allocator), "Couldn't create allocator!");
        
        return VulkanResult::Success();
    }

    ResultValue<Buffer> Allocator::createBuffer(size_t size,
        vk::BufferUsageFlagBits bufferUsage,
        VmaAllocationCreateFlags vmaAllocFlags,
        vk::MemoryPropertyFlags requiredFlags,
        VmaMemoryUsage vmaUsage)
    {
        vk::BufferCreateInfo bufferInfo{};

        bufferInfo.setSize(size);
        bufferInfo.setUsage(bufferUsage);

        VmaAllocationCreateInfo vmaAlloc{};
        vmaAlloc.flags = vmaAllocFlags;
        vmaAlloc.requiredFlags = (VkMemoryPropertyFlags)requiredFlags;
        vmaAlloc.usage = vmaUsage;

        Buffer returnValue{};

        VULKAN_QUICK_BAIL((vk::Result)vmaCreateBuffer(allocator,
                                (VkBufferCreateInfo *)&bufferInfo, &vmaAlloc, (VkBuffer *)&returnValue.buffer, &returnValue.allocation, nullptr),
        "Couldn't create buffer!");

        return returnValue;
    }

}