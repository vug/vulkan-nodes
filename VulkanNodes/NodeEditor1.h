#pragma once

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "dependencies/imnodes.h"

namespace ne1 {

	struct MyStruct {
		int count;
		float magnitude;
	};

	enum class YourEnum {
		Opt1,
		Opt2,
	};

	struct YourStruct {
		YourEnum option;
		int num;
	};

	// Cannot have a Value& in Attribute but Value can be made of references!
	using ValueRef = std::variant<std::reference_wrapper<int>, std::reference_wrapper<float>, std::reference_wrapper<YourEnum>>;
	// Objects that can be hold/represented by Nodes
	using ObjectRef = std::variant<std::reference_wrapper<MyStruct>, std::reference_wrapper<YourStruct>, std::reference_wrapper<int>, std::reference_wrapper<float>>;

	class AttributeBase {
	public:
		int id;
		std::string name;
		int type = -2;

		AttributeBase(int id, std::string name, int type) : id{ id }, name{ name }, type{ type } {}
		virtual void foo() {}
	};

	class ValueAttribute : public AttributeBase {
	public:
		ValueRef value;

		ValueAttribute(int id, std::string name, ValueRef value) : AttributeBase{ id, name, 10 }, value{ value } {}
	};

	class ObjectOutputAttribute : public AttributeBase {
	public:
		ObjectRef object;

		ObjectOutputAttribute(int id, std::string name, ObjectRef object) : AttributeBase{ id, name, 20 }, object{ object } {}
	};

	class ViewerInputAttribute : public AttributeBase {
	public:
		std::optional<ObjectRef> optObject;

		ViewerInputAttribute(int id, std::string name) : AttributeBase{ id, name, 30 }, optObject{} {}
		ViewerInputAttribute(int id, std::string name, ObjectRef object) : AttributeBase{ id, name, 30 }, optObject{ object } {}
	};

	struct AttributeDrawer {
		bool operator()(float& val);
		bool operator()(int& val);
		bool operator()(YourEnum& val);
	};

	struct ViewerNodeDrawer {
		void operator()(float& val);
		void operator()(int& val);
		void operator()(MyStruct& obj);
		void operator()(YourStruct& obj);
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
			void operator()(YourStruct& ms);
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
		std::unordered_map<int, Link> links;
		std::unordered_map<int, std::reference_wrapper<AttributeBase>> attributes;
		int counter{};
		void AddNode(std::shared_ptr<ObjectEditorNode> node) {
			node->id = ++counter;
			for (auto& attr : node->inputs) {
				attr.id = ++counter;
				attributes.insert({ attr.id, attr });
			}
			node->output.id = ++counter;
			attributes.insert({ node->output.id, node->output });
			nodes.push_back(node);
		}
		void AddNode(std::shared_ptr<ObjectViewerNode> node) {
			node->id = ++counter;
			node->input.id = ++counter;
			attributes.insert({ node->input.id, node->input });				
			nodes.push_back(node);
		}

		void AddLink(const int attr1Id, const int attr2Id) {
			assert(attributes.contains(attr1Id));
			assert(attributes.contains(attr2Id));
			AddLink(attributes.at(attr1Id), attributes.at(attr2Id));
		}
		void AddLink(AttributeBase& attr1, AttributeBase& attr2) {
			AttributeBase* pBase1 = &attr1;
			AttributeBase* pBase2 = &attr2;
			if (ViewerInputAttribute* attrIn = dynamic_cast<ViewerInputAttribute*>(pBase2)) {
				if (ObjectOutputAttribute* attrOut = dynamic_cast<ObjectOutputAttribute*>(pBase1)) {

					// does input attr already has connection?
					std::vector<int> linksToDelete;
					for (const auto& [linkId, link] : links) {
						// if yes, mark input's link for deletion, and reset view reference
						if (link.end_attr == attrIn->id) {
							linksToDelete.push_back(linkId);
							attrIn->optObject.reset();
						}
					}
					for (int linkId : linksToDelete)
						links.erase(linkId);
					// new view reference
					attrIn->optObject = attrOut->object;
				}
			}

			const int id = ++counter;
			links[id] = { id, attr1.id, attr2.id };
		}

		void RemoveLink(const int linkId) {
			assert(links.contains(linkId)); // linkId to be destroyed should exist
			const auto& link = links.at(linkId);
			AttributeBase& in = attributes.at(link.end_attr);
			AttributeBase* pIn = &in;
			ViewerInputAttribute* attrIn = dynamic_cast<ViewerInputAttribute*>(pIn);
			if (attrIn)
				attrIn->optObject.reset();
			links.erase(linkId);
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

