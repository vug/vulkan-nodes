#pragma once

#include "Attributes.h"
#include "Nodes.h"

#include "dependencies/imnodes.h"

#include <cassert>
#include <memory>
#include <string>
#include <vector>

namespace ne {
	struct Link {
		int id;
		int startAttrId;
		int endAttrId;
	};

	class Graph {
	public:
		std::vector<std::shared_ptr<NodeBase>> nodes;
		std::vector<Link> links;
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

		// TODO: RemoveNode()

		Link& AddLink(int startAttrId, int endAttrId) {
			return links.emplace_back(counter++, startAttrId, endAttrId);
			// TODO: handle linking from ObjectOutputAttribute to ObjectInputAttribute.
		}

		void RemoveLink(int id) {
			auto erased = std::erase_if(links, [&](Link& lnk) { return lnk.id == id; });
			// TODO: handle unlinking from ObjectOutputAttribute to ObjectInputAttribute.
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
		void DrawNodesAndLinks();
		void SaveLoadGraph();
		void CreateDeleteLinks();
	};
}