# TinyReflection

轻量型C++反射库



## How to use

0.注册类型

```
     registerClass<DerivObj>("DerivObj").
         registerConstructor<DerivObj, int, std::string>().
         registerProperty(&DerivObj::x, "x").
         registerProperty(&DerivObj::y, "y").
         registerMethod(&DerivObj::getId, "getId");
     // 显式注册类名
     // registerClass<DerivObj>("DerivObj");
     // 隐式注册类名
     // registerClass<DerivObj>("");
```

1.根据名称读写对象的属性

```
     DerivObj obj;
     MetaType metaType = MetaTypeOf(obj);
     Property prop = metaType.property(propertyName);
     U val = prop.getValue<U>(obj);
     prop.setValue(obj, newVal);
```

2.根据名称调用函数

```
     Method method = metaType.method(methodName);
     ReturnType ret = method.invoke<ReturnType>(obj, args...);
```

3.根据类名称创建实例

```
     Constructor ctor = metaType.constructor<int, std::string>();
     Variant v = ctor.invoke(222, std::string("deriv"));
     DerivObj& d = v.getValue<DerivObj&>();
```

4.迭代对象的所有属性、方法

```
     for (int i = 0; i < metaType.propertyCount(); i++) {
         auto property = metaType.property(i);
     }
     for (int i = 0; i < metaType.methodCount(); i++) {
         auto method = metaType.method(i);
     }
```

5.为类型，属性，函数，参数追加元数据
     TODO



## How to build

```
mkdir build
cd build
cmake ..
```
