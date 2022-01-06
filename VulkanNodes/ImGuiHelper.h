#pragma once

#include "VulkanContext.h"

class ImGuiHelper {
public:
	ImGuiHelper(const VulkanContext& vc);
	~ImGuiHelper();
	void Begin();
	void End();

	void OnRender();
private:
	const VulkanContext& vc;
	VkDescriptorPool imguiPool;
};