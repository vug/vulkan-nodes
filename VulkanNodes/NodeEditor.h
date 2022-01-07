#pragma once

#include <string>
#include <vector>

#include "dependencies/imnodes.h"

struct Attribute {
	int id;
	std::string name;
	float value;
};

struct Node {
	int id;
	std::string title;
	std::vector<Attribute> inputs;
	std::vector<Attribute> outputs;
	float value;
};

struct Link {
	int id;
	int start_attr, end_attr;
};

struct Graph {
	std::vector<Node> nodes;
	std::vector<Link> links;
	int counter = 0;
};

class NodeEditor {
public:
	NodeEditor();
	void Draw();
public:
	Graph graph = {};
private:
	ImNodesEditorContext* context = nullptr;
};