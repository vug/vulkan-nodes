#pragma once

#include "Attributes.h"
#include "Nodes.h"

#include "dependencies/imnodes.h"

#include <cassert>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

namespace ne {
	struct Link {
		int id;
		int startAttrId;
		int endAttrId;
	};

	class Graph {
	public:
		// Graph owns nodes and links
		std::unordered_map<int, std::shared_ptr<NodeBase>> nodes;
		std::unordered_map<int, Link> links;
		int counter{};

		void AddNode(std::shared_ptr<NodeBase> nd) {
			assert(nd->id != -1); // node should be given an id
			for (std::reference_wrapper<AttributeBase>& attrRef : nd->GetAllAttributes())
				assert(attrRef.get().id != -1);  // all attributes of a node should be given an id
			nodes[nd->id] = nd;
		}

		template<IsNode TNode, typename... Args>
		std::shared_ptr<TNode> AddNode(Args... args) {
			std::shared_ptr<TNode> nd = std::make_shared<TNode>(args...);
			nd->id = counter++;
			nodes[nd->id] = nd;

			for (auto attrRef : nd->GetAllAttributes())
				attrRef.get().id = counter++;
			return nd;
		}

		// TODO: RemoveNode()

		Link& AddLink(int startAttrId, int endAttrId) {
			const int id = counter++;
			auto pair = links.emplace(id, Link { id, startAttrId, endAttrId });
			Link& link = pair.first->second;
			return link;
			// TODO: handle linking from ObjectOutputAttribute to ObjectInputAttribute.
		}

		void RemoveLink(int id) {
			if(links.contains(id))
				links.erase(id);
			//auto erased = std::erase_if(links, [&](Link& lnk) { return lnk.id == id; });
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