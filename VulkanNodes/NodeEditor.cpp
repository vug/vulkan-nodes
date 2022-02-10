#include "NodeEditor.h"

#include <string>

namespace ne {
	bool AttributeDrawer::operator()(float& val) {
		return ImGui::DragFloat("##hidelabel", &val, 0.01f);
	}

	bool AttributeDrawer::operator()(int& val) {
		return ImGui::DragInt("##hidelabel", &val);
	}

	bool AttributeDrawer::operator()(YourEnum& val) {
		const char* items[] = { "Opt1", "Opt2" };
		int choice = static_cast<int>(val);
		bool hasModified = ImGui::Combo("##hidelabel", &choice, items, IM_ARRAYSIZE(items));
		val = static_cast<YourEnum>(choice);
		return hasModified;
	}

	// -------

	bool ValueAttribute::Draw() const {
		return std::visit(AttributeDrawer{}, value);
	}

	// -------

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

	std::vector<std::reference_wrapper<int>> ObjectViewerNode::GetAllAttributeIds() {
		std::vector<std::reference_wrapper<int>> ids = { input.id };
		return ids;
	}

	// -------

	NodeEditor::NodeEditor(const Graph& graph) : graph{ graph } {
		ImNodes::EditorContextSet(context);
		ImNodes::PushAttributeFlag(ImNodesAttributeFlags_EnableLinkDetachWithDragClick);
		ImNodesIO& io{ ImNodes::GetIO() };
		io.LinkDetachWithModifierClick.Modifier = &ImGui::GetIO().KeyCtrl;
	}

	NodeEditor::NodeEditor() : NodeEditor{ Graph{} } {}

	void NodeEditor::Draw() {
		ImGui::Begin("Node Editor");
		ImGui::TextUnformatted("A: add node. CTRL+s: save node pos. CTRL+l: load node pos.");
		// Hack for learning key codes
		//for (int key = 0; key < 200; key++) { if (ImGui::IsKeyDown(key)) ImGui::Text("key: %d", key); }
		ImNodes::BeginNodeEditor();

		for (const auto& nd : graph.nodes) {
			nd->Draw();
		}

		ImNodes::MiniMap();
		ImNodes::MiniMap(0.2f, ImNodesMiniMapLocation_BottomRight);
		ImNodes::EndNodeEditor();
		ImGui::End();
	}

	Graph NodeEditor::MakeTestGraph() {
		Graph graph{};
		auto nd1 = graph.AddNode<ne::ObjectEditorNode<ne::MyStruct>>("Node1", 2, 3.0f);
		auto nd2 = graph.AddNode<ne::ObjectEditorNode<ne::YourStruct>>("Node2", ne::YourEnum::Opt2, 4);
		auto nd3 = graph.AddNode<ne::ObjectViewerNode>();
		nd3->input.optObject = nd2->output.object;
		return graph;
	}
}