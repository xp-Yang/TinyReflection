#ifndef Meta_hpp
#define Meta_hpp

#include <map>
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include <assert.h>
#include "traits.hpp"

namespace Meta {

//1.根据名称读写对象的属性
//     T obj;
//     MetaType metaType = MetaTypeOf(obj);
//     Property prop = metaType.property(propertyName);
//     prop.getValue<U>(&obj);
//     prop.setValue<U>(&obj, val);
//2.根据名称调用函数
//     Method method = metaType.method(methodName);
//     method.invoke<ReturnType>(&obj, args...);
//3.根据类名称创建实例
//     MetaType meta = MetaTypeOf<T>(obj);
//     Instance v = meta.createInstance();
//4.迭代对象的所有属性、方法
//     Instance v(obj)
//     MetaType type = v.metaType();
//     for (int i = 0; i < type.propertyCount(); i++) {
//         auto property = type.property(i);
//     }
//     for (int i = 0; i < type.methodCount(); i++) {
//         auto property = type.method(i);
//     }
//5.为类型，属性，函数，参数追加元数据
//     TODO


struct Property {
    enum Type : int {
        Unknown = 0,
        Fundamental = 1,
        Custom = 1 << 1,
        Pointer = 1 << 2,
        SequenceContainer = 1 << 3,
        AssociativeContainer = 1 << 4,
    };

    int type = Type::Unknown;
    size_t offset;
    std::string type_name;
    std::string name;

    template<typename T>
    bool isType() {
        return traits::typeName<T> == type_name;
    }

    template<typename T>
    T getValue(void* instance) {
        // T是指针类型，说明property的类型本身就是指针类型，走T为值类型的分支
        // if constexpr (std::is_pointer_v<T>) {
        //     return reinterpret_cast<T>(((char*)instance + offset));
        // }
        if constexpr (std::is_lvalue_reference_v<T>) {
            if constexpr (std::is_const_v<std::remove_reference_t<T>>) {
                // T是const左值引用
                using ValueType = std::remove_reference_t<T>;
                return *reinterpret_cast<const ValueType*>(((char*)instance + offset));
            }
            else {
                // T是非const左值引用
                using ValueType = std::remove_reference_t<T>;
                return *reinterpret_cast<ValueType*>(((char*)instance + offset));
            }
        }
        else if constexpr (std::is_rvalue_reference_v<T>) {
            // T是右值引用
            using ValueType = std::remove_reference_t<T>;
            return std::move(*reinterpret_cast<ValueType*>(((char*)instance + offset)));
        }
        else {
            // T是值类型
            return *reinterpret_cast<T*>(((char*)instance + offset));
        }
    }

    template<typename T>
    void setValue(void* instance, T&& value) {
        // T是指针类型，说明property的类型本身就是指针类型，当成左值指针或右值指针，走引用分支
        // if constexpr (std::is_pointer_v<std::remove_reference_t<T>>) {
        //     *getValue<T>(instance) = *value;
        // }
        if constexpr (std::is_lvalue_reference_v<T>) {
            if constexpr (std::is_const_v<std::remove_reference_t<T>>) {
                // const左值引用
                using ValueType = std::remove_const_t<std::remove_reference_t<T>>;
                getValue<ValueType&>(instance) = value;
            }
            else {
                // 非const左值引用
                getValue<T>(instance) = value;
            }
        }
        else if constexpr (std::is_rvalue_reference_v<T&&>) {
            // 右值引用
            getValue<T&>(instance) = value;
        }
    }
};

struct Constructor {
    std::vector<std::string> arg_types;

    template <typename T, typename ... Args>
    T invoke(Args&&... args) const {
        return T(args...);
    }
};

struct Method {
    struct Dumb {};
    void (Dumb::* func)();
    std::string return_type_name;
    std::string method_name;
    std::vector<std::string> arg_types;
    std::string signature;

    template <typename ReturnType, typename T, typename ... Args>
    ReturnType invoke(T* instance, Args&&... args) const {
        auto method = reinterpret_cast<ReturnType (T::*)(Args...)>(func);
        return (instance->*method)(std::forward<Args>(args)...);
    }
};

struct ClassInfo {
    std::string class_name;
    std::vector<Property> property_infos;
    std::vector<Method> method_infos;
    std::vector<Constructor> ctor_infos;

    template <class T, class PropertyType>
    inline ClassInfo& registerProperty(PropertyType T::* var_ptr, std::string_view property_name)
    {
        std::string class_name = traits::typeName<T>();
        std::string property_type_name_ = traits::typeName<PropertyType>();
        std::string property_name_ = std::string(property_name);
        size_t offset = reinterpret_cast<size_t>(&(reinterpret_cast<T const volatile*>(nullptr)->*var_ptr));
        using ValueType = std::remove_const_t<std::remove_pointer_t<std::remove_reference_t<PropertyType>>>;
        int prop_type = Property::Type::Unknown;
        if constexpr (traits::is_sequence_container<ValueType>::value)
            prop_type = Property::Type::SequenceContainer;
        else if constexpr (traits::is_associative_container<ValueType>::value)
            prop_type = Property::Type::AssociativeContainer;
        else if constexpr (std::is_fundamental_v<ValueType>)
            prop_type = Property::Type::Fundamental;
        else
            prop_type = Property::Type::Custom;
        if constexpr (std::is_pointer_v<PropertyType>)
            prop_type = prop_type | Property::Type::Pointer;
        Property property_info = { prop_type, offset, property_type_name_, property_name_ };
        property_infos.emplace_back(property_info);
        return *this;
    }

    template <typename Arg>
    inline std::string getArgTypeName() {
        return traits::typeName<Arg>();
    }

    template <class T, typename ... Args>
    inline ClassInfo& registerConstructor(Args... args)
    {
        auto arg_types = std::initializer_list<std::string>{ getArgTypeName<Args>() ... };
        Constructor ctor_info = { arg_types };
        ctor_infos.emplace_back(ctor_info);
        return *this;
    }

    template <class T, class ReturnType, typename ... Args>
    inline ClassInfo& registerMethod(ReturnType(T::* method_ptr)(Args...) const, std::string_view method_name)
    {
        std::string class_name = traits::typeName<T>();
        std::string return_type_name = traits::typeName<ReturnType>();
        std::string method_name_ = std::string(method_name);
        auto arg_types = std::initializer_list<std::string>{ getArgTypeName<Args>() ... };
        std::string method_signature = return_type_name + " " + method_name_ + "(";
        if (arg_types.size() == 0)
            method_signature += std::string(")");
        else {
            for (auto it = arg_types.begin(); it != arg_types.end(); it++) {
                method_signature += *it;
                method_signature += ((it + 1) == arg_types.end()) ? std::string(")") : std::string(", ");
            }
        }
        Method method_info = { reinterpret_cast<void (Method::Dumb::*)()>(method_ptr), return_type_name, method_name_, arg_types, method_signature };
        method_infos.emplace_back(method_info);
        return *this;
    }
};

inline std::unordered_map<std::string, ClassInfo> global_class_info;

template <class T>
inline ClassInfo& registerClass()
{
    std::string class_name = traits::typeName<T>();
    if (global_class_info.find(class_name) != global_class_info.end()) {
        assert(false);
    }
    global_class_info.insert({ class_name, ClassInfo{class_name, {}, {}} });
    return global_class_info.at(class_name);
}


class MetaType {
public:
    MetaType(std::string_view class_name)
        : m_class_info(global_class_info.at(std::string(class_name)))
    {}
    MetaType(const MetaType& rhs) = default;

    const std::string& className() const { return m_class_info.class_name; }

    template<typename T, typename ... Args>
    T createInstance(Args&& ... args) {
        const auto& arg_types = std::initializer_list<std::string>{ getArgTypeName<Args>() ... };
        for (const auto& ctor_info : m_class_info.ctor_infos) {
            if (ctor_info.arg_types.size() != arg_types.size())
                continue;
            auto it = ctor_info.arg_types.begin();
            auto initializer_it = arg_types.begin();
            for (; it != ctor_info.arg_types.end(); it++, initializer_it++) {
                if (*it != *initializer_it)
                    continue;
            }
            T instance = ctor_info.invoke<T>(args...);
            return instance;
        }
        return T(args...);
    }

    int propertyCount() const { return m_class_info.property_infos.size(); }
    Property property(int index) const {
        if (0 <= index && index < m_class_info.property_infos.size())
            return m_class_info.property_infos[index];
        return {};
    }
    Property property(const std::string& name) const {
        const auto it = std::find_if(m_class_info.property_infos.begin(), m_class_info.property_infos.end(), [&name](const auto& property_info) {
            return property_info.name == name;
            });
        if (it != m_class_info.property_infos.end())
            return *it;
        return {};
    }
    std::vector<Property> properties() const {
        std::vector<Property> res;
        res.reserve(m_class_info.property_infos.size());
        for (int i = 0; i < m_class_info.property_infos.size(); i++) {
            res.push_back(m_class_info.property_infos[i]);
        }
        return res;
    }

    int methodCount() const { return m_class_info.method_infos.size(); }
    Method method(int index) const {
        if (0 <= index && index < m_class_info.method_infos.size())
            return m_class_info.method_infos[index];
        return {};
    }
    Method method(const std::string& name) const {
        const auto it = std::find_if(m_class_info.method_infos.begin(), m_class_info.method_infos.end(), [&name](const auto& method_info) {
            return method_info.method_name == name;
            });
        if (it != m_class_info.method_infos.end())
            return *it;
        return {};
    }
    std::vector<Method> methods() const {
        std::vector<Method> res;
        res.reserve(m_class_info.method_infos.size());
        for (int i = 0; i < m_class_info.method_infos.size(); i++) {
            res.push_back(m_class_info.method_infos[i]);
        }
        return res;
    }

private:
    MetaType() = delete;

    ClassInfo m_class_info;
};

template <class T>
inline MetaType MetaTypeOf() { return MetaType(traits::typeName<std::remove_const_t<std::remove_pointer_t<std::remove_reference_t<T>>>>()); }

template <class T>
inline MetaType MetaTypeOf(T&& obj) { return MetaTypeOf<T>(); }


class Instance {
public:
    template <class T>
    Instance(T& obj, std::enable_if_t<!std::is_pointer_v<T>>* = nullptr) : Instance(MetaTypeOf<T>(), (void*)(&obj)) {}
    Instance(std::string_view type_name, void* instance) : Instance(MetaType(type_name), instance) {}
    Instance(const Instance& rhs) = default;

public:
    void* instance() { return m_instance; }
    const MetaType& metaType() const { return m_meta; }

    void* getPropertyValue(int index) const {
        auto& property = m_meta.property(index);
        return property.getValue<void*>(m_instance);
    }
    template<typename T>
    T getPropertyValue(int index) const {
        auto& property = m_meta.property(index);
        return property.getValue<T>(m_instance);
    }
    template<typename T>
    T getPropertyValue(const std::string& name) const {
        auto& property = m_meta.property(name);
        return property.getValue<T>(m_instance);
    }
    template<typename T>
    void setPropertyValue(const std::string& name, T&& value) const {
        auto& property = m_meta.property(name);
        property.setValue(m_instance, std::forward<T>(value));
    }

protected:
    Instance() = delete;
    Instance(const MetaType& meta, void* instance) : m_meta(meta), m_instance(instance) {}

    void* m_instance;
    MetaType m_meta;
};

}

#endif // !Meta_hpp
