#pragma once

#include "Attributes.h"
#include "Nodes.h"

#include "dependencies/imnodes.h"

#include <cassert>
#include <memory>
#include <string>
#include <vector>

namespace ne {
	class Graph {
	public:
		std::vector<std::shared_ptr<NodeBase>> nodes;
		int counter{};

		void AddNode(std::shared_ptr<NodeBase> nd) {
			assert(nd->id != -1); // node should be given an id
			for (std::reference_wrapper<int>& idRef : nd->GetAllAttributeIds())
				assert(idRef.get() != -1);  // all attributes of a node should be given an id
			nodes.push_back(nd);
		}

		template<IsNode TNode, typename... Args>
		std::shared_ptr<TNode> AddNode(Args... args) {
			std::shared_ptr<TNode> nd = std::make_shared<TNode>(args...);
			nd->id = counter++;
			nodes.push_back(nd);

			for (auto& idRef : nd->GetAllAttributeIds())
				idRef.get() = counter++;
			return nd;
		}
	};

	// ----------------

	class NodeEditor {
	public:
		NodeEditor();
		NodeEditor(const Graph& graph);
		void Draw();

		static Graph MakeTestGraph();
	public:
		Graph graph{};

	private:
		ImNodesEditorContext* context = ImNodes::EditorContextCreate();

		void DrawPopupMenu();
	};
}