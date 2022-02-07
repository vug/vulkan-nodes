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
		bool hasModified = ImGui::Combo("combo", &choice, items, IM_ARRAYSIZE(items));
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
		auto nd2 = std::make_shared<ne::ObjectEditorNode<ne::MyStruct>>("Node1", 3, 4.0f);
		graph.nodes.push_back(nd2);
		nd2->id = 1;
		nd2->inputs[0].id = 2;
		nd2->inputs[1].id = 3;
		return graph;
	}
}