#include "NodeEditor.h"

NodeEditor::NodeEditor()
	: context(ImNodes::EditorContextCreate()) {

	ImNodes::EditorContextSet(context);
	//ImNodes::PushAttributeFlag(ImNodesAttributeFlags_EnableLinkDetachWithDragClick);
	//ImNodesIO& io = ImNodes::GetIO();
	//io.LinkDetachWithModifierClick.Modifier = &ImGui::GetIO().KeyCtrl;

	graph.nodes.push_back({
		++graph.counter,
		"node1",
		{{++graph.counter, "input1", 0.1f}, {++graph.counter, "input2", 0.2f}},
		{{++graph.counter, "output1", 0.3f}},
		1.0f
	});
	graph.nodes.push_back({
		++graph.counter,
		"node2",
		{{++graph.counter, "input1", 0.4f}, {++graph.counter, "input2", 0.5f}},
		{{++graph.counter, "output1", 0.6f}},
		1.0f
	});
	graph.links = { {++graph.counter, graph.nodes[0].outputs[0].id, graph.nodes[1].inputs[0].id} };
}

void NodeEditor::Draw() {
	ImGui::Begin("Node Editor");
	ImGui::TextUnformatted("CTRL+s saves node positions. CTRL+l loads them.");
	ImNodes::BeginNodeEditor();

	for (auto& node : graph.nodes) {
		const float nodeWidth = 100;
		ImNodes::BeginNode(node.id);

		ImNodes::BeginNodeTitleBar();
		ImGui::TextUnformatted(node.title.c_str());
		ImNodes::EndNodeTitleBar();

		for (auto& attr : node.inputs) {
			ImNodes::BeginInputAttribute(attr.id);
			const float labelWidth = ImGui::CalcTextSize(attr.name.c_str()).x;
			ImGui::TextUnformatted(attr.name.c_str());

			ImGui::SameLine();
			ImGui::PushItemWidth(nodeWidth - labelWidth);
			ImGui::DragFloat("##hidelabel", &attr.value, 0.01f);
			ImGui::PopItemWidth();
			ImNodes::EndOutputAttribute();
		}

		for (auto& attr : node.outputs) {
			ImNodes::BeginOutputAttribute(attr.id);
			const float labelWidth = ImGui::CalcTextSize(attr.name.c_str()).x;
			ImGui::Indent(20);
			ImGui::TextUnformatted(attr.name.c_str());

			ImGui::SameLine();
			ImGui::PushItemWidth(nodeWidth - labelWidth - 20);
			ImGui::DragFloat("##hidelabel", &attr.value, 0.01f);
			ImGui::PopItemWidth();
			ImNodes::EndOutputAttribute();
		}
		ImNodes::EndNode();
	}

	for (const auto& link : graph.links) {
		ImNodes::Link(link.id, link.start_attr, link.end_attr);
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
