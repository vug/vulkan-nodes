#include "Nodes.h"

#include <imgui_stdlib.h>

#include <string>

namespace ne {
	void NodeBase::Draw() {
		ImNodes::BeginNode(id);

		ImNodes::BeginNodeTitleBar();
		ImGui::TextUnformatted(title.c_str());
		ImNodes::EndNodeTitleBar();

		DrawContent();
		ImNodes::EndNode();

		const std::string label = std::to_string(id) + "NodePopup";
		if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
			ImGui::OpenPopup(label.c_str());
		if (ImGui::BeginPopup(label.c_str())) {
			ImGui::InputText("rename", &title);
			ImGui::EndPopup();
		}
	}

	// -------

	void ObjectViewerNode::Drawer::operator()(float& val) {
		ImGui::Text("%f", val);
	}

	void ObjectViewerNode::Drawer::operator()(int& val) {
		ImGui::Text("%d", &val);
	}

	void ObjectViewerNode::Drawer::operator()(VkAttachmentDescription& obj) {
		ImGui::Text("MyStruct");
		ImGui::Text("flags: %s", enums::GetFlagLabel(obj.flags).c_str());
		ImGui::Text("format: %s", enums::GetEnumLabel(obj.format));
		ImGui::Text("samples: %s", enums::GetFlagLabel(obj.samples).c_str());
		ImGui::Text("load op: %s", enums::GetEnumLabel(obj.loadOp));
		ImGui::Text("store op: %s", enums::GetEnumLabel(obj.storeOp));
		ImGui::Text("stencil load op: %s", enums::GetEnumLabel(obj.stencilLoadOp));
		ImGui::Text("stencil store op: %s", enums::GetEnumLabel(obj.stencilStoreOp));
		ImGui::Text("initial layout: %s", enums::GetEnumLabel(obj.initialLayout));
		ImGui::Text("final layout: %s", enums::GetEnumLabel(obj.finalLayout));
	}

	void ObjectViewerNode::Drawer::operator()(YourStruct& obj) {
		ImGui::Text("YourStruct");
		ImGui::Text("num: %d", obj.num);
		ImGui::Text("magnitude: %f", obj.magnitude);
		ImGui::Text("color components: %s", enums::GetFlagLabel(obj.colorComponents).c_str());
	}

	void ObjectViewerNode::DrawContent() const {
		ImNodes::BeginInputAttribute(input.id);
		ImGui::Text(input.name.c_str());
		input.Draw();
		ImNodes::EndInputAttribute();

		if (input.optObject.has_value()) {
			std::visit(Drawer{}, input.optObject.value());
		}
		else {
			ImGui::Text("no input");
		}
	}

	std::vector<std::reference_wrapper<AttributeBase>> ObjectViewerNode::GetAllAttributes() {
		std::vector<std::reference_wrapper<AttributeBase>> attrs = { input };
		return attrs;
	}
}