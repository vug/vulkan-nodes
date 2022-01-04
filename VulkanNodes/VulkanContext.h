#pragma once

#include "Window.h"

#include <vulkan/vulkan.h>
#include <VkBootstrap.h>

#include <iostream>

namespace vkb {
	namespace detail {
		// TODO: maybe add an optional error message prefix
		template <typename T>
		T GetResult(const Result<T>& res) {
			if (!res) {
				std::cout << "CRITICAL: " << res.error().message() << "\n";
				exit(EXIT_FAILURE);
			}
			return res.value();
		}
	}
}

class VulkanContext {
public:
	VulkanContext(const Window& win);
	~VulkanContext();

	void RecreateSwapchain();
public:
	struct SurfaceInfo {
		VkRenderPass renderPass;
		VkFormat depthFormat = VK_FORMAT_D24_UNORM_S8_UINT;
		VkImage depthImage;
		VkDeviceMemory depthMemory;
		VkImageView depthImageView;
	};
public:
	const Window& win;
	vkb::Instance instance;
	VkSurfaceKHR surface = {};
	vkb::Device device;
	vkb::Swapchain swapchain;
	SurfaceInfo surfaceInfo;
private:
	vkb::Instance InitInstance();
	VkSurfaceKHR InitSurface();
	vkb::Device InitDevice();
	vkb::Swapchain InitSwapchain();
	SurfaceInfo InitSurfaceInfo();
	VkRenderPass CreateSurfaceRenderPass();
};