#pragma once

#include "Objects.h"

#include <vulkan/vulkan.h>
#include "dependencies/imnodes.h"

#include <optional>
#include <string>
#include <variant>

namespace ne {
	// Cannot have a Value& in Attribute but Value can be made of references!
	using ValueRef = std::variant<
		std::reference_wrapper<int>, std::reference_wrapper<float>, std::reference_wrapper<YourEnum>, std::reference_wrapper<VkFormat>,
		std::reference_wrapper<VkAttachmentLoadOp>, std::reference_wrapper<VkAttachmentStoreOp>, std::reference_wrapper<VkImageLayout>
	>;

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

		template <typename TVkEnum>
		static bool DrawVkEnum(TVkEnum& val) {
			const char* previewVal = enums::GetLabel(val);
			bool wasUsed = false;
			if (ImGui::BeginCombo("##hidelabel", previewVal, ImGuiComboFlags_PopupAlignLeft | ImGuiComboFlags_HeightLarge)) {
				for (auto& [opVal, opLabel] : enums::GetDict<TVkEnum>()) {
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

		// Visitor that draws UI for ValueAttribute editing
		struct Drawer {
			bool operator()(float& val);
			bool operator()(int& val);
			bool operator()(YourEnum& val);
			bool operator()(VkFormat& val) {
				return DrawVkEnum(val);
			}
			bool operator()(VkAttachmentLoadOp& val) {
				return DrawVkEnum(val);
			}
			bool operator()(VkAttachmentStoreOp& val) {
				return DrawVkEnum(val);
			}
			bool operator()(VkImageLayout& val) {
				return DrawVkEnum(val);
			}
		};

		bool Draw() const override;
	};


	class ObjectOutputAttribute : public AttributeBase {
	public:
		ObjectRef object;

		ObjectOutputAttribute(ObjectRef obj)
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

