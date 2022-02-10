#include "NodeEditor.h"

#include <string>

namespace ne {
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