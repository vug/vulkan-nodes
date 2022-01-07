#pragma once

#include <string>
#include <vector>
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
