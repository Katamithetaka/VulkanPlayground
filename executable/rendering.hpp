
#include "module_loader.hpp"
#include "common.hpp"
#include "command.hpp"
#include <vulkan_app.hpp>

#include <filesystem>
#include <vector>
#include <optional>
#include <concepts>


using DLLoader = dlloader::DLLoader;

struct IModule {
    DLLoader loader;
    IModule(DLLoader loader): loader{loader} {

    }

    void load() {
        loader.DLOpenLib();
        OnLoad();
    }

    virtual void OnLoad() = 0;

    virtual ~IModule() {
        loader.DLCloseLib();
        std::cout << "Closed Module " << loader._pathToLib << std::endl;
    }
};

struct Module: public IModule {
    std::optional<DLLoader::FunctionType<Vulkan::VulkanApplication*>> create_application;


    Module(DLLoader loader): IModule(loader) {

    }

    void OnLoad() override {
		create_application = loader.DLGetFunction < Vulkan::VulkanApplication * > ("create_application");

        std::cout << (create_application.has_value() ? "Create application has a value" : "Create application has no value.") << std::endl;
    }
};


template<typename T>
concept IsModule = std::is_base_of<IModule, T>::value;

template<IsModule Module>
std::vector<std::shared_ptr<Module>> getModules(std::filesystem::path path)  {
    std::vector<std::shared_ptr<Module>> returnVal{};

    for(const auto& file : std::filesystem::directory_iterator{path}) {
        std::filesystem::path path = file.path();

        // those don't realistically need to be here, they just are here so windows doesn't throw an error popup 
        // everytime it fails
        // 
		// the dll loading code deals just fine with files not being valid files and displays the correct errors
        // only reason why i'm doing the dll exclusion way instead of only loading .so/.dll files is because 
        // I'm not confident files will have the correct extensions depending on compilers and operating systems.

#ifdef _WIN32
        if(path.extension() == ".lib" || path.extension() == "lib") {
            continue;
        }

        if (path.extension() == ".exp" || path.extension() == "exp")
		{
			continue;
		}
#endif

        if(path.extension() == "pdb" || path.extension() == ".pdb") {
            continue;
        }

        std::cout << "Loading module " << path.string() << std::endl;

        std::shared_ptr<Module> mod (new Module(
            DLLoader{ path.string() }
        ));

        mod->load();

        returnVal.push_back(
            mod
        );
    }

    return returnVal;
}