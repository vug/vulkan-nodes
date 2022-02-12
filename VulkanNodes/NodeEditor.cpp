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
		DrawNodesAndLinks();
		SaveLoadGraph();
		ImNodes::MiniMap(0.2f, ImNodesMiniMapLocation_BottomRight);

		ImNodes::EndNodeEditor();

		CreateDeleteLinks();

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
				auto node = graph.AddNode<ObjectEditorNode<YourStruct>>("YourStruct");
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

	void NodeEditor::DrawNodesAndLinks() {
		for (const auto& pair : graph.nodes) {
			NodeBase& nd = *pair.second;
			nd.Draw();
		}
		//for (const auto& [id, link] : graph.links) {
		for (const auto& pair : graph.links) {
			const Link& lnk = pair.second;
			ImNodes::Link(lnk.id, lnk.startAttrId, lnk.endAttrId);
		}
	}

	void NodeEditor::SaveLoadGraph() {
		// TODO: fix save/load. It is just saving positions of a fixed graph
		const char* editorStateSaveFile{ "editor_state.ini" };
		ImGuiIO& io{ ImGui::GetIO() };
		if (io.KeyCtrl && ImGui::IsKeyPressed(83, false)) {
			ImNodes::SaveCurrentEditorStateToIniFile(editorStateSaveFile);
		}
		else if (io.KeyCtrl && ImGui::IsKeyPressed(76, false)) {
			ImNodes::LoadCurrentEditorStateFromIniFile(editorStateSaveFile);
		}
	}

	void NodeEditor::CreateDeleteLinks() {
		int linkStartId, linkEndId;
		if (ImNodes::IsLinkCreated(&linkStartId, &linkEndId))
			graph.AddLink(linkStartId, linkEndId);

		int linkId;
		if (ImNodes::IsLinkDestroyed(&linkId))
			graph.RemoveLink(linkId);
	}

	Graph NodeEditor::MakeTestGraph() {
		Graph graph{};
		auto nd1 = graph.AddNode<ObjectEditorNode<MyStruct>>("Node1");
		auto nd2 = graph.AddNode<ObjectEditorNode<YourStruct>>("Node2", 4, 3.14f, static_cast<VkColorComponentFlagBits>(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT));
		auto nd3 = graph.AddNode<ObjectViewerNode>();
		graph.AddLink(nd1->output.id, nd3->input.id);
		return graph;
	}
}