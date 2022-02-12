#include "Objects.h"

namespace ne {
	int VkImageLayoutToIndex(const VkImageLayout& val) {
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
	VkImageLayout IndextoVkImageLayout(int index) {
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
}