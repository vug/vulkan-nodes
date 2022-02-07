#pragma once

#include "dependencies/imnodes.h"

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
		int id;
		std::string name;

		AttributeBase(int id, std::string name) : id{ id }, name{ name } {}

		virtual bool Draw() const = 0;
	};

	// Cannot have a Value& in Attribute but Value can be made of references!
	using ValueRef = std::variant<std::reference_wrapper<int>, std::reference_wrapper<float>, std::reference_wrapper<YourEnum>>;

	struct AttributeDrawer {
		bool operator()(float& val);
		bool operator()(int& val);
		bool operator()(YourEnum& val);
	};

	class ValueAttribute : public AttributeBase {
	public:
		ValueRef value;

		ValueAttribute(int id, std::string title, ValueRef value)
			: AttributeBase{ id, title }, value{ value } {}

		bool Draw() const override;

	};

	// ---------

	class NodeBase {
	public:
		int id;
		std::string title;

		const float nodeWidth{ 100 };

		// Note that, when virtual Draw method is added to NodeBase it is not an aggregate class anymore
		// hence it cannot be aggregate initialized, i.e. NodeBase { -1, "title" } implicit constructor cease to exist
		NodeBase(int id, std::string title)
			: id{id}, title{title} {}

		void Draw();

		virtual void DrawContent() const = 0;
	};

	template <typename TObj>
	class ObjectEditorNode : public NodeBase {
	public:
		// Object that is populated using UI
		TObj object;

		// Attributes that refer to Object members. Created at Node construction.
		std::vector<ValueAttribute> inputs;
		// TODO: Define ObjectOutputAttribute
		//ObjectOutputAttribute output;

		ObjectEditorNode(std::string title)
			: NodeBase{-1, title}, object{} { 
			AddInputs(object);
		}

		template<typename... Args>
		ObjectEditorNode(std::string title, Args... args) 
			: NodeBase{ -1, title }, object{ args... } {
			AddInputs(object);
		}

		ObjectEditorNode(std::string title, TObj&& obj)
			: NodeBase{ -1, title }, object{std::move(obj)} {
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
	private:
		void AddInputs(MyStruct& myStruct) {
			inputs.emplace_back(-1, "count", myStruct.count);
			inputs.emplace_back(-1, "magnitude", myStruct.magnitude);
		}
		void AddInputs(YourStruct& yourStruct) {
			inputs.emplace_back(-1, "option", yourStruct.option);
			inputs.emplace_back(-1, "num", yourStruct.num);
		}
	};

	// ----------------

	class Graph {
	public:
		std::vector<std::shared_ptr<NodeBase>> nodes;
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