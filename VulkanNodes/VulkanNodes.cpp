#include "VulkanNodes.h"

#include "Window.h"
#include "VulkanContext.h"
#include "ImGuiHelper.h"

#include <imgui.h>

#include <cassert>
#include <functional>

struct RenderData {
	size_t currentInFlightFrame = 0;
};

void DrawFrame(VulkanContext& vc, RenderData& data, std::function<void(const VkCommandBuffer&)> cmdBufFillingFunc) {
	vkWaitForFences(vc.device, 1, &vc.sync.in_flight_fences[data.currentInFlightFrame], VK_TRUE, UINT64_MAX);

	uint32_t image_index = 0;
	VkResult result = vkAcquireNextImageKHR(vc.device,
		vc.swapchain,
		UINT64_MAX,
		vc.sync.available_semaphores[data.currentInFlightFrame],
		VK_NULL_HANDLE,
		&image_index);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		vc.RecreateSwapchain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		std::cout << "failed to acquire swapchain image. Error " << result << "\n";
		exit(EXIT_FAILURE);
	}

	if (vc.sync.image_in_flight[image_index] != VK_NULL_HANDLE) {
		vkWaitForFences(vc.device, 1, &vc.sync.image_in_flight[image_index], VK_TRUE, UINT64_MAX);
	}
	vc.sync.image_in_flight[image_index] = vc.sync.in_flight_fences[data.currentInFlightFrame];


	// Rest cmdBuf, prepare RenderPass Begin, fill draw commands, end renderpass
	assert(vkResetCommandBuffer(vc.commandBuffer, 0) == VK_SUCCESS);

	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	assert(vkBeginCommandBuffer(vc.commandBuffer, &begin_info) == VK_SUCCESS);

	std::vector<VkClearValue> clearValues(2);
	clearValues[0].color = { 0.0f, 0.0f, 0.0f, 0.0f };
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_info.renderPass = vc.surfaceRenderPass;
	render_pass_info.framebuffer = vc.surfaceFramebuffers[image_index];
	render_pass_info.renderArea.offset = { 0, 0 };
	render_pass_info.renderArea.extent = vc.swapchain.extent;
	render_pass_info.clearValueCount = static_cast<uint32_t>(clearValues.size());
	render_pass_info.pClearValues = clearValues.data();

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)vc.swapchain.extent.width;
	viewport.height = (float)vc.swapchain.extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = vc.swapchain.extent;

	vkCmdSetViewport(vc.commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(vc.commandBuffer, 0, 1, &scissor);

	vkCmdBeginRenderPass(vc.commandBuffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

	cmdBufFillingFunc(vc.commandBuffer);

	vkCmdEndRenderPass(vc.commandBuffer);

	assert(vkEndCommandBuffer(vc.commandBuffer) == VK_SUCCESS);
	//


	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkSemaphore wait_semaphores[] = { vc.sync.available_semaphores[data.currentInFlightFrame] };
	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = wait_semaphores;
	submitInfo.pWaitDstStageMask = wait_stages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &vc.commandBuffer;
	VkSemaphore signal_semaphores[] = { vc.sync.finished_semaphore[data.currentInFlightFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signal_semaphores;

	vkResetFences(vc.device, 1, &vc.sync.in_flight_fences[data.currentInFlightFrame]);

	assert(vkQueueSubmit(vc.graphics_queue, 1, &submitInfo, vc.sync.in_flight_fences[data.currentInFlightFrame]) == VK_SUCCESS);

	VkPresentInfoKHR present_info = {};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = signal_semaphores;

	VkSwapchainKHR swapChains[] = { vc.swapchain };
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swapChains;

	present_info.pImageIndices = &image_index;

	result = vkQueuePresentKHR(vc.present_queue, &present_info);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		vc.RecreateSwapchain();
		return;
	}
	else if (result != VK_SUCCESS) {
		std::cout << "failed to present swapchain image\n";
		exit(EXIT_FAILURE);
	}

	data.currentInFlightFrame = (data.currentInFlightFrame + 1) % vc.MAX_FRAMES_IN_FLIGHT;
}

int main() {
	Window win;

	VulkanContext vc(win);
	RenderData render_data;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pushConstantRangeCount = 0;

	VkPipelineLayout pipelineLayout;
	assert(vkCreatePipelineLayout(
		vc.device, &pipelineLayoutInfo, nullptr, &pipelineLayout) == VK_SUCCESS);

	auto vertCode = vc.ReadFile(std::string("shaders/shade-vert.spv"));
	auto fragCode = vc.ReadFile(std::string("shaders/shade-frag.spv"));
	VkShaderModule vert = vc.CreateShaderModule(vertCode);
	VkShaderModule frag = vc.CreateShaderModule(fragCode);
	VkPipeline pipeline = vc.CreateSurfaceCompatiblePipeline(vert, frag, pipelineLayout);
	vkDestroyShaderModule(vc.device, vert, nullptr);
	vkDestroyShaderModule(vc.device, frag, nullptr);

	ImGuiHelper imGuiHelper(vc);

	while (!win.ShouldClose()) {
		win.PollEvents();

		imGuiHelper.Begin();
		static bool showDemo = true;
		ImGui::ShowDemoWindow(&showDemo);
		imGuiHelper.End();

		auto func = [&](const VkCommandBuffer& cmdBuf) {
			vkCmdBindPipeline(vc.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
			vkCmdDraw(vc.commandBuffer, 3, 1, 0, 0);
			imGuiHelper.AddDrawCalls(vc.commandBuffer);
		};
		DrawFrame(vc, render_data, func);
	}

	// Cleanup
	vkDeviceWaitIdle(vc.device);
	vkDestroyPipelineLayout(vc.device, pipelineLayout, nullptr);
	vkDestroyPipeline(vc.device, pipeline, nullptr);
	return 0;
}
