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

		static std::unordered_map<VkFormat, const char*> VkFormatOpDict = {
			{VK_FORMAT_UNDEFINED, "Undefined"},
			{VK_FORMAT_D24_UNORM_S8_UINT, "D24 Unorm S8 Uint"},
			{VK_FORMAT_R32G32B32_SFLOAT, "R32G32B32 Sfloat"},
			{VK_FORMAT_B8G8R8A8_SRGB, "B8G8R8A8 Srgb"},
			{VK_FORMAT_R8G8B8A8_UNORM, "R8G8B8A8 Unorm"},
			{VK_FORMAT_R8G8B8A8_SRGB, "R8G8B8A8 Srgb"},
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

		static std::unordered_map<VkSampleCountFlagBits, const char*> VkSampleCountDict = {
			{VK_SAMPLE_COUNT_1_BIT, "1"},
			{VK_SAMPLE_COUNT_2_BIT, "2"},
			{VK_SAMPLE_COUNT_4_BIT, "4"},
			{VK_SAMPLE_COUNT_8_BIT, "8"},
			{VK_SAMPLE_COUNT_16_BIT, "16"},
			{VK_SAMPLE_COUNT_32_BIT, "32"},
			{VK_SAMPLE_COUNT_64_BIT, "64"},
		};

		static std::unordered_map<VkColorComponentFlagBits, const char*> VkColorComponentDict = {
			{VK_COLOR_COMPONENT_R_BIT, "R"},
			{VK_COLOR_COMPONENT_G_BIT, "G"},
			{VK_COLOR_COMPONENT_B_BIT, "B"},
			{VK_COLOR_COMPONENT_A_BIT, "A"},
		};

		template <typename TVkEnum>
		const std::unordered_map<TVkEnum, const char*>& GetDict() {
			if constexpr (std::is_same_v<TVkEnum, VkAttachmentLoadOp>)
				return VkAttachmentLoadOpDict;
			else if constexpr (std::is_same_v<TVkEnum, VkAttachmentStoreOp>)
				return VkAttachmentStoreOpDict;
			else if constexpr (std::is_same_v<TVkEnum, VkFormat>)
				return VkFormatOpDict;
			else if constexpr (std::is_same_v<TVkEnum, VkImageLayout>)
				return VkImageLayoutDict;

			else if constexpr (std::is_same_v<TVkEnum, VkSampleCountFlagBits>)
				return VkSampleCountDict;
			else if constexpr (std::is_same_v<TVkEnum, VkColorComponentFlagBits>)
				return VkColorComponentDict;
		}

		template <typename TVkEnum>
		const char* GetEnumLabel(const TVkEnum& val) {
			return GetDict<TVkEnum>().at(val);
		}

		template <typename TVkFlag>
		std::string GetFlagLabel(const TVkFlag& val) {
			std::string label;
			auto& dict = enums::GetDict<TVkFlag>();
			for (auto& [opVal, opLabel] : dict) {
				if ((opVal & val) == opVal) {
					label += opLabel;
					label += ",";
				}
			}
			return label;
		}
	};

	struct MyStruct {
		int count;
		float magnitude;
		VkFormat format;
		VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
		VkAttachmentLoadOp loadOp;
		VkAttachmentStoreOp storeOp;
		VkImageLayout initialLayout;
	};

	enum class YourEnum {
		Opt1,
		Opt2,
	};

	struct YourStruct {
		YourEnum option;
		int num;
		VkColorComponentFlagBits colorComponents;
	};
}