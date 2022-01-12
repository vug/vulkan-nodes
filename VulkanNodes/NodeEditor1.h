#pragma once

#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "dependencies/imnodes.h"

namespace ne1 {

	struct MyStruct {
		int count;
		float magnitude;
	};

	// Cannot have a Value& in Attribute but Value can be made of references!
	using Value = std::variant<std::reference_wrapper<int>, std::reference_wrapper<float>>;

	class AttributeBase {
	public:
		int id;
		std::string name;

		AttributeBase(int id, std::string title) : id(id), name(name) {}
	};

	class ValueAttribute : public AttributeBase {
	public:
		Value value;

		ValueAttribute(int id, std::string name, Value value) : AttributeBase(id, name), value(value) {}
	};

	// to std::visit and draw UI of attributes
	struct AttributeDrawer {
		bool operator()(float& val);
		bool operator()(int& val);
		bool operator()(MyStruct& obj);
	};

	// Objects that can be hold/represented by Nodes
	using Object = std::variant<std::reference_wrapper<MyStruct>, std::reference_wrapper<int>, std::reference_wrapper<float>>;

	class ObjectAttribute : public AttributeBase {
	public:
		Object object;

		ObjectAttribute(int id, std::string name, Object object) : AttributeBase(id, name), object(object) {}
	};

	// TODO: placeholder until I figure out how to get the reference to Object from output
	static int outVal = 6;

	class NodeBase {
	public:
		int id = -1;
		std::string title;

		const float nodeWidth = 100;

		NodeBase(int id, std::string title) : id(id), title(title) {}
		virtual void Draw() = 0;
	};

	class Node : public NodeBase {
	public:
		// Object that is populated using UI
		Object object;
		// Attributes that refer to Object members. Created at Node construction.
		std::vector<ValueAttribute> inputs;
		ObjectAttribute output;

		// Used in constructor to populate inputs and outputs based on the Object
		struct InputAddingVisitor {
			Node& node;
			void operator()(MyStruct& ms);
			void operator()(int& num);
			void operator()(float& num);
		};
		InputAddingVisitor adder = { *this };

		Node(std::string title, Object obj);
		virtual void Draw() override;
	};

	class ObjectViewerNode : public NodeBase {
	public:
		// TODO: instead of the whole Attribute just make the object of the attribute a pointer
		std::shared_ptr<ObjectAttribute> input = nullptr;

		ObjectViewerNode() : NodeBase(-1, "Viewer") {}
		virtual void Draw() override;
	};

	struct Link {
		int id;
		int start_attr;
		int end_attr;
	};

	struct Graph {
		std::vector<std::shared_ptr<NodeBase>> nodes;
		std::vector<Link> links;
		int counter = 0;
		void AddNode(std::shared_ptr<Node> node) {
			node->id = ++counter;
			for (auto& attr : node->inputs) attr.id = ++counter;
			node->output.id = ++counter;
			nodes.push_back(node);
		}
		void AddNode(std::shared_ptr<ObjectViewerNode> node) {
			node->id = ++counter;
			if (node->input != nullptr)
				node->input->id = ++counter;
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
		ImNodesEditorContext* context = nullptr;
	};

} // namespace ne1

