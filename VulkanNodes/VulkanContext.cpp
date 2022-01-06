#include "VulkanContext.h"

#include <array>
#include <iostream>
#include <fstream>

static uint32_t GetMemoryType(VkPhysicalDeviceMemoryProperties memoryProperties, uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound = nullptr) {
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
		if ((typeBits & 1) == 1) {
			if ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				if (memTypeFound)
					*memTypeFound = true;
				return i;
			}
		}
		typeBits >>= 1;
	}

	if (memTypeFound) {
		*memTypeFound = false;
		return 0;
	}
	else {
		std::cout << "Could not find a matching memory type\n";
		exit(EXIT_FAILURE);
	}
}

VulkanContext::VulkanContext(const Window& win)
	: win(win),
	instance(InitInstance()),
	surface(InitSurface()),
	device(InitDevice()),
	graphics_queue(vkb::detail::GetResult(device.get_queue(vkb::QueueType::graphics))),
	present_queue(vkb::detail::GetResult(device.get_queue(vkb::QueueType::present))),
	swapchain(vkb::detail::GetResult(vkb::SwapchainBuilder(device).build())),
	// vkb::Swapchain's get_images() and _views methods are meant to be only called once after Swapchain is created
	// It's a confusing API, so we have to make sure to update swapchainData any time we recreate swapchain too
	swapchainData({ swapchain.get_images().value(), swapchain.get_image_views().value() }),
    surfaceDepthFormat(VK_FORMAT_D24_UNORM_S8_UINT),
	surfaceRenderPass(CreateSurfaceRenderPass()),
	surfaceDepthAttachment(CreateDepthAttachment()),
	surfaceFramebuffers(CreateFramebuffers()),
	commandPool(CreateCommandPool()),
	commandBuffer(CreateCommandBuffer()),
	sync(InitSync()) {}

vkb::Instance VulkanContext::InitInstance() {
	vkb::InstanceBuilder instanceBuilder = vkb::InstanceBuilder()
		.use_default_debug_messenger()
		.enable_validation_layers();
	for (const auto& extName : win.GetInstanceExtensions())
		instanceBuilder.enable_extension(extName);
	return vkb::detail::GetResult(instanceBuilder.build());
}

VkSurfaceKHR VulkanContext::InitSurface() {
	VkSurfaceKHR surface;
	win.CreateSurface(instance, &surface);
	return surface;
}

VkCommandPool VulkanContext::CreateCommandPool() {
	VkCommandPoolCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	info.queueFamilyIndex = device.get_queue_index(vkb::QueueType::graphics).value();
	info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VkCommandPool commandPool;
	if (vkCreateCommandPool(device, &info, nullptr, &commandPool) != VK_SUCCESS) {
		std::cout << "failed to create command pool\n";
		exit(EXIT_FAILURE);
	}
	return commandPool;
}

vkb::Device VulkanContext::InitDevice() {
	vkb::PhysicalDevice physical_device = vkb::detail::GetResult(
		vkb::PhysicalDeviceSelector(instance).set_surface(surface).select()
	);
	return vkb::detail::GetResult(vkb::DeviceBuilder(physical_device).build());
}

VkCommandBuffer VulkanContext::CreateCommandBuffer() {
	VkCommandBuffer cmdBuf;
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1u;

	if (vkAllocateCommandBuffers(device, &allocInfo, &cmdBuf) != VK_SUCCESS) {
		exit(EXIT_FAILURE);
	}

	return cmdBuf;
}

std::vector<VkFramebuffer> VulkanContext::CreateFramebuffers() {
	std::vector<VkFramebuffer> framebuffers;

	vkDestroyImage(device, surfaceDepthAttachment.image, nullptr);
	vkDestroyImageView(device, surfaceDepthAttachment.imageView, nullptr);
	vkFreeMemory(device, surfaceDepthAttachment.memory, nullptr);

	surfaceDepthAttachment = CreateDepthAttachment();
	framebuffers.resize(swapchainData.imageViews.size());

	for (size_t i = 0; i < swapchainData.imageViews.size(); i++) {
		std::vector<VkImageView> attachments = { swapchainData.imageViews[i], surfaceDepthAttachment.imageView };

		VkFramebufferCreateInfo framebuffer_info = {};
		framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_info.renderPass = surfaceRenderPass;
		framebuffer_info.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebuffer_info.pAttachments = attachments.data();
		framebuffer_info.width = swapchain.extent.width;
		framebuffer_info.height = swapchain.extent.height;
		framebuffer_info.layers = 1;

		if (vkCreateFramebuffer(device, &framebuffer_info, nullptr, &framebuffers[i]) != VK_SUCCESS) {
			exit(EXIT_FAILURE);
		}
	}

	return framebuffers;
}

VkRenderPass VulkanContext::CreateSurfaceRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapchain.image_format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = surfaceDepthFormat;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // not _COMPUTE
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    std::vector<VkAttachmentDescription> attachments = { colorAttachment, depthAttachment };
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    VkRenderPass renderPass;
    VkResult result = vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass);
    if (result != VK_SUCCESS) {
        std::cout << "Failed to create RenderPass for Swapchain!\n";
        exit(EXIT_FAILURE);
    }
    return renderPass;
}

VulkanContext::Sync VulkanContext::InitSync() {
	Sync sync;
	sync.available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
	sync.finished_semaphore.resize(MAX_FRAMES_IN_FLIGHT);
	sync.in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);
	sync.image_in_flight.resize(swapchain.image_count, VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphore_info = {};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fence_info = {};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore(device, &semaphore_info, nullptr, &sync.available_semaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(device, &semaphore_info, nullptr, &sync.finished_semaphore[i]) != VK_SUCCESS ||
			vkCreateFence(device, &fence_info, nullptr, &sync.in_flight_fences[i]) != VK_SUCCESS) {
			std::cout << "failed to create sync objects\n";
			exit(EXIT_FAILURE);
		}
	}
	return sync;
}

VulkanContext::FramebufferAttachment VulkanContext::CreateDepthAttachment() {
	FramebufferAttachment attachment;

	VkImageCreateInfo depthImageInfo = {};
	depthImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	depthImageInfo.pNext = nullptr;
	depthImageInfo.imageType = VK_IMAGE_TYPE_2D;
	depthImageInfo.format = surfaceDepthFormat;
	depthImageInfo.extent = { swapchain.extent.width, swapchain.extent.height, 1u };
	depthImageInfo.mipLevels = 1;
	depthImageInfo.arrayLayers = 1;
	depthImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	depthImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

	VkMemoryAllocateInfo memAlloc = {};
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	assert(vkCreateImage(device, &depthImageInfo, nullptr, &attachment.image) == VK_SUCCESS);
	VkMemoryRequirements memReqs;
	vkGetImageMemoryRequirements(device, attachment.image, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = GetMemoryType(device.physical_device.memory_properties, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	assert(vkAllocateMemory(device, &memAlloc, nullptr, &attachment.memory) == VK_SUCCESS);
	assert(vkBindImageMemory(device, attachment.image, attachment.memory, 0) == VK_SUCCESS);

	VkImageViewCreateInfo depthImageViewInfo = {};
	depthImageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	depthImageViewInfo.pNext = nullptr;
	depthImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	depthImageViewInfo.image = attachment.image;
	depthImageViewInfo.format = depthImageInfo.format;
	depthImageViewInfo.subresourceRange.baseMipLevel = 0;
	depthImageViewInfo.subresourceRange.levelCount = 1;
	depthImageViewInfo.subresourceRange.baseArrayLayer = 0;
	depthImageViewInfo.subresourceRange.layerCount = 1;
	depthImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	if (vkCreateImageView(device, &depthImageViewInfo, nullptr, &attachment.imageView) != VK_SUCCESS) {
		exit(EXIT_FAILURE);
	}

	return attachment;
}

VulkanContext::~VulkanContext() {
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(device, sync.finished_semaphore[i], nullptr);
		vkDestroySemaphore(device, sync.available_semaphores[i], nullptr);
		vkDestroyFence(device, sync.in_flight_fences[i], nullptr);
	}

	vkDestroyCommandPool(device, commandPool, nullptr);

	for (auto& framebuffer : surfaceFramebuffers) {
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	}

	swapchain.destroy_image_views(swapchainData.imageViews);
	vkb::destroy_swapchain(swapchain);

	vkFreeMemory(device, surfaceDepthAttachment.memory, nullptr);
	vkDestroyImageView(device, surfaceDepthAttachment.imageView, nullptr);
	vkDestroyImage(device, surfaceDepthAttachment.image, nullptr);

	vkDestroyRenderPass(device, surfaceRenderPass, nullptr);

	vkb::destroy_device(device);
	vkb::destroy_surface(instance, surface);
	vkb::destroy_instance(instance);
}

void VulkanContext::RecreateSwapchain() {
	vkDeviceWaitIdle(device);

	for (auto& framebuffer : surfaceFramebuffers) {
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	}
	swapchain.destroy_image_views(swapchainData.imageViews);

	auto oldSwapchain = swapchain;
	swapchain = vkb::detail::GetResult(
		vkb::SwapchainBuilder(device).set_old_swapchain(oldSwapchain).build()
	);
	vkb::destroy_swapchain(oldSwapchain);

	swapchainData = { swapchain.get_images().value(), swapchain.get_image_views().value() };
	surfaceFramebuffers = CreateFramebuffers();
}

std::vector<char> VulkanContext::ReadFile(const std::string& filename) {
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

VkShaderModule VulkanContext::CreateShaderModule(const std::vector<char>& code) {
	VkShaderModuleCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.codeSize = code.size();
	create_info.pCode = reinterpret_cast<const uint32_t*> (code.data());

	VkShaderModule shaderModule;
	VkResult result = vkCreateShaderModule(device, &create_info, nullptr, &shaderModule);
	assert(result == VK_SUCCESS);

	return shaderModule;
}

VkPipeline VulkanContext::CreateSurfaceCompatiblePipeline(VkShaderModule vert, VkShaderModule frag, VkPipelineLayout layout) {
	VkPipelineShaderStageCreateInfo vert_stage_info = {};
	vert_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vert_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vert_stage_info.module = vert;
	vert_stage_info.pName = "main";

	VkPipelineShaderStageCreateInfo frag_stage_info = {};
	frag_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	frag_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	frag_stage_info.module = frag;
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
	viewport.width = (float)swapchain.extent.width;
	viewport.height = (float)swapchain.extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = swapchain.extent;

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
	pipeline_info.layout = layout;
	pipeline_info.renderPass = surfaceRenderPass;
	pipeline_info.subpass = 0;
	pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

	VkPipeline graphicsPipeline;
	assert(vkCreateGraphicsPipelines(
		device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &graphicsPipeline) == VK_SUCCESS);
	return graphicsPipeline;
}
