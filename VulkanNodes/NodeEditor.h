#pragma once

#include <string>
#include <variant>
#include <vector>
#include <memory>

#include "dependencies/imnodes.h"

// ATTEMPT 1

// Cannot have a Value& in Attribute but Value can be made of references!
using Value = std::variant<std::reference_wrapper<int>, std::reference_wrapper<float>>;

struct Attribute {
	int id;
	std::string name;
	Value value;
};

// to std::visit and draw UI of attributes
struct AttributeDrawer {
	bool operator()(float& val);
	bool operator()(int& val);
};

struct Node1 {
	int id;
	std::string title;
	std::vector<Attribute> inputs;
	std::vector<Attribute> outputs;
	float value;
};

struct Link1 {
	int id;
	int start_attr, end_attr;
};

struct Graph1 {
	std::vector<Node1> nodes;
	std::vector<Link1> links;
	int counter = 0;
};

// ATTEMPT 2

struct MyStruct {
	int count;
	float magnitude;
};

class BaseAttribute {
public:
	int id;
	std::string name;
	// T& val; // Derived classes will have a member "val" of any type that'll be manipulated via UI

	// draw UI to manipulate val of type T
	virtual bool Draw() = 0;
protected:
	BaseAttribute(int id, std::string name) : id(id), name(name) {}
};

class FloatAttribute : public BaseAttribute {
private:
	float& val;
public:
	FloatAttribute() = delete;
	FloatAttribute(int id, std::string name, float& x) : BaseAttribute(id, name), val(x) {}

	virtual bool Draw() override;
};

class IntAttribute : public BaseAttribute {
private:
	int& val;
public:
	IntAttribute() = delete;
	IntAttribute(int id, std::string name, int& x) : BaseAttribute(id, name), val(x) {}

	virtual bool Draw() override;
};

struct Node2 {
	int id;
	std::string title;
	// cannot have reference to abstract base class, there for pointer needed
	std::vector<std::shared_ptr<BaseAttribute>> inputs;
	std::vector<std::shared_ptr<BaseAttribute>> outputs;
};

struct Graph2 {
	std::vector<Node2> nodes;
	// links here
	int counter = 1000;
};

class NodeEditor {
public:
	NodeEditor();
	void Draw();
public:
	Graph1 graph1 = {};
	Graph2 graph2 = {};

	// TODO: Store actual values to be edited somewhere else
	MyStruct ms = { 7, 8.5 };
	float f1 = 0.3f, f2 = 0.4f, f3 = 0.6f;
	int i1 = 5;
private:
	AttributeDrawer drawer;
	ImNodesEditorContext* context = nullptr;
};

