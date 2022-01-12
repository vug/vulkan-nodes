#pragma once

#include <memory>
#include <string>
#include <vector>

#include "dependencies/imnodes.h"

namespace ne2 {

	struct MyStruct {
		int count;
		float magnitude;
	};

	class BaseAttribute {
	public:
		int id;
		std::string name;
		// T& val; // Derived classes will have a member "val" of any type that'll be manipulated via UI

		// draw UI to manipulate val of type T
		virtual bool Draw() = 0;
	protected:
		BaseAttribute(int id, std::string name) : id(id), name(name) {}
	};

	class FloatAttribute : public BaseAttribute {
	private:
		float& val;
	public:
		FloatAttribute() = delete;
		FloatAttribute(int id, std::string name, float& x) : BaseAttribute(id, name), val(x) {}

		virtual bool Draw() override;
	};

	class IntAttribute : public BaseAttribute {
	private:
		int& val;
	public:
		IntAttribute() = delete;
		IntAttribute(int id, std::string name, int& x) : BaseAttribute(id, name), val(x) {}

		virtual bool Draw() override;
	};

	struct NodeA {
		int id;
		std::string title;
		// cannot have reference to abstract base class, there for pointer needed
		std::vector<std::unique_ptr<BaseAttribute>> inputs;
		std::vector<std::unique_ptr<BaseAttribute>> outputs;
	};

	struct NodeB {
		int id;
		std::string title;
		// cannot have reference to abstract base class, there for pointer needed
		std::vector<std::shared_ptr<BaseAttribute>> inputs;
		std::vector<std::shared_ptr<BaseAttribute>> outputs;
	};

	struct GraphA {
		std::vector<NodeA> nodes;
		// links here
		int counter = 1000;
	};

	struct GraphB {
		std::vector<NodeB> nodes;
		// links here
		int counter = 2000;
	};

	class NodeEditor {
	public:
		NodeEditor();
		void Draw();
	public:
		GraphA graphA = {};
		GraphB graphB = {};

		// TODO: Store actual values to be edited somewhere else
		MyStruct ms = { 7, 8.5 };
		float f1 = 0.3f, f2 = 0.4f, f3 = 0.6f;
		int i1 = 5;
	private:
		ImNodesEditorContext* context = nullptr;
	};

} // namespace2