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

enum class E {
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
	E e = E::V3;
	CusType cus_type;

	int getId(int a, std::string b, const std::vector<float>& c) { 
		std::cout << "invoke Obj::getId success" << std::endl;
		return id; 
	}

	virtual ~Obj() = default;
};

class DerivObj : public Obj {
public:
	int derivId = 109;
	int getDerivId(int a, std::string b, const std::vector<float>& c) {
		std::cout << "invoke Deriv::getDerivId success" << std::endl;
		return derivId;
	}
	void dumb() {
		std::cout << "invoke Deriv::dumb success" << std::endl;
	}
};

std::unordered_map<std::string, std::function<void(Property, void*)>> m_property_reader;
std::unordered_map<std::string, std::function<void(std::string, const Instance&)>> m_inst_reader;

void registerAll() {
	registerClass<std::vector<int>>("std::vector<int>");
	registerClass<std::vector<float>>("std::vector<float>");
	registerClass<std::string>("std::string");
	registerClass<std::map<std::string, int>>("std::map<std::string, int>");
	registerClass<std::vector<std::shared_ptr<Obj>>>("std::vector<std::shared_ptr<Obj>>");
	registerClass<E>("E");

	registerClass<CusType>("CusType")
		.registerProperty(&CusType::r, "r");

	registerClass<Obj>("Obj").
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

	registerClass<DerivObj>("DerivObj").
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
		registerMethod(&DerivObj::getDerivId, "getDerivId").
		registerMethod(&DerivObj::dumb, "dumb");

	m_property_reader[MetaTypeOf<float>().typeName()] = [](Property prop, void* instance) {
		std::cout << (int)prop.type << " " << prop.type_name << " " << prop.name << " " << (prop.getValue<float>(instance)) << std::endl;
	};
	m_property_reader[MetaTypeOf<float*>().typeName()] = [](Property prop, void* instance) {
		std::cout << (int)prop.type << " " << prop.type_name << " " << prop.name << " " << *(prop.getValue<float*>(instance)) << std::endl;
	};
	m_property_reader[MetaTypeOf<int>().typeName()] = [](Property prop, void* instance) {
		std::cout << (int)prop.type << " " << prop.type_name << " " << prop.name << " " << (prop.getValue<int>(instance)) << std::endl;
	};
	m_property_reader[MetaTypeOf<std::string>().typeName()] = [](Property prop, void* instance) {
		std::cout << (int)prop.type << " " << prop.type_name << " " << prop.name << " " << (prop.getValue<std::string>(instance)) << std::endl;
	};
	m_property_reader[MetaTypeOf<std::vector<int>>().typeName()] = [](Property prop, void* instance) {
		std::cout << (int)prop.type << " " << prop.type_name << " " << prop.name;
		int i = 0;
		const std::vector<int>& vec = prop.getValue<const std::vector<int>&>(instance);
		for (const auto& val : vec) {
			if (i == 0)
				std::cout << " [";
			if (i == vec.size() - 1)
				std::cout << val << "]";
			else
				std::cout << val << ", ";
			i++;
		}
		std::cout << std::endl;
	};
	m_property_reader[MetaTypeOf<std::vector<std::shared_ptr<Obj>>>().typeName()] = [](Property prop, void* instance) {
		std::cout << (int)prop.type << " " << prop.type_name << " " << prop.name;
		int i = 0;
		const std::vector<std::shared_ptr<Obj>>& vec = (prop.getValue<const std::vector<std::shared_ptr<Obj>>&>(instance));
		for (const auto& val : vec) {
			if (i == 0)
				std::cout << " [";
			if (i == vec.size() - 1)
				std::cout << val.get() << "]";
			else
				std::cout << val.get() << ", ";
			i++;
		}
		std::cout << std::endl;
	};
	m_property_reader[MetaTypeOf<std::map<std::string, int>>().typeName()] = [](Property prop, void* instance) {
		std::cout << (int)prop.type << " " << prop.type_name << " " << prop.name;
		int i = 0;
		const std::map<std::string, int>& map = prop.getValue<const std::map<std::string, int>&>(instance);
		for (const auto& pair : map) {
			if (i == 0)
				std::cout << " {";
			if (i == map.size() - 1)
				std::cout << "{" << pair.first << ", " << pair.second << "}" << "}";
			else
				std::cout << "{" << pair.first << ", " << pair.second << "}" << ", ";
			i++;
		}
		std::cout << std::endl;
	};
	m_property_reader[MetaTypeOf<E>().typeName()] = [](Property prop, void* instance) {
		std::cout << (int)prop.type << " " << prop.type_name << " " << prop.name << " " << (prop.getValue<int>(instance)) << std::endl;
	};
	m_property_reader[MetaTypeOf<CusType>().typeName()] = [](Property prop, void* instance) {
		std::cout << (int)prop.type << " " << prop.type_name << " " << prop.name << /*" " << (prop.getValue<CusType>(instance)) <<*/ std::endl;
		MetaType mt = MetaTypeOf<CusType>();
		for (auto& p : mt.properties()) {
			if (m_property_reader.find(p.type_name) != m_property_reader.end()) {
				m_property_reader[p.type_name](p, prop.getValue(instance));
			}
		}
	};

	m_inst_reader[MetaTypeOf<float>().typeName()] = [](std::string inst_name, const Instance& inst) {
		std::cout << inst.typeName() << " " << inst_name << " " << (inst.getValue<float>()) << std::endl;
	};
	m_inst_reader[MetaTypeOf<float*>().typeName()] = [](std::string inst_name, const Instance& inst) {
		std::cout << inst.typeName() << " " << inst_name << " " << *(inst.getValue<float*>()) << std::endl;
	};
	m_inst_reader[MetaTypeOf<int>().typeName()] = [](std::string inst_name, const Instance& inst) {
		std::cout << inst.typeName() << " " << inst_name << " " << (inst.getValue<int>()) << std::endl;
	};
	m_inst_reader[MetaTypeOf<std::string>().typeName()] = [](std::string inst_name, const Instance& inst) {
		std::cout << inst.typeName() << " " << inst_name << " " << (inst.getValue<std::string>()) << std::endl;
	};	
	m_inst_reader[MetaTypeOf<std::vector<int>>().typeName()] = [](std::string inst_name, const Instance& inst) {
		std::cout << inst.typeName() << " " << inst_name;
		int i = 0;
		const std::vector<int>& vec = inst.getValue<const std::vector<int>&>();
		for (const auto& val : vec) {
			if (i == 0)
				std::cout << " [";
			if (i == vec.size() - 1)
				std::cout << val << "]";
			else
				std::cout << val << ", ";
			i++;
		}
		std::cout << std::endl;
	};
	m_inst_reader[MetaTypeOf<std::vector<std::shared_ptr<Obj>>>().typeName()] = [](std::string inst_name, const Instance& inst) {
		std::cout << inst.typeName() << " " << inst_name;
		int i = 0;
		const std::vector<std::shared_ptr<Obj>>& vec = (inst.getValue<const std::vector<std::shared_ptr<Obj>>&>());
		for (const auto& val : vec) {
			if (i == 0)
				std::cout << " [";
			if (i == vec.size() - 1)
				std::cout << val.get() << "]";
			else
				std::cout << val.get() << ", ";
			i++;
		}
		std::cout << std::endl;
	};
	m_inst_reader[MetaTypeOf<std::map<std::string, int>>().typeName()] = [](std::string inst_name, const Instance& inst) {
		std::cout << inst.typeName() << " " << inst_name;
		int i = 0;
		const std::map<std::string, int>& map = inst.getValue<const std::map<std::string, int>&>();
		for (const auto& pair : map) {
			if (i == 0)
				std::cout << " {";
			if (i == map.size() - 1)
				std::cout << "{" << pair.first << ", " << pair.second << "}" << "}";
			else
				std::cout << "{" << pair.first << ", " << pair.second << "}" << ", ";
			i++;
		}
		std::cout << std::endl;
	};
	m_inst_reader[MetaTypeOf<E>().typeName()] = [](std::string inst_name, const Instance& inst) {
		//std::cout << inst.typeName() << " " << inst_name << " " << inst.getValue<int>() << std::endl;
	};
	m_inst_reader[MetaTypeOf<CusType>().typeName()] = [](std::string inst_name, const Instance& inst) {
		std::cout << inst.typeName() << " " << inst_name << " " << &inst.getValue<CusType>() << std::endl;
		MetaType mt = MetaTypeOf<CusType>();
		for (auto& p : mt.properties()) {
			if (m_inst_reader.find(p.type_name) != m_inst_reader.end()) {
				Instance inst_ = Instance(p.type_name, p.getValue(inst.instance()));
				m_inst_reader[p.type_name](p.name, inst_);
			}
		}
	};
	m_inst_reader[MetaTypeOf<DerivObj>().typeName()] = [](std::string inst_name, const Instance& inst) {
		std::cout << inst.typeName() << " " << inst_name << " " << &inst.getValue<DerivObj>() << std::endl;
		MetaType mt = MetaTypeOf<DerivObj>();
		for (auto& p : mt.properties()) {
			if (m_inst_reader.find(p.type_name) != m_inst_reader.end()) {
				Instance inst_ = Instance(p.type_name, p.getValue(inst.instance()));
				m_inst_reader[p.type_name](p.name, inst_);
			}
		}
	};
}

void traitsTest() {
	std::cout << "==========" << __FUNCTION__ << "==========" << std::endl;
	auto n0 = MetaTypeOf<int>();
	auto n1 = MetaTypeOf<const int&>();
	auto n2 = MetaTypeOf<int&>();
	auto n3 = MetaTypeOf<int*&>();
	auto n4 = MetaTypeOf<int**&&>();
	auto n5 = MetaTypeOf<int***>();
	auto n6 = MetaTypeOf<int* const>();
	auto n7 = MetaTypeOf<const int*& const>();
	const int&& ia = 1;
	const int* ib = new int(2);
	auto n8 = MetaTypeOf(ia);
	auto n9 = MetaTypeOf(ib);
	delete ib;
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
	for (auto method : mt.methods()) {
		std::cout << method.signature << std::endl;
	}
	std::vector<float> v = { 3.0f };
	auto ret = mt.method("getId").invoke<int>(&obj, 1, std::string("2"), v);
	mt.method("dumb").invoke<void>(&obj);
}

void createInstanceTest() {
	std::cout << "==========" << __FUNCTION__ << "==========" << std::endl;
	MetaType mt = MetaTypeOf<Obj>();
	Constructor obj_ctor = mt.constructor<>();
	obj_ctor.invoke<Obj>();
}

void polymorphismTest() {
	std::cout << "==========" << __FUNCTION__ << "==========" << std::endl;
	Obj& obj = DerivObj();
	auto n1 = MetaTypeOf(obj).typeName();
	auto n2 = MetaTypeOf<DerivObj>().typeName();
	bool equal = n1 == n2;

	MetaType mt = MetaTypeOf(obj);
	auto typeName = mt.typeName();
}

void serializeTest() {
	std::cout << "==========" << __FUNCTION__ << "==========" << std::endl;
	DerivObj obj;
	Instance inst = Instance(obj);
	m_inst_reader[inst.typeName()]("obj", inst);
}

void variantTest() {
	std::cout << "==========" << __FUNCTION__ << "==========" << std::endl;

	Variant v_i = 2;
	v_i.setValue(3);
	int i = v_i.getValue<int>();

	int* i_ptr = new int(3);
	int** i_pptr = new int*(i_ptr);
	Variant v_i_ptr = i_ptr;
	int* i_p = v_i_ptr.getValue<int*>();
	Variant v_i_pptr = i_pptr;
	int** i_pp = v_i_pptr.getValue<int**>();
}

int main() {
	registerAll();
	traitsTest();
	readPropertiesTest();
	writePropertiesTest();
	serializeTest();
	invokeMethodsTest();
	createInstanceTest();
	polymorphismTest();
	variantTest();
	return 0;
}