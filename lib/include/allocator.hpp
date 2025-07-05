

#include "vulkan.hpp"
#include "device.hpp"
#include "vk_mem_alloc.h"
#include "vulkan/vulkan.hpp"

namespace Vulkan {
    struct VulkanAllocatorConfig {
        Device* device;
        vk::Instance instance;
    };

    struct Buffer
    {
        VmaAllocation allocation;
        vk::Buffer buffer;
    };

    class LIBRARY_DLL Allocator {

        
        public:

        VulkanResult createAllocator(VulkanAllocatorConfig allocConfig);

        VmaAllocator getAllocator() { return allocator; }
        VulkanAllocatorConfig& getConfig() { return config; }

        ResultValue<Buffer> createBuffer(size_t size,
            vk::BufferUsageFlagBits bufferUsage,
            VmaAllocationCreateFlags vmaAllocFlags = {},
            vk::MemoryPropertyFlags requiredFlags = {},
            VmaMemoryUsage vmaUsage = {});


        private:

        VulkanAllocatorConfig config;
        VmaAllocator allocator;
    } ;
}