


#include "common.hpp"
#include "shared.hpp"
#include <memory>
#include <iostream>

MODULES_DLL ExecutionResult run(std::shared_ptr<ExecutionContext> context, int a) {
    std::cout << "Goodbye Pizza " <<  a << std::endl;
    
    return ExecutionResult::Ok();
}