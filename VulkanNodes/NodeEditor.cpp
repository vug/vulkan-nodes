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

		DrawPopupMenu();

		for (const auto& nd : graph.nodes) {
			nd->Draw();
		}

		ImNodes::MiniMap();
		ImNodes::MiniMap(0.2f, ImNodesMiniMapLocation_BottomRight);
		ImNodes::EndNodeEditor();
		ImGui::End();
	}

	void NodeEditor::DrawPopupMenu() {
		const bool shouldOpenPopup = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
			ImNodes::IsEditorHovered() &&
			ImGui::IsKeyReleased(65);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.f, 8.f));

		if (!ImGui::IsAnyItemHovered() && shouldOpenPopup)
			ImGui::OpenPopup("Add Object");

		if (ImGui::BeginPopup("Add Object")) {
			const ImVec2 clickPos = ImGui::GetMousePosOnOpeningCurrentPopup();

			if (ImGui::MenuItem("MyStruct")) {
				auto node = graph.AddNode<ObjectEditorNode<MyStruct>>("MyStruct");
				ImNodes::SetNodeScreenSpacePos(node->id, clickPos);
			}
			if (ImGui::MenuItem("YourStruct")) {
				auto node = graph.AddNode<ObjectEditorNode<YourStruct>>("MyStruct");
				ImNodes::SetNodeScreenSpacePos(node->id, clickPos);
			}
			if (ImGui::MenuItem("Viewer")) {
				auto node = graph.AddNode<ObjectViewerNode>();
				ImNodes::SetNodeScreenSpacePos(node->id, clickPos);
			}

			ImGui::EndPopup();
		}

		ImGui::PopStyleVar();
	}

	Graph NodeEditor::MakeTestGraph() {
		Graph graph{};
		auto nd1 = graph.AddNode<ObjectEditorNode<MyStruct>>("Node1", 2, 3.0f);
		auto nd2 = graph.AddNode<ObjectEditorNode<YourStruct>>("Node2", YourEnum::Opt2, 4);
		auto nd3 = graph.AddNode<ObjectViewerNode>();
		nd3->input.optObject = nd2->output.object;
		return graph;
	}
}