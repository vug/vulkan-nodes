#pragma once

#include "VulkanContext.h"

class ImGuiHelper {
public:
	ImGuiHelper(const VulkanContext& vc);
	~ImGuiHelper();
	void Begin() const;
	void End() const;

	void AddDrawCalls(const VkCommandBuffer& cmdBuf) const;
private:
	const VulkanContext& vc;
	VkDescriptorPool imguiPool;
};