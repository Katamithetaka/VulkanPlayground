#ifndef LIB_COMMON_HPP
#define LIB_COMMON_HPP

#include <string_view>
#include <optional>
#include <variant>
#include "macro_map.hpp"



#define GENERATE_ENUM_VARIANT(x) struct x {\
    [[nodiscard]] std::string_view get_value() const noexcept { return #x; }\
};


#define DEFINE_ENUM(name, ...) namespace name {\
 MAP(GENERATE_ENUM_VARIANT, __VA_ARGS__) \
 using Type = std::variant<__VA_ARGS__>; \
}\
using name##Type = name::Type;


// This defines a fake enum that can be used like this:
// - ExecutionResultVariantsType => a std::variant with all the enum options
// - ExecutionResultVariants => a namespace containing all variants
// - You can initiate a variant like this:
//      ExecutionResultVariantsType result = ExecutionResultVariants::Ok{};
DEFINE_ENUM(ExecutionResultVariants, Ok)



struct ExecutionResult {
public:
    ExecutionResultVariantsType result;
    std::optional<std::string_view> message;

    static ExecutionResult Ok() { return ExecutionResult(ExecutionResultVariants::Ok{}); }

private:

    ExecutionResult(ExecutionResultVariantsType type, std::optional<std::string_view> message = std::nullopt)
        : result{type}, message{message}    
    {

    }

};

// Important point about this:
// - Should be thread safe
// - Will always be sent in a std::shared_ptr to modules
struct ExecutionContext {};


#endif