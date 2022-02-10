#pragma once

#include "Objects.h"

#include <optional>
#include <string>
#include <variant>

namespace ne {
	// Cannot have a Value& in Attribute but Value can be made of references!
	using ValueRef = std::variant<std::reference_wrapper<int>, std::reference_wrapper<float>, std::reference_wrapper<YourEnum>>;

	using ObjectRef = std::variant<std::reference_wrapper<MyStruct>, std::reference_wrapper<YourStruct>, std::reference_wrapper<int>, std::reference_wrapper<float>>;

	class AttributeBase {
	public:
		// Members required by ImNode
		int id{ -1 };
		std::string name;

		AttributeBase(std::string name) : name{ name } {}

		// Logic to draw Attribute UI in a Node body
		virtual bool Draw() const = 0;
	};

	class ValueAttribute : public AttributeBase {
	public:
		ValueRef value;

		ValueAttribute(std::string title, ValueRef value)
			: AttributeBase{ title }, value{ value } {}

		// Visitor that draws UI for ValueAttribute editing
		struct Drawer {
			bool operator()(float& val);
			bool operator()(int& val);
			bool operator()(YourEnum& val);
		};

		bool Draw() const override;
	};

	template <typename TObj>
	class ObjectOutputAttribute : public AttributeBase {
	public:
		TObj& object;

		ObjectOutputAttribute(TObj& obj)
			: AttributeBase{ "out" }, object{ obj } {}

		bool Draw() const {
			return false;
		}
	};

	class ObjectInputAttribute : public AttributeBase {
	public:
		std::optional<ObjectRef> optObject;

		ObjectInputAttribute() : AttributeBase{ "input" }, optObject{} {}
		ObjectInputAttribute(ObjectRef object) : AttributeBase{ "input" }, optObject{ object } {}

		bool Draw() const {
			return false;
		}
	};
}
