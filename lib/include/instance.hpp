#ifndef LIB_VULKAN_INSTANCE_HPP
#define LIB_VULKAN_INSTANCE_HPP

#include "vulkan.hpp"

namespace Vulkan
{

	using DebugCallbackFN = std::function<void(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	                                           vk::DebugUtilsMessageTypeFlagsEXT messageTypes,
	                                           const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData)>;

	struct InstanceConfig
	{
		std::string appName = "";
		uint32_t appVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
		std::string engineName = "";
		uint32_t engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
		uint32_t vulkanVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);

		std::vector<const char *> requiredInstanceExtensions = {};

		bool enableLayers = false;
		std::vector<const char *> requiredLayers = {};

		bool usesWindow = false;

		bool createDebugCallbackMessenger = false;

		vk::DebugUtilsMessageSeverityFlagsEXT messageSeverity = {};
		vk::DebugUtilsMessageTypeFlagsEXT messageTypes = {};

		std::vector<DebugCallbackFN> debugCallback = {};

        vk::detail::DispatchLoaderDynamic* loader = &VULKAN_HPP_DEFAULT_DISPATCHER; 
	};

	class LIBRARY_DLL Instance
	{

	public:
		VulkanResult createInstance(const InstanceConfig &_config)
		{
			this->config = _config;

			vk::ApplicationInfo appInfo{};

			appInfo.setPApplicationName(config.appName.c_str());
			appInfo.setApplicationVersion(config.appVersion);
			appInfo.setPEngineName(config.engineName.c_str());
			appInfo.setEngineVersion(config.engineVersion);
			appInfo.setApiVersion(config.vulkanVersion);

			vk::InstanceCreateInfo instanceInfo{};
			instanceInfo.setFlags(vk::InstanceCreateFlags{});
			instanceInfo.setPApplicationInfo(&appInfo);

			if (config.enableLayers)
			{
				instanceInfo.setPEnabledLayerNames(config.requiredLayers);
			}

			if (config.usesWindow)
			{
				uint32_t glfwExtensionCount = 0;
				const char **glfwExtensions;
				glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

				std::vector<const char *> extensions = std::vector<const char *>(glfwExtensions,
				                                                                 glfwExtensions + glfwExtensionCount);

				// in this case .insert would actually be less memory efficient than resizing and
				// then inserting manually because it would grow exponentially instead of only for the required size.
				// for a few SIZE_OF_VOIDP bytes though it should be fine anyways.
				config.requiredInstanceExtensions.insert(config.requiredInstanceExtensions.end(), extensions.begin(), extensions.end());
			}

			instanceInfo.setPEnabledExtensionNames(config.requiredInstanceExtensions);

			VULKAN_SET_AND_BAIL_RESULT_VALUE(
				vk::createInstanceUnique(instanceInfo, nullptr, getDispatcher()),
				instance, 
				"Couldn't create instance!"
			);

			getDispatcher().init(instance.get());

			if (!config.createDebugCallbackMessenger)
				return VulkanResult::Success();

			LIB_SET_AND_BAIL_RESULT_VALUE(
				createDebugCallback(config.messageSeverity, config.messageTypes),
				debugMessenger
			);

			return VulkanResult::Success();
		}

		ResultValue<vk::UniqueDebugUtilsMessengerEXT> createDebugCallback(vk::DebugUtilsMessageSeverityFlagsEXT severity,
		                                                                  vk::DebugUtilsMessageTypeFlagsEXT types)
		{
			if (debugMessenger)
			{
				return VulkanResult::BadUsage("A debug messenger shouldn't be created multiple times for a single instance.");
			}

			vk::DebugUtilsMessengerCreateInfoEXT debugMessengerInfo(
			    vk::DebugUtilsMessengerCreateFlagsEXT{},
			    severity,
			    types,
			    debugCallback, this);


			VULKAN_SET_AND_BAIL_RESULT_VALUE_UNSCOPPED(
				instance->createDebugUtilsMessengerEXTUnique(debugMessengerInfo, nullptr, getDispatcher()),
				auto result, 
				"Couldn't create debug messenger!"
			);

			return ResultValue(std::move(result));
		}

		void addDebugCallback(DebugCallbackFN callback)
		{
			config.debugCallback.push_back(callback);
		}

		static VKAPI_ATTR vk::Bool32 VKAPI_CALL
		debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		              vk::DebugUtilsMessageTypeFlagsEXT messageTypes,
		              const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData,
		              void *pUserData)
		{

			for (const auto &debugCallback : reinterpret_cast<Instance *>(pUserData)->config.debugCallback)
			{
				debugCallback(messageSeverity, messageTypes, pCallbackData);
			}
			return VK_FALSE;
		}

		vk::Instance &getInstance() { return instance.get(); }
		vk::DebugUtilsMessengerEXT &getDebugMessenger() { return debugMessenger.get(); }
		// for debug purposes
		InstanceConfig &getConfig() { return config; }

		operator vk::Instance() { return instance.get(); }
		operator VkInstance() { return instance.get(); }

		vk::detail::DispatchLoaderDynamic& getDispatcher() {
			return *config.loader;
		}

	private:
		vk::UniqueInstance instance;
		vk::UniqueDebugUtilsMessengerEXT debugMessenger;

		// for debug purposes
		InstanceConfig config;
	};
}

#endif