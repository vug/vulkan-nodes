#pragma once

#include "VulkanContext.h"

class ImGuiHelper {
public:
	ImGuiHelper(const VulkanContext& init);
	~ImGuiHelper();
	void Begin();
	void End();

	void OnRender();
private:
	const VulkanContext& init;
	VkDescriptorPool imguiPool;
};