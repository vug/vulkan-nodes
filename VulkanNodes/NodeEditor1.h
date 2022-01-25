#pragma once

#include <memory>
#include <optional>
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
	using ValueRef = std::variant<std::reference_wrapper<int>, std::reference_wrapper<float>>;
	// Objects that can be hold/represented by Nodes
	using ObjectRef = std::variant<std::reference_wrapper<MyStruct>, std::reference_wrapper<int>, std::reference_wrapper<float>>;

	class AttributeBase {
	public:
		int id;
		std::string name;
	};

	class ValueAttribute : public AttributeBase {
	public:
		ValueRef value;

		ValueAttribute(int id, std::string name, ValueRef value) : AttributeBase{ id, name }, value{ value } {}
	};

	class ObjectOutputAttribute : public AttributeBase {
	public:
		ObjectRef object;

		ObjectOutputAttribute(int id, std::string name, ObjectRef object) : AttributeBase{ id, name }, object{ object } {}
	};

	class ViewerInputAttribute : public AttributeBase {
	public:
		std::optional<ObjectRef> optObject;

		ViewerInputAttribute(int id, std::string name) : AttributeBase{ id, name }, optObject{} {}
		ViewerInputAttribute(int id, std::string name, ObjectRef object) : AttributeBase{ id, name }, optObject{ object } {}
	};

	struct AttributeDrawer {
		bool operator()(float& val);
		bool operator()(int& val);
	};

	struct ViewerNodeDrawer {
		void operator()(float& val);
		void operator()(int& val);
		void operator()(MyStruct& obj);
	};

	class NodeBase {
	public:
		int id{ -1 };
		std::string title{};

		const float nodeWidth{ 100 };

		NodeBase(int id, std::string title) : id{ id }, title{ title } {}
		virtual void Draw() const = 0;
	};

	class ObjectEditorNode : public NodeBase {
	public:
		// Object that is populated using UI
		ObjectRef object;
		// Attributes that refer to Object members. Created at Node construction.
		std::vector<ValueAttribute> inputs;
		ObjectOutputAttribute output;

		// Used in constructor to populate inputs and outputs based on the Object
		struct InputAddingVisitor {
			ObjectEditorNode& node;
			void operator()(MyStruct& ms);
			void operator()(int& num);
			void operator()(float& num);
		};
		InputAddingVisitor adder{ *this };

		ObjectEditorNode(std::string title, ObjectRef obj);
		virtual void Draw() const override;
	};

	class ObjectViewerNode : public NodeBase {
	public:
		ViewerInputAttribute input;

		ObjectViewerNode() : NodeBase{ -1, "Viewer" }, input{ -1, "input" } {}
		virtual void Draw() const override;
	};

	struct Link {
		int id;
		int start_attr;
		int end_attr;
	};

	struct Graph {
		std::vector<std::shared_ptr<NodeBase>> nodes;
		std::vector<Link> links;
		int counter{};
		void AddNode(std::shared_ptr<ObjectEditorNode> node) {
			node->id = ++counter;
			for (auto& attr : node->inputs) attr.id = ++counter;
			node->output.id = ++counter;
			nodes.push_back(node);
		}
		void AddNode(std::shared_ptr<ObjectViewerNode> node) {
			node->id = ++counter;
			if (node->input.optObject.has_value())
				node->input.id = ++counter;
			nodes.push_back(node);
		}
		void AddLink(const AttributeBase& attr1, const AttributeBase attr2) {
			links.emplace_back(++counter, attr1.id, attr2.id);
		}
	};

	class NodeEditor {
	public:
		NodeEditor();
		NodeEditor(const Graph& graph);
		void Draw();
	public:
		Graph graph{};

		// TODO: Store actual values to be edited somewhere else
		MyStruct ms1{ 7, 8.5 };
		MyStruct ms2{ 4, 1.5 };
		int myNum{ 66 };
	private:
		ImNodesEditorContext* context = ImNodes::EditorContextCreate();
		Graph MakeTestGraph();
	};

} // namespace ne1

