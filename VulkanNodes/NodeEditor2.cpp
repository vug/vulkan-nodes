#include "NodeEditor2.h"

namespace ne2 {

	NodeEditor::NodeEditor()
		: context(ImNodes::EditorContextCreate()) {

		ImNodes::EditorContextSet(context);
		//ImNodes::PushAttributeFlag(ImNodesAttributeFlags_EnableLinkDetachWithDragClick);
		//ImNodesIO& io = ImNodes::GetIO();
		//io.LinkDetachWithModifierClick.Modifier = &ImGui::GetIO().KeyCtrl;

		NodeA nd;
		nd.id = ++graphA.counter;
		nd.title = "G2a node1 (MyStruct)";
		nd.inputs.push_back(std::make_unique<FloatAttribute>(++graphA.counter, "magnitude", ms.magnitude));
		nd.inputs.push_back(std::make_unique<IntAttribute>(++graphA.counter, "count", ms.count));
		graphA.nodes.push_back(std::move(nd));

		graphB.nodes.push_back({
			++graphA.counter,
			"G2b node1 (MyStruct)",
			{
				std::make_shared<FloatAttribute>(++graphA.counter, "magnitude", ms.magnitude),
				std::make_shared<IntAttribute>(++graphA.counter, "count", ms.count),
			}
		});
	}

	void NodeEditor::Draw() {
		ImGui::Begin("Node Editor");
		ImGui::TextUnformatted("CTRL+s saves node positions. CTRL+l loads them.");
		ImNodes::BeginNodeEditor();

		// Wish this could have worked...
		//std::vector<std::variant<NodeA, NodeB>> nodes;
		//nodes.insert(nodes.end(), graphA.nodes.begin(), graphA.nodes.end());
		//nodes.insert(nodes.end(), graphB.nodes.begin(), graphB.nodes.end());

		for (auto& node : graphA.nodes) {
			const float nodeWidth = 100;
			ImNodes::BeginNode(node.id);

			ImNodes::BeginNodeTitleBar();
			ImGui::TextUnformatted(node.title.c_str());
			ImNodes::EndNodeTitleBar();

			for (auto& attr : node.inputs) {
				const float nodeWidth = 100;
				ImNodes::BeginInputAttribute(attr->id);
				const float labelWidth = ImGui::CalcTextSize(attr->name.c_str()).x;
				ImGui::TextUnformatted(attr->name.c_str());

				ImGui::SameLine();
				ImGui::PushItemWidth(nodeWidth - labelWidth);
				attr->Draw();
				ImGui::PopItemWidth();
				ImNodes::EndOutputAttribute();
			}
			ImNodes::EndNode();
		}

		for (auto& node : graphB.nodes) {
			const float nodeWidth = 100;
			ImNodes::BeginNode(node.id);

			ImNodes::BeginNodeTitleBar();
			ImGui::TextUnformatted(node.title.c_str());
			ImNodes::EndNodeTitleBar();

			for (auto& attr : node.inputs) {
				const float nodeWidth = 100;
				ImNodes::BeginInputAttribute(attr->id);
				const float labelWidth = ImGui::CalcTextSize(attr->name.c_str()).x;
				ImGui::TextUnformatted(attr->name.c_str());

				ImGui::SameLine();
				ImGui::PushItemWidth(nodeWidth - labelWidth);
				attr->Draw();
				ImGui::PopItemWidth();
				ImNodes::EndOutputAttribute();
			}
			ImNodes::EndNode();
		}


		const char* editorStateSaveFile = "editor_state.ini";
		ImGuiIO& io = ImGui::GetIO();
		if (io.KeyCtrl && ImGui::IsKeyPressed(83, false)) {
			ImNodes::SaveCurrentEditorStateToIniFile(editorStateSaveFile);
		}
		else if (io.KeyCtrl && ImGui::IsKeyPressed(76, false)) {
			ImNodes::LoadCurrentEditorStateFromIniFile(editorStateSaveFile);
		}

		ImNodes::EndNodeEditor();
		ImGui::End();
	}

	bool FloatAttribute::Draw() {
		return ImGui::DragFloat("##hidelabel", &val, 0.01f);
	}

	bool IntAttribute::Draw() {
		return ImGui::DragInt("##hidelabel", &val);
	}

} // namespace ne2