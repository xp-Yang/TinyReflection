#ifndef Traits_hpp
#define Traits_hpp

#include <vector>
#include <string>
#include <regex>
#include <map>
#include <unordered_map>
#include <set>
#include <type_traits>
#include <utility>

namespace Meta {

namespace traits {

//template<typename T>
//constexpr auto rawSignature() noexcept {
//#  if defined(__clang__)
//    return std::string_view{ __PRETTY_FUNCTION__ };
//#  elif defined(__GNUC__)
//    return std::string_view{ __PRETTY_FUNCTION__ };
//#  elif defined(_MSC_VER)
//    return std::string_view{ __FUNCSIG__ };
//#  endif
//}
//
//template<typename T>
//constexpr std::string rawTypeName2() noexcept {
//    constexpr std::string_view mark_str = "rawSignature";
//    std::string_view sig = rawSignature<T>();
//#if defined(__clang__)
//    sig = sig.substr(37);
//    std::string name = std::string(sig.substr(0, sig.size() - 1));
//#elif defined(__GNUC__)
//    sig = sig.substr(52);
//    std::string name = std::string(sig.substr(0, sig.size() - 1));
//#elif defined(_MSC_VER)
//    int start_index = sig.find(mark_str) + mark_str.size() + 1;
//    sig = sig.substr(start_index);
//    std::string name = std::string(sig.substr(0, sig.size() - 16));
//#endif
//    return name;
//}

// typeid能正确推导多态的类型（要有虚函数），和各种引用类型
// typeid(T).name()的结果：
// T -> T
// const T& -> T
// T& -> T
// T&& -> T
// T* -> T * __ptrXX
// const T* -> T const * __ptrXX
// T* const -> T * __ptrXX
// const T* const & -> T const * __ptrXX
template<typename T>
constexpr std::string rawTypeName() noexcept {
    return typeid(T).name();
}

template<typename T>
constexpr std::string rawTypeName(T&& obj) noexcept {
    return rawTypeName<T>();
}


// 基础容器特征检查
template <typename T, typename = void>
struct is_container : std::false_type {};

template <typename T>
struct is_container<T, std::void_t<
    typename T::value_type,        // 必须有 value_type
    typename T::iterator,          // 必须有迭代器类型
    typename T::const_iterator,    // 必须有常量迭代器类型
    decltype(std::declval<T>().begin()),  // 必须有 begin() 方法
    decltype(std::declval<T>().end()),    // 必须有 end() 方法
    decltype(std::declval<T>().size())    // 应该有 size() 方法
    >> : std::true_type{};

// 检查是否是关联容器
template <typename T, typename = void>
struct is_associative_container : std::false_type {};

template <typename T>
struct is_associative_container<T, std::void_t<
    typename T::key_type,
    typename T::key_compare,
    decltype(std::declval<T>().find(std::declval<typename T::key_type>()))
    >> : std::true_type {};

// 检查是否是序列容器
template <typename T>
struct is_sequence_container : std::conjunction<
    is_container<T>,
    std::negation<is_associative_container<T>>,
    std::negation<std::is_same<T, std::string>>,
    std::negation<std::is_same<T, std::wstring>>,
    std::negation<std::is_same<T, std::u16string>>,
    std::negation<std::is_same<T, std::u32string>>
> {};

template <typename T>
constexpr bool is_container_v = is_container<T>::value;
template <typename T>
constexpr bool is_associative_container_v = is_associative_container<T>::value;
template <typename T>
constexpr bool is_sequence_container_v = is_sequence_container<T>::value;
template <typename T>
constexpr bool is_fundamental_v =
    std::is_fundamental_v<T> ||
    std::is_same_v<T, std::string> ||
    std::is_same_v<T, std::wstring> ||
    std::is_same_v<T, std::u16string> ||
    std::is_same_v<T, std::u32string>;

}

}

#endif // !Meta_hpp
