#include "Attributes.h"

#include "dependencies/imnodes.h"

namespace ne {
	bool ValueAttribute::Drawer::operator()(float& val) {
		return ImGui::DragFloat("##hidelabel", &val, 0.01f);
	}

	bool ValueAttribute::Drawer::operator()(int& val) {
		return ImGui::DragInt("##hidelabel", &val);
	}

	bool ValueAttribute::Drawer::operator()(YourEnum& val) {
		const char* items[] = { "Opt1", "Opt2" };
		int choice = static_cast<int>(val);
		bool hasModified = ImGui::Combo("##hidelabel", &choice, items, IM_ARRAYSIZE(items));
		val = static_cast<YourEnum>(choice);
		return hasModified;
	}

	// -------

	bool ValueAttribute::Draw() const {
		return std::visit(Drawer{}, value);
	}
}