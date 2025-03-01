#ifndef MODULE_LOADER_HPP
#define MODULE_LOADER_HPP
#ifdef _WIN32


#include <iostream>
#include "Windows.h"
#include <optional>

namespace dlloader
{
	class DLLoader
	{

	public:
		HMODULE			_handle;
		std::string		_pathToLib;

	public:
		DLLoader(std::string_view pathToLib) :
			_handle(nullptr), _pathToLib(pathToLib)
		{}

		~DLLoader() = default;

		void DLOpenLib()
		{
			if (!(_handle = LoadLibrary(_pathToLib.c_str()))) {
				std::cerr << "Can't open and load " << _pathToLib << std::endl;
			}
		}

        template<typename ReturnType, typename ...Args>
        using FunctionType = ReturnType (*) (Args...);

        template<typename ReturnType, typename ...Args>
 		std::optional<FunctionType<ReturnType, Args...>> DLGetFunction(std::string_view name)
		{
			using returnType = FunctionType<ReturnType, Args...>;

			auto function = reinterpret_cast<returnType>(
				GetProcAddress(_handle, name.data()));

			if (!function) {
                return std::nullopt;
			}

            return function;
		}

		void DLCloseLib()
		{
			if (FreeLibrary(_handle) == 0) {
				std::cerr << "Can't close " << _pathToLib << std::endl;
			}
		}

	};

}

#else 

#include <iostream>
#include <dlfcn.h>
#include <optional>

namespace dlloader
{
	class DLLoader
	{

	public:
		void* _handle;
		std::string		_pathToLib;

	public:

		DLLoader(std::string pathToLib) :
			_handle(nullptr),
			_pathToLib(pathToLib)
		{
		}

		~DLLoader() = default;

		void DLOpenLib()
		{
			std::cout << _pathToLib.c_str() << std::endl;
			if (!(_handle = dlopen(_pathToLib.c_str(), RTLD_NOW | RTLD_LAZY))) {
				std::cerr << dlerror() << std::endl;
			}
		}

		template<typename ReturnType, typename ...Args>
        using FunctionType = ReturnType (*) (Args...);


        template<typename ReturnType, typename ...Args>
 		std::optional<FunctionType<ReturnType, Args...>> DLGetFunction(std::string_view name)
		{
			using returnType = FunctionType<ReturnType, Args...>;


			auto test = reinterpret_cast<returnType>(dlsym(_handle, name.data()));

			if (!test) {
                return std::nullopt;
			}

            return test;
		}

		void DLCloseLib()
		{
			if (dlclose(_handle) != 0) {
				std::cerr << dlerror() << std::endl;
			}
		}

	};
}

#endif
#endif