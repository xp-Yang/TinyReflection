#include "Meta.hpp"

#include <functional>
#include <iostream>

using namespace Meta;

struct Struc {
	double s = 22.22;
};

class CusType {
public:
	float r = 11.11f;
};

enum E {
	V0,
	V1,
	V2,
	V3,
	V4,
};
class Obj {
public:
	float x = 1.1f;
	float y = 2.2f;
	float* z = new float(3.3f);
	int id = 99;
	std::string name = "reflectionTest";
	std::vector<int> ids = {1,3,5,7};
	std::vector<std::shared_ptr<Obj>> points;
	std::map<std::string, int> idMap = { {"a", 1}, {"b", 2} };
	E e = V3;
	CusType cus_type;

	int getId(int a, std::string b, const std::vector<float>& c) const { 
		std::cout << "invoke Obj::getId success" << std::endl;
		return id; 
	}

	virtual ~Obj() = default;
};

class DerivObj : public Obj {
public:
	int derivId = 109;
	int getDerivId(int a, std::string b, const std::vector<float>& c) const {
		std::cout << "invoke Deriv::getDerivId success" << std::endl;
		return derivId;
	}
};

std::unordered_map<std::string, std::function<void(Property, void*)>> m_property_reader;

void registerAll() {
	registerClass<Obj>().
		registerConstructor<Obj>().
		registerProperty(&Obj::x, "x").
		registerProperty(&Obj::y, "y").
		registerProperty(&Obj::z, "z").
		registerProperty(&Obj::id, "id").
		registerProperty(&Obj::name, "name").
		registerProperty(&Obj::ids, "ids").
		registerProperty(&Obj::points, "points").
		registerProperty(&Obj::idMap, "idMap").
		registerProperty(&Obj::e, "e").
		registerProperty(&Obj::cus_type, "cus_type").
		registerMethod(&Obj::getId, "getId");

	registerClass<DerivObj>().
		registerProperty(&DerivObj::x, "x").
		registerProperty(&DerivObj::y, "y").
		registerProperty(&DerivObj::z, "z").
		registerProperty(&DerivObj::id, "id").
		registerProperty(&DerivObj::name, "name").
		registerProperty(&DerivObj::ids, "ids").
		registerProperty(&DerivObj::points, "points").
		registerProperty(&DerivObj::idMap, "idMap").
		registerProperty(&DerivObj::e, "e").
		registerProperty(&DerivObj::cus_type, "cus_type").
		registerProperty(&DerivObj::derivId, "derivId").
		registerMethod(&DerivObj::getId, "getId").
		registerMethod(&DerivObj::getDerivId, "getDerivId");

	m_property_reader[traits::typeName<float>()] = [](Property prop, void* instance) {
		std::cout << (int)prop.type << " " << prop.type_name << " " << prop.name << " " << (prop.getValue<float>(instance)) << std::endl;
	};
	m_property_reader[traits::typeName<float*>()] = [](Property prop, void* instance) {
		std::cout << (int)prop.type << " " << prop.type_name << " " << prop.name << " " << *(prop.getValue<float*>(instance)) << std::endl;
	};
	m_property_reader[traits::typeName<int>()] = [](Property prop, void* instance) {
		std::cout << (int)prop.type << " " << prop.type_name << " " << prop.name << " " << (prop.getValue<int>(instance)) << std::endl;
	};
	m_property_reader[traits::typeName<std::string>()] = [](Property prop, void* instance) {
		std::cout << (int)prop.type << " " << prop.type_name << " " << prop.name << " " << (prop.getValue<std::string>(instance)) << std::endl;
	};
	m_property_reader[traits::typeName<std::vector<int>>()] = [](Property prop, void* instance) {
		std::cout << (int)prop.type << " " << prop.type_name << " " << prop.name << std::endl;
		int i = 0;
		const std::vector<int>& vec = prop.getValue<const std::vector<int>&>(instance);
		for (const auto& val : vec) {
			if (i == 0)
				std::cout << "    [";
			if (i == vec.size() - 1)
				std::cout << val << "]" << std::endl;
			else
				std::cout << val << ", ";
			i++;
		}
	};
	m_property_reader[traits::typeName<std::vector<std::shared_ptr<Obj>>>()] = [](Property prop, void* instance) {
		std::cout << (int)prop.type << " " << prop.type_name << " " << prop.name << " " /*<< (prop.getValue<std::vector<std::shared_ptr<Obj>>&>(&obj)) */ << std::endl;
	};
	m_property_reader[traits::typeName<std::map<std::string, int>>()] = [](Property prop, void* instance) {
		std::cout << (int)prop.type << " " << prop.type_name << " " << prop.name << std::endl;
		int i = 0;
		const std::map<std::string, int>& map = prop.getValue<const std::map<std::string, int>&>(instance);
		for (const auto& pair : map) {
			if (i == 0)
				std::cout << "    {";
			if (i == map.size() - 1)
				std::cout << "{" << pair.first << ", " << pair.second << "}" << "}" << std::endl;
			else
				std::cout << "{" << pair.first << ", " << pair.second << "}" << ", ";
			i++;
		}
	};
	m_property_reader[traits::typeName<E>()] = [](Property prop, void* instance) {
		std::cout << (int)prop.type << " " << prop.type_name << " " << prop.name << " " << (prop.getValue<int>(instance)) << std::endl;
	};
	m_property_reader[traits::typeName<CusType>()] = [](Property prop, void* instance) {

		std::cout << (int)prop.type << " " << prop.type_name << " " << prop.name << " " /*<< (prop.getValue<CusType>(instance))*/ << std::endl;
	};
	m_property_reader[traits::typeName<Obj>()] = [](Property prop, void* instance) {
		
	};
	m_property_reader[traits::typeName<DerivObj>()] = [](Property prop, void* instance) {
	
	};
}

void readPropertiesTest() {
	std::cout << "==========" << __FUNCTION__ << "==========" << std::endl;
	DerivObj obj;
	MetaType mt = MetaTypeOf(obj);
	for (auto& prop : mt.properties()) {
		if (m_property_reader.find(prop.type_name) != m_property_reader.end())
			m_property_reader[prop.type_name](prop, &obj);
		else
			assert(false);
	}
}

void writePropertiesTest() {
	std::cout << "==========" << __FUNCTION__ << "==========" << std::endl;
	Obj obj;
	MetaType mt = MetaTypeOf(obj);
	Property prop = mt.property("x");
	const float ff = 1.9f;
	prop.setValue(&obj, ff);
	std::cout << prop.type_name << " " << prop.name << " " << " " << (prop.getValue<float>(&obj)) << std::endl;
	prop.setValue(&obj, 2.9f);
	std::cout << prop.type_name << " " << prop.name << " " << " " << (prop.getValue<const float&>(&obj)) << std::endl;
}

void invokeMethodsTest() {
	std::cout << "==========" << __FUNCTION__ << "==========" << std::endl;
	DerivObj obj;
	MetaType mt = MetaTypeOf(obj);
	for (auto& method : mt.methods()) {
		std::cout << method.return_type_name << " " << method.method_name << " " << method.signature << std::endl;
		for (const auto& arg : method.arg_types) {
			std::cout << "    " << arg << std::endl;
		}
		auto ret = method.invoke<int>(&obj, 1, std::string("2"), std::vector<float>{ 3.0f });
	}
}

void createInstanceTest() {
	std::cout << "==========" << __FUNCTION__ << "==========" << std::endl;
	MetaType mt = MetaTypeOf<Obj>();
	Obj refl_obj = mt.createInstance<Obj>();
}

void polymorphismTest() {
	std::cout << "==========" << __FUNCTION__ << "==========" << std::endl;
	Obj& obj = DerivObj();
	auto n1 = traits::typeName(obj);
	auto n2 = traits::typeName<DerivObj>();
	bool equal = n1 == n2;

	MetaType mt = MetaTypeOf(obj);
	auto className = mt.className();
}

void serializeTest() {
	std::cout << "==========" << __FUNCTION__ << "==========" << std::endl;

}

int main() {
	registerAll();
	readPropertiesTest();
	writePropertiesTest();
	invokeMethodsTest();
	createInstanceTest();
	polymorphismTest();

	return 0;
}