#pragma once

#include "Attributes.h"

#include "dependencies/imnodes.h"

#include <vector>

namespace ne {
	class NodeBase {
	public:
		// Members required by ImNodes
		int id{ -1 };
		std::string title;

		const float nodeWidth{ 200 };

		// Note that, when virtual Draw method is added to NodeBase it is not an aggregate class anymore
		// hence it cannot be aggregate initialized, i.e. NodeBase { -1, "title" } implicit constructor cease to exist
		NodeBase(std::string title)
			: title{ title } {}

		void Draw();

		virtual void DrawContent() const = 0;

		// helper to assign unique Ids to every attribute of a node, and for graph to keep references to attributes
		virtual std::vector<std::reference_wrapper<AttributeBase>> GetAllAttributes() = 0;
	};

	template<typename T>
	concept IsNode = std::is_base_of<NodeBase, T>::value;

	template <typename TObj>
	class ObjectEditorNode : public NodeBase {
	public:
		// Object that is populated using UI
		TObj object{};

		// Attributes that refer to Object members. Created at Node construction.
		std::vector<ValueAttribute> inputs;
		ObjectOutputAttribute output;

		// initialize a default object
		ObjectEditorNode(std::string title)
			: NodeBase{ title }, object{}, output{ object } {
			AddInputs(object);
		}

		// construct an object with given arguments
		template<typename... Args>
		ObjectEditorNode(std::string title, Args... args)
			: NodeBase{ title }, object{ args... }, output{ object } {
			AddInputs(object);
		}

		// move provided object
		ObjectEditorNode(std::string title, TObj&& obj)
			: NodeBase{ title }, object{ std::move(obj) }, output{ object } {
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
				ImNodes::EndInputAttribute();
			}

			{
				const auto& attr{ output };
				ImNodes::BeginOutputAttribute(attr.id);
				const float labelWidth{ ImGui::CalcTextSize(attr.name.c_str()).x };
				ImGui::Indent(nodeWidth - labelWidth);
				ImGui::Text(attr.name.c_str());
				ImNodes::EndOutputAttribute();
			}
		}

		std::vector<std::reference_wrapper<AttributeBase>> GetAllAttributes() override {
			std::vector<std::reference_wrapper<AttributeBase>> attrs;
			for (AttributeBase& attr : inputs)
				attrs.push_back(attr);
			attrs.push_back(output);
			return attrs;
		}
	private:
		void AddInputs(VkAttachmentDescription& obj) {
			inputs.emplace_back("flags", obj.flags);
			inputs.emplace_back("format", obj.format);
			inputs.emplace_back("samples", obj.samples);
			inputs.emplace_back("load op", obj.loadOp);
			inputs.emplace_back("store op", obj.storeOp);
			inputs.emplace_back("stencil load op", obj.stencilLoadOp);
			inputs.emplace_back("stencil store op", obj.stencilStoreOp);
			inputs.emplace_back("initial layout", obj.initialLayout);
			inputs.emplace_back("final layout", obj.finalLayout);
		}
		void AddInputs(YourStruct& yourStruct) {
			inputs.emplace_back("num", yourStruct.num);
			inputs.emplace_back("magnitude", yourStruct.magnitude);
			inputs.emplace_back("color components", yourStruct.colorComponents);
		}
	};


	class ObjectViewerNode : public NodeBase {
	public:
		ObjectInputAttribute input;

		ObjectViewerNode() : NodeBase{ "Viewer" } {}

		struct Drawer {
			void operator()(float& val);
			void operator()(int& val);
			void operator()(VkAttachmentDescription& obj);
			void operator()(YourStruct& obj);
		};

		void DrawContent() const override;

		std::vector<std::reference_wrapper<AttributeBase>> GetAllAttributes() override;
	};
}