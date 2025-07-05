#ifndef LIB_VULKAN_HPP_INCLUDED
#define LIB_VULKAN_HPP_INCLUDED

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#define VULKAN_HPP_NO_EXCEPTIONS 1
#define VK_NO_PROTOTYPES 1
#define VULKAN_HPP_ASSERT(x)
#define GLFW_INCLUDE_VULKAN
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#define VMA_CALL_PRE LIBRARY_DLL
#define VULKAN_HPP_STORAGE_API LIBRARY_DLL
#define GLM_ENABLE_EXPERIMENTAL 1
#include "common.hpp"
#include "vk_mem_alloc.h"

#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_handles.hpp"
#include "vulkan/vulkan_structs.hpp"
#include <GLFW/glfw3.h>
#include <functional>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>

#include "SPIRV/GlslangToSpv.h"
#include <filesystem>
#include <fstream>
#include <glslang/Public/ShaderLang.h>
#include <iostream>

#define LIB_CAT_(a, b) a ## b
#define LIB_CAT(a, b) LIB_CAT_(a, b)
#define LIB_VARNAME(Var) LIB_CAT(Var, __LINE__)

#define LIB_QUICK_BAIL(x)                                     \
	do                                                        \
	{                                                         \
		auto __result = x;                                    \
		if (__result.type() != VulkanResultVariants::Success) \
		{                                                     \
			return __result;                                  \
		}                                                     \
	} while (false)
#define VULKAN_QUICK_BAIL(x, description)                                                      \
	do                                                                                         \
	{                                                                                          \
		auto __result = x;                                                                     \
		if (__result != vk::Result::eSuccess)                                                  \
		{                                                                                      \
			return VulkanResult::VulkanError(__result, description + vk::to_string(__result)); \
		}                                                                                      \
	} while (false)

#define LIB_SET_AND_BAIL_RESULT_VALUE(x, r)           \
	do                                                \
	{                                                 \
		LIB_SET_AND_BAIL_RESULT_VALUE_UNSCOPPED(x, r) \
	} while (false)

#define LIB_SET_AND_BAIL_RESULT_VALUE_UNSCOPPED(x, r) \
	auto LIB_VARNAME(___result) = x;                     \
	LIB_QUICK_BAIL(LIB_VARNAME(___result).result);                 \
	r = std::move(LIB_VARNAME(___result).value);

#define VULKAN_SET_AND_BAIL_RESULT_VALUE(x, r, description)            \
	do                                                                 \
	{                                                                  \
		VULKAN_SET_AND_BAIL_RESULT_VALUE_UNSCOPPED(x, r, description); \
	} while (false)

#define VULKAN_SET_AND_BAIL_RESULT_VALUE_UNSCOPPED(x, r, description) \
                                                                      \
	auto LIB_VARNAME(___result) = x;                                     \
	VULKAN_QUICK_BAIL(LIB_VARNAME(___result) .result, description);                 \
	r = std::move(LIB_VARNAME(___result) .value)


namespace Vulkan
{

	enum class VulkanResultVariants
	{
		Success,
		GLFWError,
		VulkanError,
		NoSuitablePhysicalDevice,
		GLSLangError,
		BadUsage
	};

	class LIBRARY_DLL VulkanResult
	{
	private:
		VulkanResultVariants t;
		std::string error;
		vk::Result result{vk::Result::eSuccess};

		VulkanResult(VulkanResultVariants _t, std::string description = "") : t{_t}, error{description}
		{
		}

		VulkanResult(VulkanResultVariants _t, vk::Result result, std::string description = "") : t{_t}, error{description}, result{result}
		{
		}

		static inline std::string glfw_error()
		{
			const char *returnVal;
			std::cout << "here" << std::endl;
			if (!glfwGetError(&returnVal))
			{
				return "No error";
			}

			return std::string("GLFW Error: ") + returnVal;
		}

	public:
		VulkanResult() : t{VulkanResultVariants::Success}, error{"Success"}
		{
		}

		VulkanResultVariants type() const { return t; }
		std::string description() const { return error; }
		vk::Result vulkan_result() const { return result; }

		static VulkanResult Success() { return VulkanResult(); }
		static VulkanResult BadUsage(std::string result) { return VulkanResult(VulkanResultVariants::BadUsage, result); }

		static VulkanResult NoSuitablePhysicalDevice() { return VulkanResult(VulkanResultVariants::NoSuitablePhysicalDevice, "No suitable physical devices (maybe because of required queue families)"); }
		static VulkanResult GLFWError() { return VulkanResult(VulkanResultVariants::GLFWError, glfw_error()); }
		static VulkanResult VulkanError(vk::Result result, std::string description) { return VulkanResult(VulkanResultVariants::VulkanError, result, description); }
		static VulkanResult GLSLangError(std::string errorLog) { return VulkanResult(VulkanResultVariants::GLSLangError, errorLog); }
	};

	inline std::string to_string(const VulkanResult &result)
	{
		return result.description();
	}

	template <typename T>
	struct ResultValue
	{
		VulkanResult result;
		T value;

		ResultValue(VulkanResult result) : result{result}
		{
		}

		ResultValue(T &&value) : value{std::move(value)}
		{
		}
	};

}

#endif