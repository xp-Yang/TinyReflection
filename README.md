# TinyReflection
轻量型C++反射库

支持：

1.根据名称读写对象的属性

```
     T obj;
     MetaType metaType = MetaTypeOf(obj);
     
     Property prop = metaType.property(propertyName);
     prop.getValue<U>(&obj);
     prop.setValue<U>(&obj, val);
```

2.根据名称调用函数

```
     Method method = metaType.method(methodName);
     method.invoke<ReturnType>(&obj, args...);
```

3.根据类名称创建实例

```
     MetaType meta = MetaTypeOf<T>(obj);
     Instance v = meta.createInstance();
```

4.迭代对象的所有属性、方法

```
     Instance v(obj)
     MetaType type = v.metaType();
     for (int i = 0; i < type.propertyCount(); i++) {
         auto property = type.property(i);
     }
     for (int i = 0; i < type.methodCount(); i++) {
         auto property = type.method(i);
     }
```

5.为类型，属性，函数，参数追加元数据
     TODO
