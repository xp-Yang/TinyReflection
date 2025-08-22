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

//0.注册类型
//     registerClass<Obj>("Obj").
//         registerConstructor<Obj>().
//         registerProperty(&Obj::x, "x").
//         registerProperty(&Obj::y, "y").
//         registerMethod(&Obj::getId, "getId");
//     显式注册类名：registerClass<Obj>("Obj");
//     隐式注册类名：registerClass<Obj>("");
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

class Instance;

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
    bool isType() const { return type_name == MetaTypeOf<T>().typeName(); }

    Instance getValue(const Instance& instance) const;

    template<typename T>
    T getValue(const Instance& instance) const { return getValue(instance).getValue<T>(); }

    template<typename T>
    void setValue(const Instance& instance, T&& value) const { getValue(instance).setValue(value); }
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
    inline ClassInfo& registerProperty(PropertyType T::* var_ptr, std::string property_name)
    {
        std::string property_type_name_ = MetaTypeOf<PropertyType>().typeName();
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
        Property property_info = { prop_type, offset, sizeof(ValueType), property_type_name_, property_name };
        property_infos.emplace_back(property_info);
        return *this;
    }

    template <typename Arg>
    inline std::string getArgTypeName() {
        return MetaTypeOf<Arg>().typeName();
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
    inline ClassInfo& registerMethod(ReturnType(T::* method_ptr)(Args...), std::string method_name)
    {
        std::string return_type_name = MetaTypeOf<ReturnType>().typeName();
        auto arg_types = std::initializer_list<std::string>{ getArgTypeName<Args>() ... };
        std::string method_signature = return_type_name + " " + method_name + "(";
        if (arg_types.size() == 0)
            method_signature += std::string(")");
        else {
            for (auto it = arg_types.begin(); it != arg_types.end(); it++) {
                method_signature += *it;
                method_signature += ((it + 1) == arg_types.end()) ? std::string(")") : std::string(", ");
            }
        }
        Method method_info = { reinterpret_cast<void (Method::Dumb::*)()>(method_ptr), return_type_name, method_name, arg_types, method_signature };
        method_infos.emplace_back(method_info);
        return *this;
    }
};

inline std::unordered_map<std::string, std::string> type_name_map;
inline std::unordered_map<std::string, ClassInfo> global_class_info;

inline ClassInfo& registerClass(std::string raw_class_name, std::string class_name) {
    if (type_name_map.find(raw_class_name) == type_name_map.end()) {
        if (class_name.empty()) {
            class_name = raw_class_name;
            std::regex pattern1(" \\* __ptr32");
            std::regex pattern2(" \\* __ptr64");
            class_name = std::regex_replace(class_name, pattern1, "*");
            class_name = std::regex_replace(class_name, pattern2, "*");
        }
        type_name_map[raw_class_name] = class_name;
        global_class_info.insert({ class_name, ClassInfo{class_name, {}, {}} });
    }
    else {
        if (class_name.empty()) {
            class_name = type_name_map.at(raw_class_name);
        }
        else {
            std::string origin_class_name = type_name_map.at(raw_class_name);
            if (origin_class_name != class_name) {
                ClassInfo origin_class_info = global_class_info.at(origin_class_name);
                type_name_map[origin_class_name] = class_name;
                global_class_info.erase(origin_class_name);
                global_class_info.insert({ class_name, origin_class_info });
            }
        }
    }
    return global_class_info.at(class_name);
}

template <class T>
inline ClassInfo& registerClass(std::string class_name = {})
{
    std::string raw_class_name = traits::rawTypeName<T>();
    return registerClass(raw_class_name, class_name);
}


class MetaType {
public:
    MetaType() = default;
    MetaType(std::string type_name) : m_class_info(global_class_info.at(type_name)) {}
    MetaType(const MetaType& rhs) = default;

    const std::string& typeName() const { return m_class_info.class_name; }
    template <typename T>
    bool isType() const { return typeName() == MetaTypeOf<T>().typeName(); }

    int propertyCount() const { return m_class_info.property_infos.size(); }
    Property property(int index) const { return m_class_info.property_infos[index]; }
    Property property(const std::string& name) const {
        const auto it = std::find_if(m_class_info.property_infos.begin(), m_class_info.property_infos.end(), [&name](const auto& property_info) {
            return property_info.name == name;
            });
        if (it != m_class_info.property_infos.end())
            return *it;
        return {};
    }
    std::vector<Property> properties() const { return m_class_info.property_infos; }

    int constructorCount() const { return m_class_info.ctor_infos.size(); }
    Constructor constructor(int index) const { return m_class_info.ctor_infos[index]; }
    template<typename ... Args>
    Constructor constructor(Args&& ... args) const {
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
            return ctor_info;
        }
        return {};
    }
    const std::vector<Constructor>& constructors() const { return m_class_info.ctor_infos; }

    int methodCount() const { return m_class_info.method_infos.size(); }
    Method method(int index) const { return m_class_info.method_infos[index]; }
    Method method(const std::string& name) const {
        const auto it = std::find_if(m_class_info.method_infos.begin(), m_class_info.method_infos.end(), [&name](const auto& method_info) {
            return method_info.method_name == name;
            });
        if (it != m_class_info.method_infos.end())
            return *it;
        return {};
    }
    const std::vector<Method>& methods() const { return m_class_info.method_infos; }

private:
    ClassInfo m_class_info;
};

template <class T>
inline MetaType MetaTypeOf() { return MetaType(registerClass<T>().class_name); }

template <class T>
inline MetaType MetaTypeOf(T&& obj) { 
    std::string raw_type_name = traits::rawTypeName(std::forward<T>(obj));
    return MetaType(registerClass(raw_type_name, "").class_name);
}


// Instance引用原始对象，不管理其生命周期
class Instance {
public:
    template <typename T>
    Instance(T&& obj)
        : m_meta(MetaTypeOf(std::forward<T>(obj)))
        , m_data((void*)(&obj))
    {}
    Instance(const Instance& rhs) = default;

public:
    void* instance() const { return m_data; }
    std::string typeName() const { return m_meta.typeName(); }
    MetaType metaType() const { return m_meta; }

public:
    template <typename T>
    bool isType() const { return m_meta.isType<T>(); }

    template<typename T>
    T getValue() const {
        // T是指针类型，说明数据本身就是指针类型，走T为值类型的分支
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
        if constexpr (std::is_lvalue_reference_v<T>) {
            if constexpr (std::is_const_v<std::remove_reference_t<T>>) {
                // const左值引用
                getValue<ValueType&>() = value;
            }
            else {
                // 非const左值引用
                getValue<T>() = value;
            }
        }
        else if constexpr (std::is_rvalue_reference_v<T&&>) {
            // 右值引用
            getValue<T&>() = value;
        }
    }

private:
    Instance() = delete;
    friend class Property;
    friend class Variant;
    Instance(std::string type_name, void* obj_ptr)
        : m_meta(MetaType(type_name))
        , m_data(obj_ptr)
    {}

    void* m_data;
    MetaType m_meta;
};

// Variant拷贝原对象，拥有拷贝的新对象的所有权
class Variant {
public:
    Variant() = default;
    Variant(const Variant& rhs) = default;
    template <class T>
    Variant(T&& obj) {
        m_meta = MetaTypeOf(std::forward<T>(obj));
        m_data_size = sizeof(T);
        using ValueType = std::remove_const_t<std::remove_reference_t<T>>;
        // 调用T的拷贝or移动构造
        m_data = new ValueType(std::forward<T>(obj));
    }
    
public:
    bool isValid() const { return m_data != nullptr; }
    MetaType metaType() const { return m_meta; }
    std::string typeName() const { return m_meta.typeName(); }
    void* rawData() const { return m_data; }
    void clear() {
        // TODO 释放m_data
        m_data = nullptr;
        m_data_size = 0;
        m_meta = MetaType();
    }

public:
    template <typename T>
    bool isType() const { return m_meta.isType<T>(); }

    template<typename T>
    T getValue() const { return Instance(typeName(), m_data).getValue<T>(); }

    template<typename T>
    void setValue(T&& value) {
        // TODO 释放m_data
        *this = Variant(value);
    }

private:
    void* m_data = nullptr;
    size_t m_data_size = 0;
    MetaType m_meta;
};

Instance Property::getValue(const Instance& instance) const {
    return Instance(type_name, ((char*)instance.instance() + offset));
}

}

#endif // !Meta_hpp
