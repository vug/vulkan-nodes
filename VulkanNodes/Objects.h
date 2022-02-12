#pragma once

#include <vulkan/vulkan.h>

#include <type_traits>

namespace ne {

	namespace enums {
		static const char* VkImageLayoutLabels[] = { "Undefined", "Color Attachment Optimal", "Depth Stencil Attachment Optimal",
		"Shader Read-Only Optimal", "Transfer Source Optimal", "Transfer Destination Optimal", "Depth Attachment Optimal",
		"Present Source Khronos" };

		static int GetIndex(const VkImageLayout& val) {
			switch (val) {
			case VK_IMAGE_LAYOUT_UNDEFINED: return 0;
			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: return 1;
			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: return 2;
			case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL: return 3;
			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: return 4;
			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: return 5;
			case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL: return 6;
			case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: return 7;
			}
			// TODO: assert
			return 0;
		}

		static VkImageLayout IndexToVkImageLayout(int index) {
			switch (index) {
			case 0: return VK_IMAGE_LAYOUT_UNDEFINED;
			case 1: return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			case 2: return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			case 3: return VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL;
			case 4: return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			case 5: return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			case 6: return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
			case 7: return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			}
			// TODO: assert
			return VK_IMAGE_LAYOUT_UNDEFINED;
		}

		template <typename TVkEnum>
		const char** GetLabels() {
			if constexpr (std::is_same_v<TVkEnum, VkImageLayout>)
				return VkImageLayoutLabels;
		}

		template <typename TVkEnum>
		int GetNumItems() {
			if constexpr (std::is_same_v<TVkEnum, VkImageLayout>)
				return sizeof(VkImageLayoutLabels) / sizeof(const char*);
		}

		template <typename TVkEnum>
		TVkEnum GetValue(int index) {
			if constexpr (std::is_same_v<TVkEnum, VkImageLayout>)
				return IndexToVkImageLayout(index);
		}

		template <typename TVkEnum>
		const char* GetLabel(const TVkEnum& val) {
			return GetLabels<TVkEnum>()[GetIndex(val)];
		}
	};

	struct MyStruct {
		int count;
		float magnitude;
		VkImageLayout imageLayout;
	};

	enum class YourEnum {
		Opt1,
		Opt2,
	};

	struct YourStruct {
		YourEnum option;
		int num;
	};
}