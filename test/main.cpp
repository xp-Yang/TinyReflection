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

std::unordered_map<std::string, std::function<std::string(std::string, const Instance&)>> m_inst_reader;

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
}

void traitsTest() {
	auto n0 = MetaTypeOf<int>();
	auto n1 = MetaTypeOf<const int&>();
	auto n3 = MetaTypeOf<int*&&>();
	auto n5 = MetaTypeOf<int***>();
	auto n6 = MetaTypeOf<int* const>();
	auto n7 = MetaTypeOf<const int*& const>();

	const int&& ia = 1;
	const int* ib = new int(2);
	auto n8 = MetaTypeOf(ia);
	auto n9 = MetaTypeOf(ib);
	delete ib;

	Obj& obj = DerivObj();
	auto n11 = MetaTypeOf(obj).typeName();
	auto n12 = MetaTypeOf<DerivObj>().typeName();
}

void variantTest() {
	Variant v_i = 2;
	v_i.setValue(3);
	int i = v_i.getValue<int>();

	int* i_ptr = new int(3);
	int** i_pptr = new int* (i_ptr);
	Variant v_i_ptr = i_ptr;
	int* i_p = v_i_ptr.getValue<int*>();
	Variant v_i_pptr = i_pptr;
	int** i_pp = v_i_pptr.getValue<int**>();
}

void readPropertiesTest() {
	std::cout << "==========" << __FUNCTION__ << "==========" << std::endl;

	DerivObj obj;
	MetaType mt = MetaTypeOf(obj);
	std::cout << "properties:" << std::endl;
	for (auto& prop : mt.properties()) {
		std::cout 
			<< "type: " << (int)prop.type << " "
			<< "offset: " << prop.offset << " "
			<< "size: " << prop.size << " "
			<< "type_name: " << prop.type_name << " "
			<< "name:" << prop.name << " ";
		if (prop.isType<float>())
			std::cout << "value: " << prop.getValue<float>(obj);
		std::cout << std::endl;
	}
	std::cout << "methods:" << std::endl;
	for (auto method : mt.methods()) {
		std::cout << "signature:" << method.signature << std::endl;
	}
}

void writePropertiesTest() {
	Obj obj;
	MetaType mt = MetaTypeOf(obj);
	Property prop = mt.property("x");
	const float ff = 1.9f;
	prop.setValue(obj, ff);
	prop.setValue(obj, 2.9f);
}

void invokeMethodsTest() {
	std::cout << "==========" << __FUNCTION__ << "==========" << std::endl;
	DerivObj obj;
	MetaType mt = MetaTypeOf(obj);
	std::vector<float> v = { 3.0f };
	auto ret = mt.method("getId").invoke<int>(&obj, 1, std::string("2"), v);
	mt.method("dumb").invoke<void>(&obj);
}

void createInstanceTest() {
	MetaType mt = MetaTypeOf<Obj>();
	Constructor obj_ctor = mt.constructor<>();
	obj_ctor.invoke<Obj>();
}

void serializeTest() {
	std::cout << "==========" << __FUNCTION__ << "==========" << std::endl;

	m_inst_reader[MetaTypeOf<float>().typeName()] = [](std::string inst_name, const Instance& inst) {
		return inst.typeName() + " " + inst_name + " " + std::to_string(inst.getValue<float>());
	};
	m_inst_reader[MetaTypeOf<float*>().typeName()] = [](std::string inst_name, const Instance& inst) {
		return inst.typeName() + " " + inst_name + " " + std::to_string(*(inst.getValue<float*>()));
	};
	m_inst_reader[MetaTypeOf<int>().typeName()] = [](std::string inst_name, const Instance& inst) {
		return inst.typeName() + " " + inst_name + " " + std::to_string(inst.getValue<int>());
	};
	m_inst_reader[MetaTypeOf<std::string>().typeName()] = [](std::string inst_name, const Instance& inst) {
		return inst.typeName() + " " + inst_name + " " + inst.getValue<std::string>();
	};
	m_inst_reader[MetaTypeOf<std::vector<int>>().typeName()] = [](std::string inst_name, const Instance& inst) {
		std::string ret;
		ret = inst.typeName() + " " + inst_name;
		int i = 0;
		const std::vector<int>& vec = inst.getValue<const std::vector<int>&>();
		ret += " [";
		for (const auto& val : vec) {
			ret += std::to_string(val);
			if (i != vec.size() - 1)
				ret += ", ";
			i++;
		}
		ret += "]";
		return ret;
	};
	m_inst_reader[MetaTypeOf<std::vector<std::shared_ptr<Obj>>>().typeName()] = [](std::string inst_name, const Instance& inst) {
		std::string ret;
		ret = inst.typeName() + " " + inst_name;
		int i = 0;
		const std::vector<std::shared_ptr<Obj>>& vec = (inst.getValue<const std::vector<std::shared_ptr<Obj>>&>());
		ret += " [";
		for (const auto& val : vec) {
			Instance inst_ = Instance(static_cast<Obj&>(*val));
			ret += m_inst_reader[inst_.typeName()]("Obj", inst_);
			if (i != vec.size() - 1)
				ret += ", ";
			i++;
		}
		ret += "]";
		return ret;
	};
	m_inst_reader[MetaTypeOf<std::map<std::string, int>>().typeName()] = [](std::string inst_name, const Instance& inst) {
		std::string ret;
		ret = inst.typeName() + " " + inst_name;
		int i = 0;
		const std::map<std::string, int>& map = inst.getValue<const std::map<std::string, int>&>();
		ret += " {";
		for (const auto& pair : map) {
			ret += "{" + pair.first + ", " + std::to_string(pair.second) + "}";
			if (i != map.size() - 1)
				ret += ", ";
			i++;
		}
		ret += "}";
		return ret;
	};
	m_inst_reader[MetaTypeOf<E>().typeName()] = [](std::string inst_name, const Instance& inst) {
		return inst.typeName() + " " + inst_name + " " + std::to_string((int)inst.getValue<E>());
	};
	m_inst_reader[MetaTypeOf<CusType>().typeName()] = [](std::string inst_name, const Instance& inst) {
		std::string ret = inst.typeName() + " " + inst_name + "\n";
		ret += "{\n";
		MetaType mt = MetaTypeOf<CusType>();
		for (auto& p : mt.properties()) {
			if (m_inst_reader.find(p.type_name) != m_inst_reader.end()) {
				Instance inst_ = p.getValue(inst);
				ret += m_inst_reader[p.type_name](p.name, inst_) + "\n";
			}
		}
		ret += "}\n";
		return ret;
	};
	m_inst_reader[MetaTypeOf<Obj>().typeName()] = [](std::string inst_name, const Instance& inst) {
		std::string ret = inst.typeName() + " " + inst_name + "\n";
		ret += "{\n";
		MetaType mt = MetaTypeOf<Obj>();
		for (auto& p : mt.properties()) {
			if (m_inst_reader.find(p.type_name) != m_inst_reader.end()) {
				Instance inst_ = p.getValue(inst);
				ret += m_inst_reader[p.type_name](p.name, inst_) + "\n";
			}
		}
		ret += "}\n";
		return ret;
	};
	m_inst_reader[MetaTypeOf<DerivObj>().typeName()] = [](std::string inst_name, const Instance& inst) {
		std::string ret = inst.typeName() + " " + inst_name + "\n";
		ret += "{\n";
		MetaType mt = MetaTypeOf<DerivObj>();
		for (auto& p : mt.properties()) {
			if (m_inst_reader.find(p.type_name) != m_inst_reader.end()) {
				Instance inst_ = p.getValue(inst);
				ret += m_inst_reader[p.type_name](p.name, inst_) + "\n";
			}
		}
		ret += "}\n";
		return ret;
	};

	DerivObj obj;
	Instance inst = Instance(obj);
	std::cout << m_inst_reader[inst.typeName()]("obj", inst);
}

void printRegisteredTypes() {
	std::cout << "==========" << __FUNCTION__ << "==========" << std::endl;

	int i = 0;
	for (auto& pair : global_class_info) {
		std::cout << std::to_string(i++) << ". type name: " << pair.first << std::endl;
	}
	//for (auto& pair : type_name_map) {
	//	std::cout << "registered raw type name: " << pair.first << " registered type name: " << pair.second << std::endl;
	//}
}

int main() {
	registerAll();
	traitsTest();
	variantTest();
	readPropertiesTest();
	writePropertiesTest();
	invokeMethodsTest();
	createInstanceTest();
	serializeTest();
	printRegisteredTypes();
	return 0;
}