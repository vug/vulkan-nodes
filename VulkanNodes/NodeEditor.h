#pragma once

#include <string>
#include <variant>
#include <vector>

#include "dependencies/imnodes.h"

struct MyStruct {
	int count;
	float magnitude;
};

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

// Objects that can be hold/represented by Nodes
using Object = std::variant<std::reference_wrapper<MyStruct>, std::reference_wrapper<int>, std::reference_wrapper<float>>;

// TODO: placeholder until I figure out how to get the reference to Object from output
static int outVal = 6;

class Node {
public:
	// for ImNodes
	int id = -1;
	std::string title;
	// Object that is populated using UI
	Object object;
	// Attributes that refer to Object members. Created at Node construction.
	std::vector<Attribute> inputs;
	std::vector<Attribute> outputs;

	// Used in constructor to populate inputs and outputs based on the Object
	struct InputAddingVisitor {
		Node& node;
		void operator()(MyStruct& ms);
		void operator()(int& num);
		void operator()(float& num);
	};
	InputAddingVisitor adder = { *this };

	Node(std::string title, Object obj);
};

struct Link {
	int id;
	int start_attr;
	int end_attr;
};

struct Graph {
	std::vector<Node> nodes;
	std::vector<Link> links;
	int counter = 0;
	void AddNode(Node node) {
		node.id = ++counter;
		for (auto& attr : node.inputs) attr.id = ++counter;
		for (auto& attr : node.outputs) attr.id = ++counter;
		nodes.push_back(node);
	}
};

class NodeEditor {
public:
	NodeEditor();
	void Draw();
public:
	Graph graph = {};

	// TODO: Store actual values to be edited somewhere else
	MyStruct ms1 = { 7, 8.5 };
	MyStruct ms2 = { 4, 1.5 };
	int myNum = 66;
private:
	AttributeDrawer drawer;
	ImNodesEditorContext* context = nullptr;
};

