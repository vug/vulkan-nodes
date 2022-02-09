#pragma once

#include "dependencies/imnodes.h"

#include <cassert>
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace ne {
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

	// ---------

	class AttributeBase {
	public:
		// Members required by ImNode
		int id{ -1 };
		std::string name;

		AttributeBase(std::string name) : name{ name } {}

		// Logic to draw Attribute UI in a Node body
		virtual bool Draw() const = 0;
	};

	// Cannot have a Value& in Attribute but Value can be made of references!
	using ValueRef = std::variant<std::reference_wrapper<int>, std::reference_wrapper<float>, std::reference_wrapper<YourEnum>>;

	// Visitor that draws UI for ValueAttribute editing
	struct AttributeDrawer {
		bool operator()(float& val);
		bool operator()(int& val);
		bool operator()(YourEnum& val);
	};

	class ValueAttribute : public AttributeBase {
	public:
		ValueRef value;

		ValueAttribute(std::string title, ValueRef value)
			: AttributeBase{ title }, value{ value } {}

		bool Draw() const override;
	};

	// ---------

	class NodeBase {
	public:
		// Members required by ImNodes
		int id{ -1 };
		std::string title;

		const float nodeWidth{ 100 };

		// Note that, when virtual Draw method is added to NodeBase it is not an aggregate class anymore
		// hence it cannot be aggregate initialized, i.e. NodeBase { -1, "title" } implicit constructor cease to exist
		NodeBase(std::string title)
			: title{title} {}

		void Draw();

		virtual void DrawContent() const = 0;

		// helper to assign unique Ids to every attribute of a node
		virtual std::vector<std::reference_wrapper<int>> GetAllAttributeIds() = 0;
	};

	template <typename TObj>
	class ObjectEditorNode : public NodeBase {
	public:
		// Object that is populated using UI
		TObj object{};

		// Attributes that refer to Object members. Created at Node construction.
		std::vector<ValueAttribute> inputs;
		// TODO: Define ObjectOutputAttribute
		//ObjectOutputAttribute output;

		// initialize a default object
		ObjectEditorNode(std::string title)
			: NodeBase{ title }, object{} { 
			AddInputs(object);
		}

		// construct an object with given arguments
		template<typename... Args>
		ObjectEditorNode(std::string title, Args... args) 
			: NodeBase{ title }, object{ args... } {
			AddInputs(object);
		}

		// move provided object
		ObjectEditorNode(std::string title, TObj&& obj)
			: NodeBase{ title }, object{std::move(obj)} {
			AddInputs(object);
		}

		void DrawContent() const override {
			for (const auto& attr : inputs) {
				ImNodes::BeginInputAttribute(attr.id);
				const float labelWidth{ ImGui::CalcTextSize(attr.name.c_str()).x };
				ImGui::TextUnformatted(attr.name.c_str());

				ImGui::SameLine();
				ImGui::PushItemWidth(nodeWidth - labelWidth);

				attr.Draw();

				ImGui::PopItemWidth();
				ImNodes::EndOutputAttribute();
			}
		}

		std::vector<std::reference_wrapper<int>> GetAllAttributeIds() override {
			std::vector<std::reference_wrapper<int>> ids;
			for (AttributeBase& attr : inputs)
				ids.push_back(attr.id);
			return ids;
		}
	private:
		void AddInputs(MyStruct& myStruct) {
			inputs.emplace_back("count", myStruct.count);
			inputs.emplace_back("magnitude", myStruct.magnitude);
		}
		void AddInputs(YourStruct& yourStruct) {
			inputs.emplace_back("option", yourStruct.option);
			inputs.emplace_back("num", yourStruct.num);
		}
	};

	// ----------------

	template<typename T>
	concept IsNode = std::is_base_of<NodeBase, T>::value;

	class Graph {
	public:
		std::vector<std::shared_ptr<NodeBase>> nodes;
		int counter{};

		void AddNode(std::shared_ptr<NodeBase> nd) {
			assert(nd->id != -1); // node should be given an id
			for (std::reference_wrapper<int>& idRef : nd->GetAllAttributeIds())
				assert(idRef.get() != -1);  // all attributes of a node should be given an id
			nodes.push_back(nd);
		}

		template<IsNode TNode, typename... Args>
		void AddNode(Args... args) {
			std::shared_ptr<TNode> nd = std::make_shared<TNode>(args...);
			nd->id = counter++;
			nodes.push_back(nd);

			for (auto& idRef : nd->GetAllAttributeIds())
				idRef.get() = counter++;

		}
	};

	// ----------------

	class NodeEditor {
	public:
		NodeEditor();
		NodeEditor(const Graph& graph);
		void Draw();

		static Graph MakeTestGraph();
	public:
		Graph graph{};

	private:
		ImNodesEditorContext* context = ImNodes::EditorContextCreate();
	};
}