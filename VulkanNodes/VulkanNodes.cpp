﻿#include "VulkanNodes.h"

#include "Window.h"
#include "VulkanContext.h"

#include <cassert>
#include <iostream>
#include <fstream>

const int MAX_FRAMES_IN_FLIGHT = 2;

struct RenderData {
	VkQueue graphics_queue = {};
	VkQueue present_queue = {};

	std::vector<VkImage> swapchain_images;
	std::vector<VkImageView> swapchain_image_views;
	std::vector<VkFramebuffer> framebuffers;

	VkRenderPass render_pass = {};
	VkPipelineLayout pipeline_layout = {};
	VkPipeline graphics_pipeline = {};

	VkCommandPool command_pool = {};
	std::vector<VkCommandBuffer> command_buffers;

	std::vector<VkSemaphore> available_semaphores;
	std::vector<VkSemaphore> finished_semaphore;
	std::vector<VkFence> in_flight_fences;
	std::vector<VkFence> image_in_flight;
	size_t current_frame = 0;
};

void create_swapchain(VulkanContext& init) {
	vkb::SwapchainBuilder swapchain_builder{ init.device };
	auto oldSwapchain = init.swapchain;
	init.swapchain = vkb::detail::GetResult(swapchain_builder.set_old_swapchain(oldSwapchain).build());
	vkb::destroy_swapchain(oldSwapchain);
}

void get_queues(VulkanContext& init, RenderData& data) {
	data.graphics_queue = vkb::detail::GetResult(
		init.device.get_queue(vkb::QueueType::graphics)
	);

	data.present_queue = vkb::detail::GetResult(
		init.device.get_queue(vkb::QueueType::present)
	);
}

int create_render_pass(VulkanContext& init, RenderData& data) {
	VkAttachmentDescription color_attachment = {};
	color_attachment.format = init.swapchain.image_format;
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_ref = {};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_info.attachmentCount = 1;
	render_pass_info.pAttachments = &color_attachment;
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;
	render_pass_info.dependencyCount = 1;
	render_pass_info.pDependencies = &dependency;

	if (vkCreateRenderPass(init.device, &render_pass_info, nullptr, &data.render_pass) != VK_SUCCESS) {
		std::cout << "failed to create render pass\n";
		return -1; // failed to create render pass!
	}
	return 0;
}

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

	VkGraphicsPipelineCreateInfo pipeline_info = {};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.stageCount = 2;
	pipeline_info.pStages = shader_stages;
	pipeline_info.pVertexInputState = &vertex_input_info;
	pipeline_info.pInputAssemblyState = &input_assembly;
	pipeline_info.pViewportState = &viewport_state;
	pipeline_info.pRasterizationState = &rasterizer;
	pipeline_info.pMultisampleState = &multisampling;
	pipeline_info.pColorBlendState = &color_blending;
	pipeline_info.pDynamicState = &dynamic_info;
	pipeline_info.layout = data.pipeline_layout;
	pipeline_info.renderPass = data.render_pass;
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

int create_framebuffers(VulkanContext& init, RenderData& data) {
	data.swapchain_images = init.swapchain.get_images().value();
	data.swapchain_image_views = init.swapchain.get_image_views().value();

	data.framebuffers.resize(data.swapchain_image_views.size());

	for (size_t i = 0; i < data.swapchain_image_views.size(); i++) {
		VkImageView attachments[] = { data.swapchain_image_views[i] };

		VkFramebufferCreateInfo framebuffer_info = {};
		framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_info.renderPass = data.render_pass;
		framebuffer_info.attachmentCount = 1;
		framebuffer_info.pAttachments = attachments;
		framebuffer_info.width = init.swapchain.extent.width;
		framebuffer_info.height = init.swapchain.extent.height;
		framebuffer_info.layers = 1;

		if (vkCreateFramebuffer(init.device, &framebuffer_info, nullptr, &data.framebuffers[i]) != VK_SUCCESS) {
			return -1; // failed to create framebuffer
		}
	}
	return 0;
}

int create_command_pool(VulkanContext& init, RenderData& data) {
	VkCommandPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.queueFamilyIndex = init.device.get_queue_index(vkb::QueueType::graphics).value();

	if (vkCreateCommandPool(init.device, &pool_info, nullptr, &data.command_pool) != VK_SUCCESS) {
		std::cout << "failed to create command pool\n";
		return -1; // failed to create command pool
	}
	return 0;
}

int create_command_buffers(VulkanContext& init, RenderData& data) {
	data.command_buffers.resize(data.framebuffers.size());

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = data.command_pool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)data.command_buffers.size();

	if (vkAllocateCommandBuffers(init.device, &allocInfo, data.command_buffers.data()) != VK_SUCCESS) {
		return -1; // failed to allocate command buffers;
	}

	for (size_t i = 0; i < data.command_buffers.size(); i++) {
		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(data.command_buffers[i], &begin_info) != VK_SUCCESS) {
			return -1; // failed to begin recording command buffer
		}

		VkRenderPassBeginInfo render_pass_info = {};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_info.renderPass = data.render_pass;
		render_pass_info.framebuffer = data.framebuffers[i];
		render_pass_info.renderArea.offset = { 0, 0 };
		render_pass_info.renderArea.extent = init.swapchain.extent;
		VkClearValue clearColor{ { { 0.0f, 0.0f, 0.0f, 1.0f } } };
		render_pass_info.clearValueCount = 1;
		render_pass_info.pClearValues = &clearColor;

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

		vkCmdSetViewport(data.command_buffers[i], 0, 1, &viewport);
		vkCmdSetScissor(data.command_buffers[i], 0, 1, &scissor);

		vkCmdBeginRenderPass(data.command_buffers[i], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(data.command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, data.graphics_pipeline);

		vkCmdDraw(data.command_buffers[i], 3, 1, 0, 0);

		vkCmdEndRenderPass(data.command_buffers[i]);

		if (vkEndCommandBuffer(data.command_buffers[i]) != VK_SUCCESS) {
			std::cout << "failed to record command buffer\n";
			return -1; // failed to record command buffer!
		}
	}
	return 0;
}

int create_sync_objects(VulkanContext& init, RenderData& data) {
	data.available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
	data.finished_semaphore.resize(MAX_FRAMES_IN_FLIGHT);
	data.in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);
	data.image_in_flight.resize(init.swapchain.image_count, VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphore_info = {};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fence_info = {};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore(init.device, &semaphore_info, nullptr, &data.available_semaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(init.device, &semaphore_info, nullptr, &data.finished_semaphore[i]) != VK_SUCCESS ||
			vkCreateFence(init.device, &fence_info, nullptr, &data.in_flight_fences[i]) != VK_SUCCESS) {
			std::cout << "failed to create sync objects\n";
			return -1; // failed to create synchronization objects for a frame
		}
	}
	return 0;
}

int recreate_swapchain(VulkanContext& init, RenderData& data) {
	vkDeviceWaitIdle(init.device);

	vkDestroyCommandPool(init.device, data.command_pool, nullptr);

	for (auto framebuffer : data.framebuffers) {
		vkDestroyFramebuffer(init.device, framebuffer, nullptr);
	}

	init.swapchain.destroy_image_views(data.swapchain_image_views);

	create_swapchain(init);
	if (0 != create_framebuffers(init, data)) return -1;
	if (0 != create_command_pool(init, data)) return -1;
	if (0 != create_command_buffers(init, data)) return -1;
	return 0;
}

int draw_frame(VulkanContext& init, RenderData& data) {
	vkWaitForFences(init.device, 1, &data.in_flight_fences[data.current_frame], VK_TRUE, UINT64_MAX);

	uint32_t image_index = 0;
	VkResult result = vkAcquireNextImageKHR(init.device,
		init.swapchain,
		UINT64_MAX,
		data.available_semaphores[data.current_frame],
		VK_NULL_HANDLE,
		&image_index);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		return recreate_swapchain(init, data);
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		std::cout << "failed to acquire swapchain image. Error " << result << "\n";
		return -1;
	}

	if (data.image_in_flight[image_index] != VK_NULL_HANDLE) {
		vkWaitForFences(init.device, 1, &data.image_in_flight[image_index], VK_TRUE, UINT64_MAX);
	}
	data.image_in_flight[image_index] = data.in_flight_fences[data.current_frame];

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore wait_semaphores[] = { data.available_semaphores[data.current_frame] };
	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = wait_semaphores;
	submitInfo.pWaitDstStageMask = wait_stages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &data.command_buffers[image_index];

	VkSemaphore signal_semaphores[] = { data.finished_semaphore[data.current_frame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signal_semaphores;

	vkResetFences(init.device, 1, &data.in_flight_fences[data.current_frame]);

	if (vkQueueSubmit(data.graphics_queue, 1, &submitInfo, data.in_flight_fences[data.current_frame]) != VK_SUCCESS) {
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

	result = vkQueuePresentKHR(data.present_queue, &present_info);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		return recreate_swapchain(init, data);
	}
	else if (result != VK_SUCCESS) {
		std::cout << "failed to present swapchain image\n";
		return -1;
	}

	data.current_frame = (data.current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
	return 0;
}

void cleanup(VulkanContext& init, RenderData& data) {
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(init.device, data.finished_semaphore[i], nullptr);
		vkDestroySemaphore(init.device, data.available_semaphores[i], nullptr);
		vkDestroyFence(init.device, data.in_flight_fences[i], nullptr);
	}

	vkDestroyCommandPool(init.device, data.command_pool, nullptr);

	for (auto framebuffer : data.framebuffers) {
		vkDestroyFramebuffer(init.device, framebuffer, nullptr);
	}

	vkDestroyPipeline(init.device, data.graphics_pipeline, nullptr);
	vkDestroyPipelineLayout(init.device, data.pipeline_layout, nullptr);
	vkDestroyRenderPass(init.device, data.render_pass, nullptr);

	init.swapchain.destroy_image_views(data.swapchain_image_views);

	vkb::destroy_swapchain(init.swapchain);
}

int main() {
	Window win;

	VulkanContext init(win);
	RenderData render_data;

	create_swapchain(init);
	get_queues(init, render_data);
	if (0 != create_render_pass(init, render_data)) return -1;
	if (0 != create_graphics_pipeline(init, render_data)) return -1;
	if (0 != create_framebuffers(init, render_data)) return -1;
	if (0 != create_command_pool(init, render_data)) return -1;
	if (0 != create_command_buffers(init, render_data)) return -1;
	if (0 != create_sync_objects(init, render_data)) return -1;


	while (!win.ShouldClose()) {
		win.PollEvents();
		int res = draw_frame(init, render_data);
		if (res != 0) {
			std::cout << "failed to draw frame \n";
			return -1;
		}
	}
	vkDeviceWaitIdle(init.device);
	cleanup(init, render_data);
	return 0;
}
