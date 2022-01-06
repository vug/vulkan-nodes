#pragma once

#include "VulkanContext.h"

class ImGuiHelper {
public:
	ImGuiHelper(const VulkanContext& vc);
	~ImGuiHelper();
	void Begin();
	void End();

	void AddDrawCalls(const VkCommandBuffer& cmdBuf);
private:
	const VulkanContext& vc;
	VkDescriptorPool imguiPool;
};