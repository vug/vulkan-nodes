#include "VulkanNodes.h"

#include "Window.h"
#include "VulkanContext.h"
#include "ImGuiHelper.h"

#include <imgui.h>

#include <cassert>

struct RenderData {
	VkPipeline graphics_pipeline = {};
	size_t current_frame = 0;
};

int fillCommandBuffer(VulkanContext& init, RenderData& data, int i, ImGuiHelper& imGuiHelper) {
	assert(vkResetCommandBuffer(init.commandBuffer, 0) == VK_SUCCESS);

	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	if (vkBeginCommandBuffer(init.commandBuffer, &begin_info) != VK_SUCCESS) {
		return -1; // failed to begin recording command buffer
	}

	std::vector<VkClearValue> clearValues(2);
	clearValues[0].color = { 0.0f, 0.0f, 0.0f, 0.0f };
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_info.renderPass = init.surfaceRenderPass;
	render_pass_info.framebuffer = init.surfaceFramebuffers[i];
	render_pass_info.renderArea.offset = { 0, 0 };
	render_pass_info.renderArea.extent = init.swapchain.extent;
	render_pass_info.clearValueCount = static_cast<uint32_t>(clearValues.size());
	render_pass_info.pClearValues = clearValues.data();

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)init.swapchain.extent.width;
	viewport.height = (float)init.swapchain.extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = init.swapchain.extent;

	vkCmdSetViewport(init.commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(init.commandBuffer, 0, 1, &scissor);

	vkCmdBeginRenderPass(init.commandBuffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(init.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, data.graphics_pipeline);

	vkCmdDraw(init.commandBuffer, 3, 1, 0, 0);

	imGuiHelper.OnRender();

	vkCmdEndRenderPass(init.commandBuffer);

	if (vkEndCommandBuffer(init.commandBuffer) != VK_SUCCESS) {
		std::cout << "failed to record command buffer\n";
		return -1; // failed to record command buffer!
	}

	return 0;
}

int draw_frame(VulkanContext& init, RenderData& data, ImGuiHelper& imGuiHelper) {
	vkWaitForFences(init.device, 1, &init.sync.in_flight_fences[data.current_frame], VK_TRUE, UINT64_MAX);

	uint32_t image_index = 0;
	VkResult result = vkAcquireNextImageKHR(init.device,
		init.swapchain,
		UINT64_MAX,
		init.sync.available_semaphores[data.current_frame],
		VK_NULL_HANDLE,
		&image_index);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		init.RecreateSwapchain();
		return 0;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		std::cout << "failed to acquire swapchain image. Error " << result << "\n";
		return -1;
	}

	if (init.sync.image_in_flight[image_index] != VK_NULL_HANDLE) {
		vkWaitForFences(init.device, 1, &init.sync.image_in_flight[image_index], VK_TRUE, UINT64_MAX);
	}
	init.sync.image_in_flight[image_index] = init.sync.in_flight_fences[data.current_frame];

	//
	assert(0 == fillCommandBuffer(init, data, image_index, imGuiHelper));
	//

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkSemaphore wait_semaphores[] = { init.sync.available_semaphores[data.current_frame] };
	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = wait_semaphores;
	submitInfo.pWaitDstStageMask = wait_stages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &init.commandBuffer;
	VkSemaphore signal_semaphores[] = { init.sync.finished_semaphore[data.current_frame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signal_semaphores;

	vkResetFences(init.device, 1, &init.sync.in_flight_fences[data.current_frame]);

	if (vkQueueSubmit(init.graphics_queue, 1, &submitInfo, init.sync.in_flight_fences[data.current_frame]) != VK_SUCCESS) {
		std::cout << "failed to submit draw command buffer\n";
		return -1; //"failed to submit draw command buffer
	}

	VkPresentInfoKHR present_info = {};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = signal_semaphores;

	VkSwapchainKHR swapChains[] = { init.swapchain };
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swapChains;

	present_info.pImageIndices = &image_index;

	result = vkQueuePresentKHR(init.present_queue, &present_info);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		init.RecreateSwapchain();
		return 0;
	}
	else if (result != VK_SUCCESS) {
		std::cout << "failed to present swapchain image\n";
		return -1;
	}

	data.current_frame = (data.current_frame + 1) % init.MAX_FRAMES_IN_FLIGHT;
	return 0;
}

int main() {
	Window win;

	VulkanContext init(win);
	RenderData render_data;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pushConstantRangeCount = 0;

	VkPipelineLayout pipelineLayout;
	assert(vkCreatePipelineLayout(
		init.device, &pipelineLayoutInfo, nullptr, &pipelineLayout) == VK_SUCCESS);

	auto vertCode = init.ReadFile(std::string("shaders/shade-vert.spv"));
	auto fragCode = init.ReadFile(std::string("shaders/shade-frag.spv"));
	VkShaderModule vert = init.CreateShaderModule(vertCode);
	VkShaderModule frag = init.CreateShaderModule(fragCode);
	VkPipeline pipeline = init.CreateSurfaceCompatiblePipeline(vert, frag, pipelineLayout);
	vkDestroyShaderModule(init.device, vert, nullptr);
	vkDestroyShaderModule(init.device, frag, nullptr);
	render_data.graphics_pipeline = pipeline;

	ImGuiHelper imGuiHelper(init);

	while (!win.ShouldClose()) {
		win.PollEvents();

		imGuiHelper.Begin();
		static bool showDemo = true;
		ImGui::ShowDemoWindow(&showDemo);
		imGuiHelper.End();

		int res = draw_frame(init, render_data, imGuiHelper);
	}

	// Cleanup
	vkDeviceWaitIdle(init.device);
	vkDestroyPipelineLayout(init.device, pipelineLayout, nullptr);
	vkDestroyPipeline(init.device, pipeline, nullptr);
	return 0;
}
