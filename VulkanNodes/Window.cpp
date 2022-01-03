#include "Window.h"

#include <cassert>

Window::Window() 
	: width(640), height(480),
	window(InitWindow()), 
	extensions(InitExtensions()) {}

GLFWwindow* Window::InitWindow() {
	assert(glfwInit());
	assert(glfwVulkanSupported());

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	// TODO: implement swapchain recreation at resize, then enable resizability, otherwise causes crashes by accidental resizes
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	GLFWwindow* window = glfwCreateWindow(width, height, "Vulkan Nodes", NULL, NULL);
	assert(window != nullptr);

	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, OnFramebufferResized);
	return window;
}

std::vector<const char*> Window::InitExtensions() {
	uint32_t count;
	// GLFW docs says returned array will be freed by GLFW
	const char** names = glfwGetRequiredInstanceExtensions(&count);
	return std::vector<const char*>(names, names + count);
}

Window::~Window() {
	glfwDestroyWindow(window);
	glfwTerminate();
}

void Window::OnFramebufferResized(GLFWwindow* window, int width, int height) {
	void* ptr = glfwGetWindowUserPointer(window);
	if (ptr == nullptr) {
		return;
	}
	Window* win = static_cast<Window*>(ptr);
	win->width = width;
	win->height = height;
}

VkResult Window::CreateSurface(VkInstance instance, VkSurfaceKHR* surface) const {
	return glfwCreateWindowSurface(instance, window, nullptr, surface);
}

bool Window::ShouldClose() {
	return glfwWindowShouldClose(window);
}

void Window::PollEvents() {
	glfwPollEvents();
}
