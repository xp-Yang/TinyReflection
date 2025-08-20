#include "Meta.hpp"

#include <iostream>

using namespace Meta;

class Obj {
public:
	float x = 1.1f;
	float y = 2.2f;
	float* z = new float(3.3f);
	int id = 99;
	std::string name = "reflectionTest";
	std::vector<int> ids = {1,3,5,7};
	std::map<std::string, int> idMap = { {"a", 1}, {"b", 2} };

	int getId(int a, std::string b, const std::vector<float>& c) const { 
		std::cout << "invoke_success" << std::endl;
		return id; 
	}
};

class DerivObj : public Obj {
public:
	int derivId = 109;
	int getDerivId(int a, std::string b, const std::vector<float>& c) const {
		std::cout << "invoke_deriv_success" << std::endl;
		return derivId;
	}
};

void registerAll() {
	registerClass<Obj>().
		registerConstructor<Obj>().
		registerProperty(&Obj::x, "x").
		registerProperty(&Obj::y, "y").
		registerProperty(&Obj::z, "z").
		registerProperty(&Obj::id, "id").
		registerProperty(&Obj::name, "name").
		registerProperty(&Obj::ids, "ids").
		registerProperty(&Obj::idMap, "idMap").
		registerMethod(&Obj::getId, "getId");

	registerClass<DerivObj>().
		registerProperty(&DerivObj::x, "x").
		registerProperty(&DerivObj::y, "y").
		registerProperty(&DerivObj::z, "z").
		registerProperty(&DerivObj::id, "id").
		registerProperty(&DerivObj::name, "name").
		registerProperty(&DerivObj::ids, "ids").
		registerProperty(&DerivObj::idMap, "idMap").
		registerProperty(&DerivObj::derivId, "derivId").
		registerMethod(&DerivObj::getId, "getId").
		registerMethod(&DerivObj::getDerivId, "getDerivId");
}

void readPropertiesTest() {
	std::cout << "==========" << __FUNCTION__ << "==========" << std::endl;
	DerivObj obj;
	MetaType mt = MetaTypeOf(obj);
	for (auto& prop : mt.properties()) {
		if (prop.name == "x" || prop.name == "y")
			std::cout << (int)prop.type << " " << prop.type_name << " " << prop.name << " " << (prop.getValue<float&>(&obj)) << std::endl;
		if (prop.name == "z")
			std::cout << (int)prop.type << " " << prop.type_name << " " << prop.name << " " << *(prop.getValue<float*>(&obj)) << std::endl;
		if (prop.name == "id")
			std::cout << (int)prop.type << " " << prop.type_name << " " << prop.name << " " << (prop.getValue<int&>(&obj)) << std::endl;
		if (prop.name == "name")
			std::cout << (int)prop.type << " " << prop.type_name << " " << prop.name << " " << (prop.getValue<std::string&>(&obj)) << std::endl;
		if (prop.name == "ids") {
			std::vector<int>& vals = (prop.getValue<std::vector<int>&>(&obj));
			std::cout << (int)prop.type << " " << prop.type_name << " " << prop.name << std::endl;
			for (const auto& val : vals)
				std::cout << "    " << val << std::endl;
		}
		if (prop.name == "idMap") {
			std::map<std::string, int>& vals = (prop.getValue<std::map<std::string, int>&>(&obj));
			std::cout << (int)prop.type << " " << prop.type_name << " " << prop.name << std::endl;
			for (const auto& pair : vals) {
				std::cout << "    " << pair.first << " " << pair.second << std::endl;
			}
		}
	}
	for (auto& method : mt.methods()) {
		std::cout << method.return_type_name << " " << method.method_name << " " << method.signature << std::endl;
		auto ret = method.invoke<int>(&obj, 1, std::string("2"), std::vector<float>{ 3.0f });
		for (const auto& arg : method.arg_types) {
			std::cout << "    " << arg << std::endl;
		}
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

void createInstanceTest() {
	std::cout << "==========" << __FUNCTION__ << "==========" << std::endl;
	MetaType mt = MetaTypeOf<Obj>();
	Obj refl_obj = mt.createInstance<Obj>();
}

int main() {
	registerAll();
	readPropertiesTest();
	writePropertiesTest();
	createInstanceTest();
	return 0;
}