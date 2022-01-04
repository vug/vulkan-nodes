#include "VulkanContext.h"

VulkanContext::VulkanContext(const Window& win)
	: win(win),
	instance(InitInstance()),
	surface(InitSurface()),
	device(InitDevice()),
	swapchain(InitSwapchain()),
	surfaceInfo(InitSurfaceInfo()) {}

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

vkb::Device VulkanContext::InitDevice() {
	vkb::PhysicalDevice physical_device = vkb::detail::GetResult(
		vkb::PhysicalDeviceSelector(instance).set_surface(surface).select()
	);
	return vkb::detail::GetResult(vkb::DeviceBuilder(physical_device).build());
}

vkb::Swapchain VulkanContext::InitSwapchain() {
	vkb::SwapchainBuilder swapchain_builder{ device };
	vkb::Swapchain newSwapchain = vkb::detail::GetResult(swapchain_builder.set_old_swapchain(nullptr).build());
	return newSwapchain;
}

VulkanContext::SurfaceInfo VulkanContext::InitSurfaceInfo() {
	SurfaceInfo surfaceInfo;
	surfaceInfo.renderPass = CreateSurfaceRenderPass();
	return surfaceInfo;
}

VkRenderPass VulkanContext::CreateSurfaceRenderPass() {
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = swapchain.image_format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	info.attachmentCount = 1;
	info.pAttachments = &colorAttachment;
	info.subpassCount = 1;
	info.pSubpasses = &subpass;
	info.dependencyCount = 1;
	info.pDependencies = &dependency;

	VkRenderPass renderPass;
	if (vkCreateRenderPass(device, &info, nullptr, &renderPass) != VK_SUCCESS) {
		std::cout << "failed to create render pass\n";
		exit(EXIT_FAILURE);
	}
	return renderPass;
}

VulkanContext::~VulkanContext() {
	vkb::destroy_device(device);
	vkb::destroy_surface(instance, surface);
	vkb::destroy_instance(instance);
}

void VulkanContext::RecreateSwapchain() {
	auto oldSwapchain = swapchain;
	swapchain = vkb::detail::GetResult(
		vkb::SwapchainBuilder(device).set_old_swapchain(oldSwapchain).build()
	);
	vkb::destroy_swapchain(oldSwapchain);
}
