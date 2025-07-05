#include "GLFW/glfw3.h"
#include "module_loader.hpp"
#include "common.hpp"
#include "command.hpp"

#include <chrono>
#include <filesystem>
#include <thread>
#include <vector>
#include <optional>
#include <concepts>

#include <vulkan_app.hpp>
#include "rendering.hpp"



int main(int _argc, const char** argv) {    
    auto dir = std::filesystem::path(argv[0]);
    dir.remove_filename();
    auto modules = getModules<Module>(dir / "modules");


    auto module = std::find_if(modules.begin(), modules.end(), [](auto& value) { return value->create_application.has_value(); });
	std::cout << (module == modules.end()) << std::endl;
    
    if (module != modules.end())
	{
		auto vulkanApp = (*module)->create_application.value()();

		std::cout << "Created vulkan app" << std::endl;

		auto result = vulkanApp->run();


		if (result.type() != Vulkan::VulkanResultVariants::Success)
		{
			std::cerr << Vulkan::to_string(result) << std::endl;
		}
   
        
        std::cin.get();
    }

    // if(argc > 1) {
        
        
    //     int i = 1; 
        
    //     for(;i < argc; ++i) {
    //         run(std::string(argv[i]), modules, context, dir / "modules");
    //     }

    //     return 0;
    // }

    // while(1) {
    //     std::string command = "";
    //     std::cin >> command;

    //     if(!run(command, modules, context, dir / "modules")) {
    //         return 0;
    //     }
    // }



    return 0;    
}