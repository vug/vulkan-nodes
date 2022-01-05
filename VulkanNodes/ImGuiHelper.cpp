#include "ImGuiHelper.h"

#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>

#include <cassert>

ImGuiHelper::ImGuiHelper(const VulkanContext& init)
	: init(init) {

	VkDescriptorPoolSize pool_sizes[] = {
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	{
		VkDescriptorPoolCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		info.maxSets = 1000;
		info.poolSizeCount = std::size(pool_sizes);
		info.pPoolSizes = pool_sizes;

		assert(vkCreateDescriptorPool(init.device, &info, nullptr, &imguiPool) == VK_SUCCESS);
	}


	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForVulkan(init.win.GetGLFWWindow(), true);

	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = init.instance;
	init_info.PhysicalDevice = init.device.physical_device;
	init_info.Device = init.device;
	//init_info.QueueFamily = device.get_queue_index(vkb::QueueType::graphics).value();
	init_info.Queue = init.graphics_queue;
	//init_info.PipelineCache = g_PipelineCache;
	init_info.DescriptorPool = imguiPool;
	init_info.MinImageCount = static_cast<uint32_t>(init.surfaceFramebuffers.size());
	init_info.ImageCount = static_cast<uint32_t>(init.surfaceFramebuffers.size());
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	//init_info.CheckVkResultFn = check_vk_result;
	ImGui_ImplVulkan_Init(&init_info, init.surfaceRenderPass);

	// Upload fonts
	{
		VkCommandPool command_pool = init.commandPool;
		VkCommandBuffer command_buffer = init.commandBuffer;

		vkResetCommandPool(init.device, command_pool, 0);
		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(command_buffer, &begin_info);

		ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

		VkSubmitInfo end_info = {};
		end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		end_info.commandBufferCount = 1;
		end_info.pCommandBuffers = &command_buffer;
		vkEndCommandBuffer(command_buffer);
		vkQueueSubmit(init.graphics_queue, 1, &end_info, VK_NULL_HANDLE);

		vkDeviceWaitIdle(init.device);
		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}
}

ImGuiHelper::~ImGuiHelper() {
	vkDestroyDescriptorPool(init.device, imguiPool, nullptr);
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void ImGuiHelper::Begin() {
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void ImGuiHelper::End() {
	ImGui::Render();
}

void ImGuiHelper::OnRender() {
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), init.commandBuffer);
}
