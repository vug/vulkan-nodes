#include "NodeEditor1.h"

#include <cassert>

namespace ne1 {

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

	void ViewerNodeDrawer::operator()(YourStruct& obj) {
		ImGui::Text("YourStruct");
		const char* items[] = { "Opt1", "Opt2" };
		ImGui::Text("option: %s", items[static_cast<int>(obj.option)]);
		ImGui::Text("num: %d", obj.num);
	}

	// -------------

	ObjectRefEditorNode::ObjectRefEditorNode(std::string title, ObjectRef obj)
		: NodeBase{ -1, title }, object{ obj }, output{ -1, "output", obj } {
		std::visit(adder, obj);
	}

	void ObjectRefEditorNode::Draw() const {
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

	void InputAddingVisitor::operator()(MyStruct& ms) {
		inputs.emplace_back(-1, "magnitude", ms.magnitude);
		inputs.emplace_back(-1, "count", ms.count);
		output = ObjectOutputAttribute{ -1, "MyStruct", ms };
	}

	void InputAddingVisitor::operator()(YourStruct& ys) {
		inputs.emplace_back(-1, "option", ys.option);
		inputs.emplace_back(-1, "num", ys.num);
		output = ObjectOutputAttribute{ -1, "YourStruct", ys };
	}

	void InputAddingVisitor::operator()(int& n) {
		inputs.emplace_back(-1, "int", n);
	}

	void InputAddingVisitor::operator()(float& x) {
		inputs.emplace_back(-1, "float", x);
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
		static MyStruct ms1{ 7, 8.5 };
		static MyStruct ms2{ 4, 1.5 };
		static int myNum{ 66 };

		Graph graph{};
		auto nd1{ std::make_shared<ObjectRefEditorNode>("MyStruct1", ms1) };
		graph.AddNode(nd1);
		auto nd2{ std::make_shared<ObjectRefEditorNode>("MyStruct2", ms2) };
		graph.AddNode(nd2);
		auto nd3{ std::make_shared<ObjectRefEditorNode>("MyNumber", myNum) };
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

	NodeEditor::NodeEditor() : NodeEditor{ Graph{} } {}

	void NodeEditor::Draw() {
		ImGui::Begin("Node Editor");
		ImGui::TextUnformatted("A: add node. CTRL+s: save node pos. CTRL+l: load node pos.");
		// Hack for learning key codes
		//for (int key = 0; key < 200; key++) { if (ImGui::IsKeyDown(key)) ImGui::Text("key: %d", key); }
		ImNodes::BeginNodeEditor();

		// Adding Nodes via UI
		{
			const bool shouldOpenPopup = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
				ImNodes::IsEditorHovered() &&
				ImGui::IsKeyReleased(65);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.f, 8.f));

			if (!ImGui::IsAnyItemHovered() && shouldOpenPopup)
				ImGui::OpenPopup("Add Object");

			if (ImGui::BeginPopup("Add Object")) {
				const ImVec2 clickPos = ImGui::GetMousePosOnOpeningCurrentPopup();

				if (ImGui::MenuItem("MyStruct")) {
					// TODO: this is horrible, memory-leaky way of letting nodes retain objects
					// instead let the Node own the struct instead of referring to it.
					MyStruct* ms = new MyStruct{};
					MyStruct& ms2 = *ms;
					auto node{ std::make_shared<ObjectRefEditorNode>("MyStruct", ms2) };
					graph.AddNode(node);
					ImNodes::SetNodeScreenSpacePos(node->id, clickPos);
				}
				if (ImGui::MenuItem("YourStruct")) {
					// TODO: this is horrible, memory-leaky way of letting nodes retain objects
					// instead let the Node own the struct instead of referring to it.
					YourStruct* pObj = new YourStruct{};
					YourStruct& obj = *pObj;
					auto node{ std::make_shared<ObjectRefEditorNode>("YourStruct", obj) };
					graph.AddNode(node);
					ImNodes::SetNodeScreenSpacePos(node->id, clickPos);
				}
				if (ImGui::MenuItem("Viewer")) {
					auto node{ std::make_shared<ObjectViewerNode>() };
					graph.AddNode(node);
					ImNodes::SetNodeScreenSpacePos(node->id, clickPos);
				}

				ImGui::EndPopup();
			}

			ImGui::PopStyleVar();
		}

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

		// TODO: fix save/load. It was just saving positions of a fixed graph
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