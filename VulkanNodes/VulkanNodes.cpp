﻿#include "VulkanNodes.h"

#include "Window.h"
#include "VulkanContext.h"
#include "ImGuiHelper.h"

#include <imgui.h>

#include <cassert>
#include <iostream>
#include <fstream>

struct RenderData {
	VkPipelineLayout pipeline_layout = {};
	VkPipeline graphics_pipeline = {};
	size_t current_frame = 0;
};

std::vector<char> readFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	size_t file_size = (size_t)file.tellg();
	std::vector<char> buffer(file_size);

	file.seekg(0);
	file.read(buffer.data(), static_cast<std::streamsize> (file_size));

	file.close();

	return buffer;
}

VkShaderModule createShaderModule(VulkanContext& init, const std::vector<char>& code) {
	VkShaderModuleCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.codeSize = code.size();
	create_info.pCode = reinterpret_cast<const uint32_t*> (code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(init.device, &create_info, nullptr, &shaderModule) != VK_SUCCESS) {
		return VK_NULL_HANDLE; // failed to create shader module
	}

	return shaderModule;
}

int create_graphics_pipeline(VulkanContext& init, RenderData& data) {
	auto vert_code = readFile(std::string("shaders/shade-vert.spv"));
	auto frag_code = readFile(std::string("shaders/shade-frag.spv"));

	VkShaderModule vert_module = createShaderModule(init, vert_code);
	VkShaderModule frag_module = createShaderModule(init, frag_code);
	if (vert_module == VK_NULL_HANDLE || frag_module == VK_NULL_HANDLE) {
		std::cout << "failed to create shader module\n";
		return -1; // failed to create shader modules
	}

	VkPipelineShaderStageCreateInfo vert_stage_info = {};
	vert_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vert_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vert_stage_info.module = vert_module;
	vert_stage_info.pName = "main";

	VkPipelineShaderStageCreateInfo frag_stage_info = {};
	frag_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	frag_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	frag_stage_info.module = frag_module;
	frag_stage_info.pName = "main";

	VkPipelineShaderStageCreateInfo shader_stages[] = { vert_stage_info, frag_stage_info };

	VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
	vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_info.vertexBindingDescriptionCount = 0;
	vertex_input_info.vertexAttributeDescriptionCount = 0;

	VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly.primitiveRestartEnable = VK_FALSE;

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

	VkPipelineViewportStateCreateInfo viewport_state = {};
	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.viewportCount = 1;
	viewport_state.pViewports = &viewport;
	viewport_state.scissorCount = 1;
	viewport_state.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo color_blending = {};
	color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blending.logicOpEnable = VK_FALSE;
	color_blending.logicOp = VK_LOGIC_OP_COPY;
	color_blending.attachmentCount = 1;
	color_blending.pAttachments = &colorBlendAttachment;
	color_blending.blendConstants[0] = 0.0f;
	color_blending.blendConstants[1] = 0.0f;
	color_blending.blendConstants[2] = 0.0f;
	color_blending.blendConstants[3] = 0.0f;

	VkPipelineLayoutCreateInfo pipeline_layout_info = {};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = 0;
	pipeline_layout_info.pushConstantRangeCount = 0;

	if (vkCreatePipelineLayout(
		init.device, &pipeline_layout_info, nullptr, &data.pipeline_layout) != VK_SUCCESS) {
		std::cout << "failed to create pipeline layout\n";
		return -1; // failed to create pipeline layout
	}

	std::vector<VkDynamicState> dynamic_states = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

	VkPipelineDynamicStateCreateInfo dynamic_info = {};
	dynamic_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_info.dynamicStateCount = static_cast<uint32_t> (dynamic_states.size());
	dynamic_info.pDynamicStates = dynamic_states.data();

	VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
	depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilInfo.pNext = nullptr;
	depthStencilInfo.depthTestEnable = VK_TRUE;
	depthStencilInfo.depthWriteEnable = VK_TRUE;
	depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
	depthStencilInfo.minDepthBounds = 0.0f; // Optional
	depthStencilInfo.maxDepthBounds = 1.0f; // Optional
	depthStencilInfo.stencilTestEnable = VK_FALSE;

	VkGraphicsPipelineCreateInfo pipeline_info = {};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.stageCount = 2;
	pipeline_info.pStages = shader_stages;
	pipeline_info.pVertexInputState = &vertex_input_info;
	pipeline_info.pInputAssemblyState = &input_assembly;
	pipeline_info.pViewportState = &viewport_state;
	pipeline_info.pRasterizationState = &rasterizer;
	pipeline_info.pMultisampleState = &multisampling;
	pipeline_info.pDepthStencilState = &depthStencilInfo;
	pipeline_info.pColorBlendState = &color_blending;
	pipeline_info.pDynamicState = &dynamic_info;
	pipeline_info.layout = data.pipeline_layout;
	pipeline_info.renderPass = init.surfaceRenderPass;
	pipeline_info.subpass = 0;
	pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

	if (vkCreateGraphicsPipelines(
		init.device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &data.graphics_pipeline) != VK_SUCCESS) {
		std::cout << "failed to create pipline\n";
		return -1; // failed to create graphics pipeline
	}

	vkDestroyShaderModule(init.device, frag_module, nullptr);
	vkDestroyShaderModule(init.device, vert_module, nullptr);
	return 0;
}

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

void cleanup(VulkanContext& init, RenderData& data) {
	vkDeviceWaitIdle(init.device);

	vkDestroyPipeline(init.device, data.graphics_pipeline, nullptr);
	vkDestroyPipelineLayout(init.device, data.pipeline_layout, nullptr);
}

int main() {
	Window win;

	VulkanContext init(win);
	RenderData render_data;

	assert(0 == create_graphics_pipeline(init, render_data));

	ImGuiHelper imGuiHelper(init);

	while (!win.ShouldClose()) {
		win.PollEvents();

		imGuiHelper.Begin();
		static bool showDemo = true;
		ImGui::ShowDemoWindow(&showDemo);
		imGuiHelper.End();

		int res = draw_frame(init, render_data, imGuiHelper);
	}
	vkDeviceWaitIdle(init.device);
	cleanup(init, render_data);
	return 0;
}
