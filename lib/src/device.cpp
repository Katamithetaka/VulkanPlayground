#include "device.hpp"
#include "vulkan.hpp"
#include "vulkan/vulkan_handles.hpp"
#include "vulkan/vulkan_structs.hpp"

#include <set>

namespace Vulkan
{
	VulkanResult Device::pickPhysicalDevice()
	{

		auto deviceExtensions = config.deviceExtensions;

		auto requiredQueues = config.queueRequirements;

		VULKAN_SET_AND_BAIL_RESULT_VALUE_UNSCOPPED(
		    config.instance->getInstance().enumeratePhysicalDevices(getDispatcher()),
		    auto physicalDevices,
		    "Couldn't enumerate physical devices!");

		GLFWwindow *window;
		vk::SurfaceKHR surface;
		if (config.requiresSwapchainSupport)
		{
			window = glfwCreateWindow(1, 1, "Pizza turtle", nullptr, nullptr);
			if (!window)
			{
				std::cout << "Couldn't create window!" << std::endl;
			}
			VULKAN_QUICK_BAIL((vk::Result)glfwCreateWindowSurface(config.instance->getInstance(), window, nullptr, (VkSurfaceKHR *)&surface), "Couldn't create surface for device!");

			if (std::find(config.deviceExtensions.begin(), config.deviceExtensions.end(), VK_KHR_SWAPCHAIN_EXTENSION_NAME) == config.deviceExtensions.end())
			{
				config.deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
			}
		}

		for (auto &physicalDevice : physicalDevices)
		{
			auto validQueueFamilies = selectQueueFamilies(physicalDevice);
			if (validQueueFamilies.result.type() != VulkanResultVariants::Success)
			{
				std::cerr << "Error while checking for physical device queue families: "
				          << Vulkan::to_string(validQueueFamilies.result) << std::endl;
				continue;
			}

			bool allHaveValue = validQueueFamilies.value.allHaveValue;
			auto swapChainSupport = querySwapchainSupport(physicalDevice, surface);

			if (swapChainSupport.result.type() != VulkanResultVariants::Success)
			{
				std::cerr << "Error while checking for swapchain support: "
				          << Vulkan::to_string(swapChainSupport.result) << std::endl;
				continue;
			}

			auto swapChainCapabilities = std::move(swapChainSupport.value);
			auto deviceExtensions = checkDeviceExtension(physicalDevice);

			if (deviceExtensions.result.type() != VulkanResultVariants::Success)
			{
				std::cerr << "Error while checking for device extensions: "
				          << Vulkan::to_string(deviceExtensions.result) << std::endl;
				continue;
			}

			bool suitable = allHaveValue && deviceExtensions.value;
			if (config.requiresSwapchainSupport && surface)
			{
				suitable = suitable && swapChainCapabilities.formats.size() != 0 &&
				           swapChainCapabilities.presentModes.size() != 0;
			}

			if (config.checkSuitability)
			{
				suitable = suitable && config.checkSuitability(physicalDevice);
			}

			if (suitable)
			{
				suitableDevices.push_back({
				    .physicalDevice = physicalDevice,
				    .deviceProperties = physicalDevice.getProperties(getDispatcher()),
				    .queues = validQueueFamilies.value.queueRequirements,
				    .swapchainDetails = swapChainCapabilities,
				});
			}
		}

		std::cout << suitableDevices.size() << " physical devices available"
		          << std::endl;

		if (!suitableDevices.size())
		{
			return VulkanResult::NoSuitablePhysicalDevice();
		}

		uint32_t bestDevice;
		if (config.pickBestPhysicalDevice)
		{
			LIB_SET_AND_BAIL_RESULT_VALUE(config.pickBestPhysicalDevice(suitableDevices), bestDevice);
		}
		else
		{
			bestDevice = 0;
		}

		selectedPhysicalDevice = bestDevice;
		physicalDevice = suitableDevices[bestDevice];

		if (config.requiresSwapchainSupport)
		{
			glfwDestroyWindow(window);
			config.instance->getInstance().destroySurfaceKHR(surface, nullptr, getDispatcher());
		}

		return VulkanResult::Success();
	}

	VulkanResult Device::createDevice(DeviceConfig deviceConfig)
	{
		config = deviceConfig;

		LIB_QUICK_BAIL(pickPhysicalDevice());

		return recreateDevice();
	}

	VulkanResult Device::recreateDevice()
	{
		physicalDevice = suitableDevices[selectedPhysicalDevice];

		std::unordered_map<uint32_t, std::vector<float>> queuePriorities = {};

		for (auto &queueFamily : physicalDevice.queues)
		{
			if (queuePriorities.contains(queueFamily.queueIndex.value()))
			{
				queuePriorities[queueFamily.queueIndex.value()].push_back(queueFamily.queuePriority);
			}
			else
			{
				queuePriorities[queueFamily.queueIndex.value()] = {queueFamily.queuePriority};
			}
		}

		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos{};
		for (auto &priorities : queuePriorities)
		{
			vk::DeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.setFlags(vk::DeviceQueueCreateFlags{});
			queueCreateInfo.setQueueFamilyIndex(priorities.first);
			queueCreateInfo.setQueuePriorities(priorities.second);
			queueCreateInfos.push_back(std::move(queueCreateInfo));
		}

		vk::DeviceCreateInfo deviceCreateInfo{};
		deviceCreateInfo.setFlags(vk::DeviceCreateFlags{});
		deviceCreateInfo.setQueueCreateInfos(queueCreateInfos);
		deviceCreateInfo.setPEnabledExtensionNames(config.deviceExtensions);
		if (config.instance->getConfig().enableLayers)
		{
			deviceCreateInfo.setPEnabledLayerNames(config.instance->getConfig().requiredLayers);
		}
		deviceCreateInfo.setPEnabledFeatures(&config.features);

		auto result = physicalDevice.physicalDevice.createDeviceUnique(deviceCreateInfo, nullptr, getDispatcher());

		VULKAN_QUICK_BAIL(result.result, "Couldn't create device!");

		result.value.swap(device);

		std::unordered_map<uint32_t, uint32_t> queueCounts;
		for (auto &queueFamily : physicalDevice.queues)
		{
			if (queueCounts.contains(queueFamily.queueIndex.value()))
			{
				queueCounts[queueFamily.queueIndex.value()] += 1;
			}
			else
			{
				queueCounts[queueFamily.queueIndex.value()] = 0;
			}

			std::cout << "Getting queue for index " << queueFamily.queueIndex.value() << "(" << queueCounts[queueFamily.queueIndex.value()] << ")" << "\n";

			queueFamily.queue = device->getQueue(queueFamily.queueIndex.value(),
			                                     queueCounts[queueFamily.queueIndex.value()]);

			if (!queueFamily.queue)
			{
				std::cout << "Couldn't get queue!" << std::endl;
			}
		}

		getDispatcher().init(device.get());

		return VulkanResult::Success();
	}

	ResultValue<bool> Device::checkDeviceExtension(vk::PhysicalDevice physicalDevice)
	{
		std::set<vk::ExtensionProperties> extensions;
		for (auto layer : config.layers)
		{
			auto result = physicalDevice.enumerateDeviceExtensionProperties(std::string(layer), getDispatcher());

			VULKAN_QUICK_BAIL(result.result, "Couldn't enumerate device extensions!");

			extensions.insert(result.value.begin(), result.value.end());
		}

		auto result = physicalDevice.enumerateDeviceExtensionProperties(nullptr, getDispatcher());

		VULKAN_QUICK_BAIL(result.result, "Couldn't enumerate device extensions!");

		extensions.insert(result.value.begin(), result.value.end());

		std::set<std::string> requiredExtensions(config.deviceExtensions.begin(),
		                                         config.deviceExtensions.end());

		for (const auto &extension : extensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	VulkanResult Device::querySwapchainSupportForDevice(vk::SurfaceKHR surface)
	{

		LIB_SET_AND_BAIL_RESULT_VALUE(
		    querySwapchainSupport(physicalDevice.physicalDevice, surface),
		    physicalDevice.swapchainDetails);

		return VulkanResult::Success();
	}

	ResultValue<SwapChainSupportDetails> Device::querySwapchainSupport(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface)
	{
		SwapChainSupportDetails result{};

		if (!config.requiresSwapchainSupport || surface == nullptr)
		{
			return result;
		}
		VULKAN_SET_AND_BAIL_RESULT_VALUE(
		    physicalDevice.getSurfaceCapabilitiesKHR(surface, getDispatcher()),
		    result.capabilities,
		    "Couldn't get surface capabilities!");

		VULKAN_SET_AND_BAIL_RESULT_VALUE(
		    physicalDevice.getSurfaceFormatsKHR(surface, getDispatcher()),
		    result.formats,
		    "Couldn't get surface formats!");

		auto presentModes = physicalDevice.getSurfacePresentModesKHR(surface, getDispatcher());

		VULKAN_SET_AND_BAIL_RESULT_VALUE(
			physicalDevice.getSurfacePresentModesKHR(surface, getDispatcher()), 
			result.presentModes, 
			"Couldn't get surface present modes");

		result.presentModes = presentModes.value;

		return result;
	}

	ResultValue<std::vector<QueueInformation>> Device::getQueueFamilies(vk::PhysicalDevice physicalDevice, const QueueInformation &match, vk::SurfaceKHR surface)
	{
		auto result = std::vector<QueueInformation>();

		auto queueFamilies = physicalDevice.getQueueFamilyProperties(getDispatcher());

		for (size_t queueFamilyIndex = 0; queueFamilyIndex < queueFamilies.size();
		     ++queueFamilyIndex)
		{
			auto queueFamilyProperties = queueFamilies[queueFamilyIndex];
			bool shouldAdd = true;
			if (match.requirePresentSupport && config.requiresSwapchainSupport && surface != nullptr)
			{
				auto result = physicalDevice.getSurfaceSupportKHR(queueFamilyIndex,
				                                                  surface, getDispatcher());
				if (result.result != vk::Result::eSuccess)
				{
					continue;
				};
				shouldAdd = shouldAdd && result.value;
			}
			shouldAdd = shouldAdd && ((queueFamilyProperties.queueFlags &
			                           match.requiredFlags) == match.requiredFlags);
			if (shouldAdd)
			{
				result.push_back(QueueInformation{
				    .requiredFlags = {},
				    .queueIndex = queueFamilyIndex,
				    .properties = queueFamilyProperties,
				});
			}
		}

		return result;
	}

	uint32_t count_ignore(
	    const std::vector<std::vector<QueueInformation>> &options,
	    uint32_t index, int begin = -1)
	{
		int i = 0;
		auto c = 0;
		for (auto &option : options)
		{
			if (i < begin)
			{
				continue;
			}
			if (std::find_if(option.begin(), option.end(), [index](auto pair)
			                 { return pair.queueIndex == index; }) != option.end())
			{
				++c;
			};
		}
		return c;
	}

	uint32_t count_those_with_no_other_options(
	    const std::vector<std::vector<QueueInformation>> &options,
	    uint32_t index, int begin = -1)
	{
		int i = 0;
		auto c = 0;
		for (auto &option : options)
		{
			if (i < begin)
			{
				continue;
			}
			if (std::find_if(option.begin(), option.end(), [index](auto pair)
			                 { return pair.queueIndex != index; }) == option.end())
			{
				++c;
			};
		}
		return c;
	}

	ResultValue<FoundQueues> Device::selectQueueFamilies(vk::PhysicalDevice physicalDevice)
	{
		FoundQueues returnValue{
		    .allHaveValue = false,
		    .queueRequirements = config.queueRequirements};

		std::vector<std::vector<QueueInformation>> options(returnValue.queueRequirements.size());
		for (size_t i = 0; i < options.size(); ++i)
		{
			auto result = getQueueFamilies(physicalDevice, returnValue.queueRequirements[i]);

			LIB_QUICK_BAIL(result.result);

			options[i] = result.value;
		}

		for (size_t i = 0; i < options.size(); ++i)
		{
			std::cout << options[i].size() << " queues available for queue " << (returnValue.queueRequirements[i].name == "" ? returnValue.queueRequirements[i].name : std::to_string(i))
			          << std::endl;
		}

		for (size_t i = 0; i < options.size(); ++i)
		{
			auto option = options[i];
			for (auto &family : option)
			{
				auto family_count = count_ignore(options, family.queueIndex.value());
				if (family_count <= family.properties.queueCount)
				{
					returnValue.queueRequirements[i].queueIndex = family.queueIndex;
					returnValue.queueRequirements[i].properties = family.properties;
					break;
				}
			}

			if (!returnValue.queueRequirements[i].queueIndex.has_value())
			{
				for (size_t y = 0; y < option.size(); ++y)
				{
					auto &family = option[y];
					if (count_those_with_no_other_options(options, y, i) <=
					    family.properties.queueCount)
					{
						returnValue.queueRequirements[i].queueIndex = family.queueIndex;
						returnValue.queueRequirements[i].properties = family.properties;

						for (size_t z = i + 1; z < options.size(); ++z)
						{
							auto erase_index = std::find_if(
							    options[z].begin(), options[z].end(), [&family](auto &option)
							    { return option.queueIndex == family.queueIndex; });
							if (erase_index != options[z].end() &&
							    std::find_if(options[z].begin(), options[z].end(),
							                 [&family](auto &option)
							                 {
								                 return option.queueIndex != family.queueIndex;
							                 }) != options[z].end())
							{
								options[z].erase(erase_index);
							}
						}
						break;
					}
				}
			}

			if (!returnValue.queueRequirements[i].queueIndex.has_value())
			{
				// now that's fucked. we literally don't have an option apparently.
			}
		}
		returnValue.allHaveValue = true;

		for (size_t i = 0; i < returnValue.queueRequirements.size(); ++i)
		{
			if (!returnValue.queueRequirements[i].queueIndex.has_value())
			{
				std::cerr << "Couldn't find a queue for " << (returnValue.queueRequirements[i].name == "" ? returnValue.queueRequirements[i].name : std::to_string(i)) << std::endl;
				returnValue.allHaveValue = false;
			}
		}

		return returnValue;
	}

}