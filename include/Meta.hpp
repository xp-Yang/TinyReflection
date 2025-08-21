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
//     TODO
//     MetaType meta = MetaTypeOf<T>(obj);
//     Variant v = meta.createInstance();
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
        Pointer = 1,
        Fundamental = 1 << 1,
        Custom = 1 << 2,
        Enum = 1 << 3,
        SequenceContainer = 1 << 4,
        AssociativeContainer = 1 << 5,
    };

    int type = Type::Unknown;
    size_t offset;
    size_t size;
    std::string type_name;
    std::string name;

    template<typename T>
    bool isType() const {
        return MetaTypeOf<T>().className() == type_name;
    }

    void* getValue(void* instance) const {
        return reinterpret_cast<void*>(((char*)instance + offset));
    }

    template<typename T>
    T getValue(void* instance) const {
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
    void setValue(void* instance, T&& value) const {
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
    std::string raw_class_name;
    std::vector<Property> property_infos;
    std::vector<Method> method_infos;
    std::vector<Constructor> ctor_infos;

    template <class T, class PropertyType>
    inline ClassInfo& registerProperty(PropertyType T::* var_ptr, std::string_view property_name)
    {
        std::string property_type_name_ = MetaTypeOf<PropertyType>().className();
        std::string property_name_ = std::string(property_name);
        size_t offset = reinterpret_cast<size_t>(&(reinterpret_cast<T const volatile*>(nullptr)->*var_ptr));
        using ValueType = std::remove_const_t<std::remove_pointer_t<std::remove_reference_t<PropertyType>>>;
        int prop_type = Property::Type::Unknown;
        if constexpr (traits::is_fundamental_v<ValueType>)
            prop_type = Property::Type::Fundamental;
        else if constexpr (traits::is_sequence_container_v<ValueType>)
            prop_type = Property::Type::SequenceContainer;
        else if constexpr (traits::is_associative_container_v<ValueType>)
            prop_type = Property::Type::AssociativeContainer;
        else if constexpr (std::is_class_v<ValueType>)
            prop_type = Property::Type::Custom;
        else if constexpr (std::is_enum_v<ValueType>)
            prop_type = Property::Type::Enum;
        if constexpr (std::is_pointer_v<PropertyType>) {
            prop_type = prop_type | Property::Type::Pointer;
        }
        Property property_info = { prop_type, offset, sizeof(ValueType), property_type_name_, property_name_ };
        property_infos.emplace_back(property_info);
        return *this;
    }

    template <typename Arg>
    inline std::string getArgTypeName() {
        return MetaTypeOf<Arg>().className();
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
    inline ClassInfo& registerMethod(ReturnType(T::* method_ptr)(Args...), std::string_view method_name)
    {
        std::string return_type_name = MetaTypeOf<ReturnType>().className();
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
inline ClassInfo& registerClass(std::string class_name = {})
{
    std::string raw_class_name = traits::typeName<T>();
    if (class_name.empty())
        class_name = raw_class_name;
    if (global_class_info.find(raw_class_name) != global_class_info.end()) {
        assert(false);
    }
    global_class_info.insert({ raw_class_name, ClassInfo{class_name, raw_class_name, {}, {}} });
    return global_class_info.at(raw_class_name);
}


class MetaType {
public:
    MetaType() {}
    MetaType(std::string_view raw_class_name)
    {
        std::string rawClassName = std::string(raw_class_name);
        if (global_class_info.find(rawClassName) == global_class_info.end()) {
            global_class_info[rawClassName] = ClassInfo{ rawClassName, rawClassName, {}, {} };
        }
        m_class_info = global_class_info.at(rawClassName);
    }
    MetaType(const MetaType& rhs) = default;

    const std::string& className() const { return m_class_info.class_name; }
    const std::string& rawClassName() const { return m_class_info.raw_class_name; }

    template<typename T, typename ... Args>
    T createInstance(Args&& ... args) const {
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
        return m_class_info.property_infos;
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
        return m_class_info.method_infos;
    }

private:
    ClassInfo m_class_info;
};

template <class T>
inline MetaType MetaTypeOf() { return MetaType(traits::typeName<T>()); }

template <class T>
inline MetaType MetaTypeOf(T&& obj) { return MetaType(traits::typeName(std::forward<T>(obj))); }


class Instance {
public:
    template <typename T>
    Instance(T& obj) : Instance(MetaTypeOf<T>(), (void*)(&obj)) {}
    template <typename T>
    Instance(T* obj) : Instance(MetaTypeOf<T>(), obj) {}
    Instance(std::string_view type_name, void* instance) : Instance(MetaType(type_name), instance) {}
    Instance(const Instance& rhs) = default;

public:
    void* instance() const { return m_instance; }
    const MetaType& metaType() const { return m_meta; }
    std::string className() const { return m_meta.className(); }

    template <typename T>
    bool isType() const {
        return m_meta.className() == MetaTypeOf<T>().className();
    }

    template<typename T>
    T getValue() const {
        using ValueType = std::remove_const_t<std::remove_reference_t<T>>;
        if (!isType<ValueType>())
            throw std::exception();
        if constexpr (std::is_lvalue_reference_v<T>) {
            if constexpr (std::is_const_v<std::remove_reference_t<T>>) {
                // T是const左值引用
                return *reinterpret_cast<const ValueType*>(m_instance);
            }
            else {
                // T是非const左值引用
                return *reinterpret_cast<ValueType*>(m_instance);
            }
        }
        else if constexpr (std::is_rvalue_reference_v<T>) {
            // T是右值引用
            return std::move(*reinterpret_cast<ValueType*>(m_instance));
        }
        else {
            // T是值类型
            return *reinterpret_cast<T*>(m_instance);
        }
    }

    void* getPropertyValue(int index) const {
        auto& property = m_meta.property(index);
        return property.getValue(m_instance);
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

class Variant {
public:
    Variant() = default;
    template <class T>
    Variant(T&& obj) {
        m_meta = MetaTypeOf<T>();
        m_data_size = sizeof(T);
        using ValueType = std::remove_const_t<std::remove_reference_t<T>>;
        if constexpr (std::is_pointer_v<ValueType>)
            m_data = obj;
        else {
            // 调用T的拷贝or移动构造
            m_data = new ValueType(std::forward<T>(obj));
        }
    }
    
public:
    bool isValid() const { return m_data != nullptr; }

    template <typename T>
    bool isType() const {
        return m_meta.className() == MetaTypeOf<T>().className();
    }

    MetaType metaType() const { return m_meta; }

    std::string className() const { return m_meta.className(); }

    void clear() {
        m_data = nullptr;
        m_data_size = 0;
        m_meta = MetaType();
    }

    template<typename T>
    T getValue() const {
        using ValueType = std::remove_const_t<std::remove_reference_t<T>>;
        if (!isType<ValueType>())
            throw std::exception();
        if constexpr (std::is_lvalue_reference_v<T>) {
            if constexpr (std::is_const_v<std::remove_reference_t<T>>) {
                // T是const左值引用
                return *reinterpret_cast<const ValueType*>(m_data);
            }
            else {
                // T是非const左值引用
                return *reinterpret_cast<ValueType*>(m_data);
            }
        }
        else if constexpr (std::is_rvalue_reference_v<T>) {
            // T是右值引用
            return std::move(*reinterpret_cast<ValueType*>(m_data));
        }
        else {
            // T是值类型
            return *reinterpret_cast<T*>(m_data);
        }
    }

    template<typename T>
    void setValue(T&& value) const {
        using ValueType = std::remove_const_t<std::remove_reference_t<T>>;
        if (!isType<ValueType>())
            throw std::exception();
        *reinterpret_cast<T*>(m_data) = value;
    }

private:
    void* m_data = nullptr;
    size_t m_data_size = 0;
    MetaType m_meta;
};

}

#endif // !Meta_hpp
