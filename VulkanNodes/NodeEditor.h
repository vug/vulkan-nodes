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
		// Since attributes are owned by nodes, graph only have references to them
		std::unordered_map<int, std::reference_wrapper<AttributeBase>> attributes;
		int counter{};

		void AddNode(std::shared_ptr<NodeBase> nd) {
			assert(nd->id != -1); // node should be given an id
			for (auto attrRef : nd->GetAllAttributes()) {
				assert(attrRef.get().id != -1);  // all attributes of a node should be given an id
				attributes.insert(std::make_pair(attrRef.get().id, attrRef));
			}
			nodes[nd->id] = nd;				
		}

		template<IsNode TNode, typename... Args>
		std::shared_ptr<TNode> AddNode(Args... args) {
			std::shared_ptr<TNode> nd = std::make_shared<TNode>(args...);
			nd->id = counter++;
			nodes[nd->id] = nd;

			for (auto attrRef : nd->GetAllAttributes()) {
				attrRef.get().id = counter++;
				attributes.insert(std::make_pair(attrRef.get().id, attrRef));
			}
			return nd;
		}

		// TODO: RemoveNode()

		Link& AddLink(int startAttrId, int endAttrId) {
			assert(attributes.contains(startAttrId));
			assert(attributes.contains(endAttrId));

			return AddLink(attributes.at(startAttrId), attributes.at(endAttrId));
		}

		Link& AddLink(AttributeBase& attr1, AttributeBase& attr2) {
			AttributeBase* pBase1 = &attr1;
			AttributeBase* pBase2 = &attr2;
			if (ObjectInputAttribute* attrIn = dynamic_cast<ObjectInputAttribute*>(pBase2)) {
				if (ObjectOutputAttribute* attrOut = dynamic_cast<ObjectOutputAttribute*>(pBase1)) {

					// does input attr already has connection?
					std::vector<int> linksToDelete;
					for (const auto& [linkId, link] : links) {
						// if yes, mark input's link for deletion, and reset view reference
						if (link.endAttrId == attrIn->id) {
							linksToDelete.push_back(linkId);
							attrIn->optObject.reset();
						}
					}
					for (int linkId : linksToDelete)
						links.erase(linkId);
					// new view reference
					attrIn->optObject = attrOut->object;

					const int id = ++counter;
					links[id] = { id, attr1.id, attr2.id };
					return links[id];
				}
			}
			// don't link in any other cases
		}

		void RemoveLink(int id) {
			assert(links.contains(id)); // linkId to be destroyed should exist

			const auto& link = links.at(id);
			AttributeBase& in = attributes.at(link.endAttrId);
			AttributeBase* pIn = &in;
			ObjectInputAttribute* attrIn = dynamic_cast<ObjectInputAttribute*>(pIn);
			if (attrIn)
				attrIn->optObject.reset();
			links.erase(id);
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