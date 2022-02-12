#pragma once

#include <vulkan/vulkan.h>

#include <type_traits>
#include <unordered_map>

namespace ne {

	namespace enums {
		static std::unordered_map<VkAttachmentLoadOp, const char*> VkAttachmentLoadOpDict = {
			{VK_ATTACHMENT_LOAD_OP_LOAD, "Load"},
			{VK_ATTACHMENT_LOAD_OP_CLEAR, "Clear"},
			{VK_ATTACHMENT_LOAD_OP_DONT_CARE, "Don't Care"},
			{VK_ATTACHMENT_LOAD_OP_NONE_EXT, "None Ext"},
		};

		static std::unordered_map<VkAttachmentStoreOp, const char*> VkAttachmentStoreOpDict = {
			{VK_ATTACHMENT_STORE_OP_STORE, "Store"},
			{VK_ATTACHMENT_STORE_OP_DONT_CARE, "Don't Care"},
			{VK_ATTACHMENT_STORE_OP_NONE_EXT, "None Ext"},
		};

		static std::unordered_map<VkImageLayout, const char*> VkImageLayoutDict = {
			{VK_IMAGE_LAYOUT_UNDEFINED, "Undefined"},
			{VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, "Color Attachment Optimal"},
			{VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, "Depth Stencil Attachment Optimal"},
			{VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL, "Shader Read-Only Optimal"},
			{VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, "Transfer Source Optimal"},
			{VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, "Transfer Destination Optimal"},
			{VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, "Depth Attachment Optimal"},
			{VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, "Present Source Khronos"},
		};

		template <typename TVkEnum>
		const std::unordered_map<TVkEnum, const char*>& GetDict() {
			if constexpr (std::is_same_v<TVkEnum, VkAttachmentLoadOp>)
				return VkAttachmentLoadOpDict;
			else if constexpr (std::is_same_v<TVkEnum, VkAttachmentStoreOp>)
				return VkAttachmentStoreOpDict;
			else if constexpr (std::is_same_v<TVkEnum, VkImageLayout>)
				return VkImageLayoutDict;
		}

		template <typename TVkEnum>
		const char* GetLabel(const TVkEnum& val) {
			return GetDict<TVkEnum>().at(val);
		}
	};

	struct MyStruct {
		int count;
		float magnitude;
		VkAttachmentLoadOp attachmentLoadOp;
		VkAttachmentStoreOp attachmentStoreOp;
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