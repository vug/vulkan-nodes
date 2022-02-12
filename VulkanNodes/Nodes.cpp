#include "Nodes.h"

namespace ne {
	void NodeBase::Draw() {
		ImNodes::BeginNode(id);

		ImNodes::BeginNodeTitleBar();
		ImGui::TextUnformatted(title.c_str());
		ImNodes::EndNodeTitleBar();

		DrawContent();
		ImNodes::EndNode();
	}

	// -------

	void ObjectViewerNode::Drawer::operator()(float& val) {
		ImGui::Text("%f", val);
	}

	void ObjectViewerNode::Drawer::operator()(int& val) {
		ImGui::Text("%d", &val);
	}

	void ObjectViewerNode::Drawer::operator()(MyStruct& obj) {
		ImGui::Text("MyStruct");
		ImGui::Text("count: %d", obj.count);
		ImGui::Text("magnitude: %f", obj.magnitude);
		ImGui::Text("image layout: %s", VkImageLayoutLabels[VkImageLayoutToIndex(obj.imageLayout)]);
	}

	void ObjectViewerNode::Drawer::operator()(YourStruct& obj) {
		ImGui::Text("YourStruct");
		const char* items[] = { "Opt1", "Opt2" };
		ImGui::Text("option: %s", items[static_cast<int>(obj.option)]);
		ImGui::Text("num: %d", obj.num);
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