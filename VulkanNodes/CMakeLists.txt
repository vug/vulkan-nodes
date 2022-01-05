cmake_minimum_required (VERSION 3.8)

add_executable (VulkanNodes "VulkanNodes.cpp" "VulkanNodes.h" "Window.h" "Window.cpp" "VulkanContext.h" "VulkanContext.cpp")

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(VulkanNodes PRIVATE glfw)

find_package(Vulkan REQUIRED)
target_link_libraries(VulkanNodes PRIVATE Vulkan::Vulkan)

include(FetchContent)
FetchContent_Declare(
    fetch_vk_bootstrap
    GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap
    GIT_TAG        6bf383778bfadb0eb173e6f25a9a39e7e9def1cf
)
FetchContent_MakeAvailable(fetch_vk_bootstrap)
target_link_libraries(VulkanNodes PRIVATE vk-bootstrap::vk-bootstrap)

find_package(imgui CONFIG REQUIRED)
target_link_libraries(VulkanNodes PRIVATE imgui::imgui)