#include "Attributes.h"

#include "dependencies/imnodes.h"

namespace ne {
	bool ValueAttribute::Drawer::operator()(float& val) {
		return ImGui::DragFloat("##hidelabel", &val, 0.01f);
	}

	bool ValueAttribute::Drawer::operator()(int& val) {
		return ImGui::DragInt("##hidelabel", &val);
	}

	// -------

	bool ValueAttribute::Draw() const {
		return std::visit(Drawer{}, value);
	}
}