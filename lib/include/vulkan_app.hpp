#ifndef VULKAN_APP_HPP
#define VULKAN_APP_HPP

#include "vulkan.hpp"



namespace Vulkan
{







	class LIBRARY_DLL VulkanApplication
	{
	public:
		VulkanResult run();
		VulkanResult init_vulkan();
		void cleanup_vulkan();

		virtual VulkanResult OnInit() { return VulkanResult::Success(); }
		virtual VulkanResult MainLoop() { return VulkanResult::Success(); }
		virtual void OnDestroy() {}

	public:


		// @group events
	};
}

#endif // VULKAN_APP_HPP