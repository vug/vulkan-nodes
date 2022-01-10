#include "NodeEditor.h"

NodeEditor::NodeEditor()
	: context(ImNodes::EditorContextCreate()) {

	ImNodes::EditorContextSet(context);
	//ImNodes::PushAttributeFlag(ImNodesAttributeFlags_EnableLinkDetachWithDragClick);
	//ImNodesIO& io = ImNodes::GetIO();
	//io.LinkDetachWithModifierClick.Modifier = &ImGui::GetIO().KeyCtrl;

	graph.AddNode({ "MyStruct1", ms1 });
	graph.AddNode({ "MyStruct2", ms2 });
	graph.links.emplace_back(++graph.counter, graph.nodes[0].outputs[0].id, graph.nodes[1].inputs[1].id);
	graph.AddNode({ "MyNumber", myNum });
	graph.links.emplace_back(++graph.counter, graph.nodes[2].outputs[0].id, graph.nodes[0].inputs[1].id);
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

bool AttributeDrawer::operator()(float& val) {
	return ImGui::DragFloat("##hidelabel", &val, 0.01f);
}

bool AttributeDrawer::operator()(int& val) {
	return ImGui::DragInt("##hidelabel", &val);
}

Node::Node(std::string title, Object obj)
	: title(title), object(obj) {
	std::visit(adder, obj);
}

void Node::InputAddingVisitor::operator()(MyStruct& ms) {
	node.inputs.emplace_back(-1, "magnitude", ms.magnitude);
	node.inputs.emplace_back(-1, "count", ms.count);
	node.outputs.emplace_back(-1, "out", outVal);
}

void Node::InputAddingVisitor::operator()(int& n) {
	node.outputs.emplace_back(-1, "int", n);
}

void Node::InputAddingVisitor::operator()(float& x) {
	node.outputs.emplace_back(-1, "float", x);
}