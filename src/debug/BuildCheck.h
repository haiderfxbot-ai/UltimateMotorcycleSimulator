#ifndef BUILD_CHECK_H
#define BUILD_CHECK_H

#include <cstddef>
#include <type_traits>

// Compile-time struct field check: verifies a type has a member
// Usage: BUILD_CHECK_HAS_MEMBER(MyStruct, myField)
template <typename T, typename = void>
struct _has_member : std::false_type {};
#define BUILD_CHECK_HAS_MEMBER(Type, Member)                           \
    template <typename T>                                              \
    struct _has_member<Type, decltype((void)&Type::Member, void())>    \
        : std::true_type {};                                           \
    static_assert(_has_member<Type>::value,                            \
                  "BUILD CHECK FAILED: " #Type " is missing member '" #Member "'")

// Compile-time size assertion
#define BUILD_CHECK_SIZE(Type, ExpectedSize)                           \
    static_assert(sizeof(Type) == (ExpectedSize),                      \
                  "BUILD CHECK FAILED: sizeof(" #Type ") != " #ExpectedSize)

// Compile-time alignment assertion
#define BUILD_CHECK_ALIGNMENT(Type, ExpectedAlign)                     \
    static_assert(alignof(Type) == (ExpectedAlign),                    \
                  "BUILD CHECK FAILED: alignof(" #Type ") != " #ExpectedAlign)

// Ensure an expression compiles and returns true
#define BUILD_CHECK_EXPR(Expr, Message)                                \
    static_assert((Expr), "BUILD CHECK FAILED: " Message)

// Forward declaration completeness check (can't be a compile-time check,
// but helper for source files to include proper headers)
#define BUILD_REQUIRE_HEADER(HeaderMsg)                                \
    static_assert(true, "Include check: " HeaderMsg)

// Validate that a pointer is not null at runtime
#define RUNTIME_CHECK_NOT_NULL(ptr, module, desc)                      \
    do {                                                                \
        if (!(ptr)) {                                                   \
            debug::ErrorLogger::instance().logCritical(                 \
                module, __FILE__, __func__, __LINE__,                   \
                desc, "Null pointer dereference",                       \
                "Add null check before use or ensure initialization");  \
        }                                                               \
    } while(0)

// Validate a condition at runtime
#define RUNTIME_CHECK(cond, module, desc, cause, fix)                  \
    do {                                                                \
        if (!(cond)) {                                                  \
            debug::ErrorLogger::instance().logError(                    \
                module, __FILE__, __func__, __LINE__,                   \
                desc, cause, fix);                                      \
        }                                                               \
    } while(0)

#endif
