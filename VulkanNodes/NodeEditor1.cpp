#include "NodeEditor1.h"

#include <cassert>

namespace ne1 {

	bool AttributeDrawer::operator()(float& val) {
		return ImGui::DragFloat("##hidelabel", &val, 0.01f);
	}

	bool AttributeDrawer::operator()(int& val) {
		return ImGui::DragInt("##hidelabel", &val);
	}

	bool AttributeDrawer::operator()(MyStruct& obj) {
		ImGui::Text("MyStruct");
		ImGui::Text("magnitude: %f", obj.magnitude);
		ImGui::Text("count: %d", obj.count);
		return false;
	}

	// -------------

	Node::Node(std::string title, Object obj)
		: NodeBase{ -1, title }, object{ obj }, output{ -1, "output", obj } {
		std::visit(adder, obj);
	}

	void Node::Draw() {
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

	void Node::InputAddingVisitor::operator()(MyStruct& ms) {
		node.inputs.emplace_back(-1, "magnitude", ms.magnitude);
		node.inputs.emplace_back(-1, "count", ms.count);
		node.output = ObjectAttribute{ -1, "MyStruct", ms };
	}

	void Node::InputAddingVisitor::operator()(int& n) {
		node.inputs.emplace_back(-1, "int", n);
	}

	void Node::InputAddingVisitor::operator()(float& x) {
		node.inputs.emplace_back(-1, "float", x);
	}

	void ObjectViewerNode::Draw() {
		if (!input)
			return;

		ImNodes::BeginInputAttribute(input->id);
		const float labelWidth{ ImGui::CalcTextSize(input->name.c_str()).x };
		ImGui::TextUnformatted(input->name.c_str());

		ImGui::SameLine();
		ImGui::PushItemWidth(nodeWidth - labelWidth);
		ImGui::Text("input");
		ImGui::PopItemWidth();
		ImNodes::EndOutputAttribute();

		std::visit(AttributeDrawer{}, (*input).object);
	}

	// -------------

	NodeEditor::NodeEditor()
		: context{ ImNodes::EditorContextCreate() } {

		ImNodes::EditorContextSet(context);
		ImNodes::PushAttributeFlag(ImNodesAttributeFlags_EnableLinkDetachWithDragClick);
		ImNodesIO& io{ ImNodes::GetIO() };
		io.LinkDetachWithModifierClick.Modifier = &ImGui::GetIO().KeyCtrl;

		auto nd1{ std::make_shared<Node>("MyStruct1", ms1) };
		graph.AddNode(nd1);
		auto nd2{ std::make_shared<Node>("MyStruct2", ms2) };
		graph.AddNode(nd2);
		auto nd3{ std::make_shared<Node>("MyNumber", myNum) };
		graph.AddNode(nd3);
		auto nd4{ std::make_shared<ObjectViewerNode>() };
		nd4->input = std::make_shared<ObjectAttribute>(-1, nd1->output.name, nd1->output.object);
		graph.AddNode(nd4);

		graph.links.emplace_back(++graph.counter, nd1->output.id, nd2->inputs[1].id);
		graph.links.emplace_back(++graph.counter, nd3->output.id, nd1->inputs[1].id);
		// TODO: call this when link created via UI. and set input to nullptr when link removed.
		graph.links.emplace_back(++graph.counter, nd1->output.id, nd4->input->id);
	}

	void NodeEditor::Draw() {
		ImGui::Begin("Node Editor");
		ImGui::TextUnformatted("CTRL+s saves node positions. CTRL+l loads them.");
		ImNodes::BeginNodeEditor();

		for (auto nodePtr : graph.nodes) {
			auto& node{ *nodePtr };
			ImNodes::BeginNode(node.id);

			ImNodes::BeginNodeTitleBar();
			ImGui::TextUnformatted(node.title.c_str());
			ImNodes::EndNodeTitleBar();

			node.Draw();
			ImNodes::EndNode();
		}

		for (const auto& link : graph.links) {
			ImNodes::Link(link.id, link.start_attr, link.end_attr);
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
				graph.links.emplace_back(++graph.counter, linkStartId, linkEndId);
			}
		}

		// Link Deletion
		{
			int linkId;
			if (ImNodes::IsLinkDestroyed(&linkId)) {
				auto iter{
					std::find_if(graph.links.begin(), graph.links.end(), [linkId](const Link& link) -> bool {
						return link.id == linkId;
					})
				};
				assert(iter != graph.links.end());
				graph.links.erase(iter);
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