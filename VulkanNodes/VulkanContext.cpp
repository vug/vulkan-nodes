#include "VulkanContext.h"

VulkanContext::VulkanContext(const Window& win)
	: win(win),
	instance(InitInstance()),
	surface(InitSurface()),
	device(InitDevice()) {
}

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

VulkanContext::~VulkanContext() {
	vkb::destroy_device(device);
	vkb::destroy_surface(instance, surface);
	vkb::destroy_instance(instance);
}
