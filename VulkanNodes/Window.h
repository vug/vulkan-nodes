#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <vector>

class Window {
public:
	Window();
	Window(const Window&) = delete;
	Window(Window&&) = delete;
	Window& operator=(const Window&) = delete;
	Window& operator=(Window&& other) = delete;
	~Window();

	VkResult CreateSurface(VkInstance instance, VkSurfaceKHR* surface) const;

	bool ShouldClose() const;
	void PollEvents() const;

	// Getters
	GLFWwindow* GetGLFWWindow() const { return window; }
	const std::vector<const char*>& GetInstanceExtensions() const { return extensions; }
	const int GetWidth() const { return width; }
	const int GetHeight() const { return height; }
private:
	// construction
	GLFWwindow* InitWindow();
	std::vector<const char*> InitExtensions();

	static void OnFramebufferResized(GLFWwindow* window, int width, int height);
private:
	int width;
	int height;
	GLFWwindow* window;
	std::vector<const char*> extensions;
};