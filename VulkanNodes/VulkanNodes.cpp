#include "VulkanNodes.h"
#include "NodeEditor.h"

#include "Window.h"
#include "VulkanContext.h"
#include "ImGuiHelper.h"

#include <imgui.h>
#include "dependencies/imnodes.h"

#include <cassert>


int main() {
	const Window win{};

	VulkanContext vc{ win };

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pushConstantRangeCount = 0;

	VkPipelineLayout pipelineLayout;
	assert(vkCreatePipelineLayout(
		vc.device, &pipelineLayoutInfo, nullptr, &pipelineLayout) == VK_SUCCESS);

	const auto vertCode{ vc.ReadFile(std::string("shaders/shade-vert.spv")) };
	const auto fragCode{ vc.ReadFile(std::string("shaders/shade-frag.spv")) };
	const VkShaderModule vert{ vc.CreateShaderModule(vertCode) };
	const VkShaderModule frag{ vc.CreateShaderModule(fragCode) };
	const VkPipeline pipeline{ vc.CreateSurfaceCompatiblePipeline(vert, frag, pipelineLayout) };
	vkDestroyShaderModule(vc.device, vert, nullptr);
	vkDestroyShaderModule(vc.device, frag, nullptr);

	const ImGuiHelper imGuiHelper{ vc };
	ne::NodeEditor nodeEditor{ ne::NodeEditor::MakeTestGraph() };

	while (!win.ShouldClose()) {
		win.PollEvents();

		imGuiHelper.Begin();
		nodeEditor.Draw();

		static bool showDemo{ true };
		ImGui::ShowDemoWindow(&showDemo);
		imGuiHelper.End();

		vc.DrawFrame(
			[&](const VkCommandBuffer& cmdBuf) {
				vkCmdBindPipeline(vc.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
				vkCmdDraw(vc.commandBuffer, 3, 1, 0, 0);
				imGuiHelper.AddDrawCalls(vc.commandBuffer);
			});
	}

	// Cleanup
	vkDeviceWaitIdle(vc.device);
	vkDestroyPipelineLayout(vc.device, pipelineLayout, nullptr);
	vkDestroyPipeline(vc.device, pipeline, nullptr);
	return 0;
}
