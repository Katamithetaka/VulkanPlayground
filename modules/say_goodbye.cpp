
#ifdef _WIN32
#define MODULES_DLL __declspec(dllexport)
#else
#define MODULES_DLL
#endif


#include "common.hpp"
#include "shared.hpp"
#include <memory>
#include <iostream>

extern "C"
{
	MODULES_DLL ExecutionResult run(std::shared_ptr<ExecutionContext> context, int a)
	{
		std::cout << "Goodbye Pizza " << a << std::endl;

		return ExecutionResult::Ok();
	}
}