#ifndef LIB_COMMON_HPP
#define LIB_COMMON_HPP

#include <string_view>
#include <optional>
#include <variant>

#ifdef _WIN32
#ifndef LIBRARY_EXPORT
#define LIBRARY_DLL __declspec(dllimport)
#else
#define LIBRARY_DLL __declspec(dllexport)

#endif
#endif

enum class ExecutionResultVariants {
    Ok
};



struct ExecutionResult {
public:
ExecutionResultVariants result;
    std::optional<std::string_view> message;

    static ExecutionResult Ok() { return ExecutionResult(ExecutionResultVariants::Ok); }

private:

    ExecutionResult(ExecutionResultVariants type, std::optional<std::string_view> message = std::nullopt)
        : result{type}, message{message}    
    {

    }

};

// Important point about this:
// - Should be thread safe
// - Will always be sent in a std::shared_ptr to modules
struct ExecutionContext {};


#endif