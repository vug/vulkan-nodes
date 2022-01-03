#pragma once

#include <vulkan/vulkan.h>
#include <VkBootstrap.h>

class VulkanContext {
public:
	vkb::Instance instance;
	VkSurfaceKHR surface = {};
	vkb::Device device;
	vkb::Swapchain swapchain;
};