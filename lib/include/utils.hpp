#ifndef LIB_VULKAN_UTILS_HPP
#define LIB_VULKAN_UTILS_HPP

#include <vulkan.hpp>
#include "GLFW/glfw3.h"
#include "common.hpp"
#include "device.hpp"
#include "glslang/Public/ShaderLang.h"
#include "swapchain.hpp"
namespace Vulkan::Utils
{
	inline std::string readFile(std::filesystem::path fileName)
	{
		std::ifstream is(fileName, std::ios_base::in);
		if (is.is_open())
		{
			std::ostringstream contents;
			contents << is.rdbuf();
			return contents.str();
		}
		else
		{
			return "";
		}
	}

	inline constexpr std::array<float, 4> rgba(int r, int g, int b, int a = 255)
	{

		return {r / 255.f, g / 255.f, b / 255.f, a / 255.f};
	}

	inline constexpr std::array<float, 4> rgba(int hex)
	{
		return rgba(hex >> 24 & 0xff, hex >> 16 & 0xff, hex >> 8 & 0xff, hex & 0xff);
	}

	inline constexpr std::array<float, 3> rgb(int r, int g, int b)
	{
		return {r / 255.f, g / 255.f, b / 255.f};
	}

	inline constexpr std::array<float, 3> rgb(int hex)
	{
		return rgb(hex >> 16 & 0xff, hex >> 8 & 0xff, hex & 0xff);
	}

    inline ResultValue<std::vector<std::vector<uint32_t>>> compileShaders(std::vector<std::pair<std::filesystem::path, EShLanguage>> shaders)
    {
        glslang::TProgram program{};
        EShMessages messages = static_cast<EShMessages>(
            EShMsgDefault | EShMsgVulkanRules | EShMsgSpvRules | EShMsgDebugInfo);

        std::vector<std::vector<uint32_t>> returnValue;
        std::vector<glslang::TShader*> preparsedShaders;
        
        for (const auto &s : shaders)
        {
            auto language = s.second;
            glslang::TShader* s1 = new glslang::TShader(language);
            preparsedShaders.push_back(s1);
            auto& shader = *preparsedShaders[preparsedShaders.size() - 1];
            auto shaderSource = readFile(s.first);
            auto shaderSources = shaderSource.c_str();

            if (shaderSource == "")
            {
                return VulkanResult::GLSLangError("Couldn't read file " + s.first.string());
            }

            std::cout << "Parsing " << shaderSources << std::endl;
            shader.setStrings(&shaderSources, 1);
            shader.setEnvInput(glslang::EShSourceGlsl, language,
                               glslang::EShClientVulkan, 100);
            shader.setEnvClient(glslang::EShClientVulkan,
                                glslang::EShTargetVulkan_1_0);
            shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);

            std::string vertOutput;
            glslang::TShader::ForbidIncluder includer;

            if (!shader.preprocess(GetDefaultResources(), 100, ENoProfile, false,
                                   false, messages, &vertOutput, includer))
            {

                return VulkanResult::GLSLangError(
                    std::string("Shader preprocessing failed: ") +
                    shader.getInfoLog());
            }

            if (!shader.parse(GetDefaultResources(), 100, false, messages))
            {
                return VulkanResult::GLSLangError(std::string("Shader parsing failed: ") +
                                                  shader.getInfoLog());
            }
        }

        for (auto &shader : preparsedShaders)
        {
            program.addShader(shader);
        }

        if (!program.link(messages))
        {
            std::cout <<  program.getInfoDebugLog() << std::endl;

            return VulkanResult::GLSLangError(std::string("Program linking failed: ") +
                                              program.getInfoLog());
        }

        for (const auto &shader : shaders)
        {
            auto language = shader.second;


            // Get SPIR-V code for shader
            glslang::TIntermediate *shaderIntermediate =
                program.getIntermediate(language);
            if (!shaderIntermediate)
            {
                return VulkanResult::GLSLangError(
                    std::string(
                        "Failed to get intermediate representation for shader: ") +
                    program.getInfoLog());
            }

            std::vector<unsigned int> shaderSpirv;
            glslang::GlslangToSpv(*shaderIntermediate, shaderSpirv);
            returnValue.push_back(shaderSpirv);
        }

        return returnValue;
    }

    LIBRARY_DLL vk::Extent2D getExtentFromWindow(const vk::SurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

    LIBRARY_DLL VulkanResult recreateSwapchainFromWindow(GLFWwindow* window, Device& device, Swapchain& swapchain);

    inline bool defaultCheckSuitability(vk::PhysicalDevice _)
	{
		return true;
	}

	inline vk::SurfaceFormatKHR defaultChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats)
	{
		for (const auto &availableFormat : availableFormats)
		{
			if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
			{
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	inline vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes, vk::PresentModeKHR preferredPresentMode = vk::PresentModeKHR::eFifo)
	{
		for (const auto &availablePresentMode : availablePresentModes)
		{
			std::cout << "Available mode: " << vk::to_string(availablePresentMode) << std::endl;
			if (availablePresentMode == preferredPresentMode)
			{
				return availablePresentMode;
			}
		}

		return vk::PresentModeKHR::eFifo;
	}

	inline ResultValue<uint32_t> defaultPickBestPhysicalDevice(const std::vector<PhysicalDevice> &physicalDevices)
	{
		std::vector<uint32_t> scores;
		for (size_t i = 0; i < physicalDevices.size(); ++i)
		{
			auto &physicalDevice = physicalDevices[i];
			auto properties = physicalDevice.deviceProperties;
			uint32_t score = 0;

			if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
			{
				score += 10000; // discrete gpus are usually more performant.
			}

			score += properties.limits.maxImageDimension2D;
			scores.push_back(score);
		}

		uint32_t maxIndex = 0;
		for (size_t i = 0; i < scores.size(); ++i)
		{
			if (scores[i] > scores[maxIndex])
			{
				maxIndex = i;
			}
		}

		auto properties = physicalDevices[maxIndex].deviceProperties;

		std::cerr << "Picked device: " << properties.deviceName << std::endl;

		return maxIndex;
	}

}

#endif