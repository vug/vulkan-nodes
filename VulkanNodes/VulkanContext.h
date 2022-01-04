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
	struct SwapchainData {
		std::vector<VkImage> images;
		std::vector<VkImageView> imageViews;
	};
	struct FramebufferAttachment {
		VkImage image = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		VkImageView imageView = VK_NULL_HANDLE;
	};
	struct Sync {
		std::vector<VkSemaphore> available_semaphores;
		std::vector<VkSemaphore> finished_semaphore;
		std::vector<VkFence> in_flight_fences;
		std::vector<VkFence> image_in_flight;
	};
public:
	const Window& win;
	vkb::Instance instance;
	VkSurfaceKHR surface = {};
	vkb::Device device;
	VkQueue graphics_queue;
	VkQueue present_queue;
	vkb::Swapchain swapchain;
	SwapchainData swapchainData;
	VkFormat surfaceDepthFormat;
	VkRenderPass surfaceRenderPass;
	FramebufferAttachment surfaceDepthAttachment;
	std::vector<VkFramebuffer> surfaceFramebuffers;
	VkCommandPool commandPool;
	std::vector<VkCommandBuffer> commandBuffers;
	const int MAX_FRAMES_IN_FLIGHT = 2;
	Sync sync;
private:
	vkb::Instance InitInstance();
	VkSurfaceKHR InitSurface();
	vkb::Device InitDevice();
	VkRenderPass CreateSurfaceRenderPass();
	FramebufferAttachment CreateDepthAttachment();
	std::vector<VkCommandBuffer> CreateCommandBuffers();
public:
	std::vector<VkFramebuffer> CreateFramebuffers();
	VkCommandPool CreateCommandPool();
private:
	Sync InitSync();
};