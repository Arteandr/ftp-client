#include "LogsWindow.h"
#pragma execution_character_set("utf-8")

#include "imgui.h"
#include "imgui_stdlib.h"
#include "imgui_internal.h"

void LogsWindow::Render() {
	ImGuiWindowClass winClass;
	winClass.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoWindowMenuButton | ImGuiDockNodeFlags_NoTabBar;
	ImGui::SetNextWindowClass(&winClass);
	ImGui::Begin(m_Title.c_str());
	{
		for (const auto& log : m_Logs)
			ImGui::Text(log.c_str());
	}
	ImGui::End();
}

void LogsWindow::ScrollToBottom() {
	ImGuiWindow* win = ImGui::FindWindowByName(m_Title.c_str());
	if (win) {
		ImGui::SetWindowFocus(m_Title.c_str());
		ImGui::SetScrollHereY(1.0f);
	}
}
