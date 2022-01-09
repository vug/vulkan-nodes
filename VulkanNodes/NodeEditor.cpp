#include "NodeEditor.h"

NodeEditor::NodeEditor()
	: context(ImNodes::EditorContextCreate()) {

	ImNodes::EditorContextSet(context);
	//ImNodes::PushAttributeFlag(ImNodesAttributeFlags_EnableLinkDetachWithDragClick);
	//ImNodesIO& io = ImNodes::GetIO();
	//io.LinkDetachWithModifierClick.Modifier = &ImGui::GetIO().KeyCtrl;

	graph1.nodes.push_back({
		++graph1.counter,
		"G1 node1 (MyStruct)",
		{{++graph1.counter, "magnitude", ms.magnitude}, {++graph1.counter, "count", ms.count}},
		{{++graph1.counter, "output1", f1}},
		1.0f
	});
	graph1.nodes.push_back({
		++graph1.counter,
		"G1 node2",
		{{++graph1.counter, "input1", f2}, {++graph1.counter, "input2", i1}},
		{{++graph1.counter, "output1", f3}},
		1.0f
	});
	graph1.links = { {++graph1.counter, graph1.nodes[0].outputs[0].id, graph1.nodes[1].inputs[0].id} };

	graph2.nodes.push_back({
		++graph2.counter,
		"G2 node1 (MyStruct)",
		{
			std::make_shared<FloatAttribute>(++graph2.counter, "magnitude", ms.magnitude),
			std::make_shared<IntAttribute>(++graph2.counter, "count", ms.count),
		}
	});
}

void NodeEditor::Draw() {
	ImGui::Begin("Node Editor");
	ImGui::TextUnformatted("CTRL+s saves node positions. CTRL+l loads them.");
	ImNodes::BeginNodeEditor();

	for (auto& node : graph1.nodes) {
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
			std::visit(drawer, attr.value);
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
			std::visit(drawer, attr.value);
			ImGui::PopItemWidth();
			ImNodes::EndOutputAttribute();
		}
		ImNodes::EndNode();
	}

	for (const auto& link : graph1.links) {
		ImNodes::Link(link.id, link.start_attr, link.end_attr);
	}

	for (auto& node : graph2.nodes) {
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

bool AttributeDrawer::operator()(float& val) {
	return ImGui::DragFloat("##hidelabel", &val, 0.01f);
}

bool AttributeDrawer::operator()(int& val) {
	return ImGui::DragInt("##hidelabel", &val);
}

bool FloatAttribute::Draw() {
	return ImGui::DragFloat("##hidelabel", &val, 0.01f);
}

bool IntAttribute::Draw() {
	return ImGui::DragInt("##hidelabel", &val);
}