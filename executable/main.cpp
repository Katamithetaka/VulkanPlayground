#include "module_loader.hpp"
#include "common.hpp"
#include "command.hpp"

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
    std::optional<DLLoader::FunctionType<ExecutionResult, std::shared_ptr<ExecutionContext>, int>> run;


    Module(DLLoader loader): IModule(loader) {

    }


    void OnLoad() override {
        run = loader.DLGetFunction<ExecutionResult, std::shared_ptr<ExecutionContext>, int>("run");
    }
};


template<typename T>
concept IsModule = std::is_base_of<IModule, T>::value;

template<IsModule Module>
std::vector<std::shared_ptr<Module>> getModules(std::filesystem::path path)  {
    std::vector<std::shared_ptr<Module>> returnVal{};

    for(const auto& file : std::filesystem::directory_iterator{path}) {
        std::filesystem::path path = file.path();

#ifdef _WIN32
        if(path.extension() == ".lib" || path.extension() == "lib") {
            continue;
        }
#endif

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

bool run(std::string_view command, std::vector<std::shared_ptr<Module>>& modules, std::shared_ptr<ExecutionContext> context, std::filesystem::path directory) {
        if(command == "reload" || command == "R") {

            modules.clear();

            runProgram(
                "cmake",
                { "--build", ".."}
            );
            
            modules = getModules<Module>(directory);
            for(auto& module : modules) {
                if(module->run.has_value()) {
                    auto func = module->run.value();
                    func(context, 170);
                }
            }
            return true;
        }

        if(command == "quit" || command == "Q") {
            return false;
        }
}



int main(int argc, const char** argv) {

    auto context = std::make_shared<ExecutionContext>();
    
    auto dir = std::filesystem::path(argv[0]);
    dir.remove_filename();
    auto modules = getModules<Module>(dir / "modules");
    if(argc > 1) {
        
        
        int i = 1; 
        
        for(;i < argc; ++i) {
            run(std::string(argv[i]), modules, context, dir / "modules");
        }

        return 0;
    }

    while(1) {
        std::string command = "";
        std::cin >> command;

        if(!run(command, modules, context, dir / "modules")) {
            return 0;
        }
    }


    return 0;    
}