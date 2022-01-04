#include "VulkanContext.h"

#include <array>

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
    depthFormat(VK_FORMAT_D24_UNORM_S8_UINT),
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

VulkanContext::SurfaceInfo VulkanContext::InitSurfaceInfo() {
	SurfaceInfo surfaceInfo;
	surfaceInfo.renderPass = CreateSurfaceRenderPass();
	// While app is being initialized a resize callback is called which calls CreateDepthAttachment anyway
	// if we call it now, it'll create lingering unused FramebufferAttachment resources
	//surfaceInfo.depthAttachment = CreateDepthAttachment();
	return surfaceInfo;
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
    depthAttachment.format = depthFormat;
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

VulkanContext::FramebufferAttachment VulkanContext::CreateDepthAttachment() {
	FramebufferAttachment attachment;

	VkImageCreateInfo depthImageInfo = {};
	depthImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	depthImageInfo.pNext = nullptr;
	depthImageInfo.imageType = VK_IMAGE_TYPE_2D;
	depthImageInfo.format = depthFormat;
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
	swapchain.destroy_image_views(swapchainData.imageViews);
	vkb::destroy_swapchain(swapchain);

	vkFreeMemory(device, surfaceInfo.depthAttachment.memory, nullptr);
	vkDestroyImageView(device, surfaceInfo.depthAttachment.imageView, nullptr);
	vkDestroyImage(device, surfaceInfo.depthAttachment.image, nullptr);

	vkDestroyRenderPass(device, surfaceInfo.renderPass, nullptr);

	vkb::destroy_device(device);
	vkb::destroy_surface(instance, surface);
	vkb::destroy_instance(instance);
}

void VulkanContext::RecreateSwapchain() {
	swapchain.destroy_image_views(swapchainData.imageViews);
	vkDestroyImage(device, surfaceInfo.depthAttachment.image, nullptr);
	vkDestroyImageView(device, surfaceInfo.depthAttachment.imageView, nullptr);
	vkFreeMemory(device, surfaceInfo.depthAttachment.memory, nullptr);

	auto oldSwapchain = swapchain;
	swapchain = vkb::detail::GetResult(
		vkb::SwapchainBuilder(device).set_old_swapchain(oldSwapchain).build()
	);
	swapchainData = { swapchain.get_images().value(), swapchain.get_image_views().value() };
	vkb::destroy_swapchain(oldSwapchain);
}
