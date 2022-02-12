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

	bool ValueAttribute::Drawer::operator()(VkAttachmentLoadOp& val) {
		const char* previewVal = enums::VkAttachmentLoadOpDict[val];
		bool wasUsed = false;
		if (ImGui::BeginCombo("##hidelabel", previewVal, ImGuiComboFlags_PopupAlignLeft | ImGuiComboFlags_HeightLarge)) {
			for (auto& [opVal, opLabel] : enums::VkAttachmentLoadOpDict) {
				const bool is_selected = (val == opVal);
				if (ImGui::Selectable(opLabel, is_selected)) {
					val = opVal;
					wasUsed = true;
				}

				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		return wasUsed;
	}

	bool ValueAttribute::Drawer::operator()(VkAttachmentStoreOp& val) {
		const char* previewVal = enums::VkAttachmentStoreOpDict[val];
		bool wasUsed = false;
		if (ImGui::BeginCombo("##hidelabel", previewVal, ImGuiComboFlags_PopupAlignLeft | ImGuiComboFlags_HeightLarge)) {
			for (auto& [opVal, opLabel] : enums::VkAttachmentStoreOpDict) {
				const bool is_selected = (val == opVal);
				if (ImGui::Selectable(opLabel, is_selected)) {
					val = opVal;
					wasUsed = true;
				}

				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		return wasUsed;
	}

	bool ValueAttribute::Drawer::operator()(VkImageLayout& val) {
        int index = enums::GetIndex(val);
        bool wasUsed = false;
        if (wasUsed = ImGui::Combo("##hidelabel", &index, enums::GetLabels<VkImageLayout>(), enums::GetNumItems<VkImageLayout>())) {
			val = enums::GetValue<VkImageLayout>(index);
        }
        return wasUsed;
	}

	// -------

	bool ValueAttribute::Draw() const {
		return std::visit(Drawer{}, value);
	}
}