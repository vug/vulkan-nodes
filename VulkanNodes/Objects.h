#pragma once

#include <vulkan/vulkan.h>

namespace ne {
	static const char* VkImageLayoutLabels[] = { "Undefined", "Color Attachment Optimal", "Depth Stencil Attachment Optimal", 
		"Shader Read-Only Optimal", "Transfer Source Optimal", "Transfer Destination Optimal", "Depth Attachment Optimal", 
		"Present Source Khronos" };
	int VkImageLayoutToIndex(const VkImageLayout& val);
	VkImageLayout IndextoVkImageLayout(int index);

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