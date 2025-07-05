
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <thread>
#include <unordered_map>

#include <vulkan_app.hpp>
#include <set>

#include "GLFW/glfw3.h"
#include "SPIRV/GlslangToSpv.h"
#include "command.hpp"
#include "common.hpp"
#include "module_loader.hpp"
#include "vulkan.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_core.h"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_handles.hpp"
#include "vulkan/vulkan_hpp_macros.hpp"
#include "vulkan/vulkan_structs.hpp"
#include <glslang/Public/ShaderLang.h>

namespace Vulkan
{



	VulkanResult VulkanApplication::run()
	{
		LIB_QUICK_BAIL(init_vulkan());
		LIB_QUICK_BAIL(MainLoop());
		cleanup_vulkan();
		return VulkanResult::Success();
	}

	VulkanResult VulkanApplication::init_vulkan() {
		if(!glfwInit()) {
			return VulkanResult::GLFWError();
		}

		if(!glslang::InitializeProcess()) {
			return VulkanResult::GLSLangError("Couldn't initialize glslang");
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		VULKAN_HPP_DEFAULT_DISPATCHER.init();

		LIB_QUICK_BAIL(OnInit());

		return VulkanResult::Success();
	}


	void VulkanApplication::cleanup_vulkan()
	{	
		OnDestroy();

		glslang::FinalizeProcess();
		glfwTerminate();
	}

} // namespace Vulkan
