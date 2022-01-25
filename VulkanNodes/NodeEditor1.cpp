#include "NodeEditor1.h"

#include <cassert>

namespace ne1 {

	bool AttributeDrawer::operator()(float& val) {
		return ImGui::DragFloat("##hidelabel", &val, 0.01f);
	}

	bool AttributeDrawer::operator()(int& val) {
		return ImGui::DragInt("##hidelabel", &val);
	}

	void ViewerNodeDrawer::operator()(float& val) {
		ImGui::Text("%f", val);
	}

	void ViewerNodeDrawer::operator()(int& val) {
		ImGui::Text("%d", &val);
	}

	void ViewerNodeDrawer::operator()(MyStruct& obj) {
		ImGui::Text("MyStruct");
		ImGui::Text("magnitude: %f", obj.magnitude);
		ImGui::Text("count: %d", obj.count);
	}

	// -------------

	ObjectEditorNode::ObjectEditorNode(std::string title, ObjectRef obj)
		: NodeBase{ -1, title }, object{ obj }, output{ -1, "output", obj } {
		std::visit(adder, obj);
	}

	void ObjectEditorNode::Draw() const {
		for (auto& attr : inputs) {
			ImNodes::BeginInputAttribute(attr.id);
			const float labelWidth{ ImGui::CalcTextSize(attr.name.c_str()).x };
			ImGui::TextUnformatted(attr.name.c_str());

			ImGui::SameLine();
			ImGui::PushItemWidth(nodeWidth - labelWidth);
			std::visit(AttributeDrawer{}, attr.value);
			ImGui::PopItemWidth();
			ImNodes::EndOutputAttribute();
		}

		{
			auto& attr{ output };
			ImNodes::BeginOutputAttribute(attr.id);
			const float labelWidth{ ImGui::CalcTextSize(attr.name.c_str()).x };
			ImGui::Indent(20);
			ImGui::TextUnformatted(attr.name.c_str());

			ImGui::SameLine();
			ImGui::PushItemWidth(nodeWidth - labelWidth - 20);
			ImGui::Text("output");
			ImGui::PopItemWidth();
			ImNodes::EndOutputAttribute();
		}
	}

	void ObjectEditorNode::InputAddingVisitor::operator()(MyStruct& ms) {
		node.inputs.emplace_back(-1, "magnitude", ms.magnitude);
		node.inputs.emplace_back(-1, "count", ms.count);
		node.output = ObjectOutputAttribute{ -1, "MyStruct", ms };
	}

	void ObjectEditorNode::InputAddingVisitor::operator()(int& n) {
		node.inputs.emplace_back(-1, "int", n);
	}

	void ObjectEditorNode::InputAddingVisitor::operator()(float& x) {
		node.inputs.emplace_back(-1, "float", x);
	}

	void ObjectViewerNode::Draw() const {
		ImNodes::BeginInputAttribute(input.id);
		const float labelWidth{ ImGui::CalcTextSize(input.name.c_str()).x };
		ImGui::TextUnformatted(input.name.c_str());
		ImNodes::EndOutputAttribute();

		if (input.optObject.has_value()) {
			std::visit(ViewerNodeDrawer{}, input.optObject.value());
		}
		else {
			ImGui::Text("no input");
		}
	}

	// -------------
	Graph NodeEditor::MakeTestGraph() {
		Graph graph{};
		auto nd1{ std::make_shared<ObjectEditorNode>("MyStruct1", ms1) };
		graph.AddNode(nd1);
		auto nd2{ std::make_shared<ObjectEditorNode>("MyStruct2", ms2) };
		graph.AddNode(nd2);
		auto nd3{ std::make_shared<ObjectEditorNode>("MyNumber", myNum) };
		graph.AddNode(nd3);
		auto nd4{ std::make_shared<ObjectViewerNode>() };
		graph.AddNode(nd4);

		graph.AddLink(nd1->output, nd2->inputs[1]);
		graph.AddLink(nd3->output, nd1->inputs[1]);
		graph.AddLink(nd1->output, nd4->input);
		return graph;
	}

	NodeEditor::NodeEditor(const Graph& graph) : graph{ graph } {
		ImNodes::EditorContextSet(context);
		ImNodes::PushAttributeFlag(ImNodesAttributeFlags_EnableLinkDetachWithDragClick);
		ImNodesIO& io{ ImNodes::GetIO() };
		io.LinkDetachWithModifierClick.Modifier = &ImGui::GetIO().KeyCtrl;
	}

	NodeEditor::NodeEditor() : NodeEditor{ MakeTestGraph() } {}

	void NodeEditor::Draw() {
		ImGui::Begin("Node Editor");
		ImGui::TextUnformatted("CTRL+s saves node positions. CTRL+l loads them.");
		ImNodes::BeginNodeEditor();

		for (const auto& nodePtr : graph.nodes) {
			const auto& node{ *nodePtr };
			ImNodes::BeginNode(node.id);

			ImNodes::BeginNodeTitleBar();
			ImGui::TextUnformatted(node.title.c_str());
			ImNodes::EndNodeTitleBar();

			node.Draw();
			ImNodes::EndNode();
		}

		for (const auto& [id, link] : graph.links) {
			ImNodes::Link(id, link.start_attr, link.end_attr);
		}

		const char* editorStateSaveFile{ "editor_state.ini" };
		ImGuiIO& io{ ImGui::GetIO() };
		if (io.KeyCtrl && ImGui::IsKeyPressed(83, false)) {
			ImNodes::SaveCurrentEditorStateToIniFile(editorStateSaveFile);
		}
		else if (io.KeyCtrl && ImGui::IsKeyPressed(76, false)) {
			ImNodes::LoadCurrentEditorStateFromIniFile(editorStateSaveFile);
		}

		ImNodes::MiniMap();
		ImNodes::MiniMap(0.2f, ImNodesMiniMapLocation_BottomRight);
		ImNodes::EndNodeEditor();

		// Link Creation
		{
			int linkStartId, linkEndId;
			if (ImNodes::IsLinkCreated(&linkStartId, &linkEndId)) {
				graph.AddLink(linkStartId, linkEndId);
			}
		}

		// Link Deletion
		{
			int linkId;
			if (ImNodes::IsLinkDestroyed(&linkId)) {
				graph.RemoveLink(linkId);
			}
		}

		// TODO: might not be needed
		int node_id;
		if (ImNodes::IsNodeHovered(&node_id)) {
			//nodeHovered = node_id;
		}

		// TODO: highlight selected nodes
		const int numSelectedNodes{ ImNodes::NumSelectedNodes() };
		if (numSelectedNodes > 0) {
			std::vector<int> selectedNodeIds;
			selectedNodeIds.resize(numSelectedNodes);
			ImNodes::GetSelectedNodes(selectedNodeIds.data());
		}

		ImGui::End();
	}
} // namespace ne1