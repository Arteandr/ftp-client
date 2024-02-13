#include "FileBrowserWindow.h"
#pragma execution_character_set("utf-8")

#include "vendor/imgui_notify/IconsFontAwesome6.h"
#include "utils/Utils.hpp"

#include "imgui.h"
#include "imgui_stdlib.h"
#include "imgui_internal.h"

#include <sstream>
#include <algorithm>
#include <numeric>

bool BothAreSpaces(char lhs, char rhs) { return (lhs == rhs) && (lhs == ' '); }

std::string FileRecord::GetFull() const {
	std::stringstream ss("");
	ss << (m_IsDirectory ? ICON_FA_FOLDER_CLOSED : ICON_FA_FILE) << " ";
	ss << m_Name;
	std::string temp = ss.str();

	return temp;
}

FileRecord FileRecord::Parse(std::string& recordString) {
	FileRecord record;
	record.m_IsDirectory = recordString[0] == 'd';

	std::string::iterator new_end = std::unique(recordString.begin(), recordString.end(), BothAreSpaces);
	recordString.erase(new_end, recordString.end());

	auto splittedRecord = Utils::String::split(recordString, ' ');
	splittedRecord.erase(splittedRecord.begin(), splittedRecord.begin() + 8);
	std::string name = std::accumulate(std::next(splittedRecord.begin()), splittedRecord.end(), splittedRecord[0], [](std::string a, std::string b) { return a + ' ' + b; });
	name.erase(std::remove(name.begin(), name.end(), '\r'), name.end());
	record.m_Name = name;

	return record;
}

void FileBrowserWindow::Render() {
	ImGuiWindowClass winClass;
	winClass.ViewportFlagsOverrideClear = ImGuiViewportFlags_NoDecoration;
	winClass.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoWindowMenuButton | ImGuiDockNodeFlags_NoTabBar;

	ImGui::SetNextWindowClass(&winClass);
	ImGui::Begin(m_Title.c_str());
	{
		Window::ColoredButton(ICON_FA_POWER_OFF, ImGui::RGBAtoIV4(196, 33, 44, 1), ImVec2{0.0f, 0.0f}, m_OnClickQuitCallback);
		ImGui::SameLine();

		Window::ColoredButton(ICON_FA_REFRESH, m_SecondaryColor, ImVec2{0.0f, 0.0f}, m_RecordOnClickRefreshCallback);
		ImGui::SameLine();
		Window::CenteredText(GetStatusLine());
		ImGui::Text("\n");

		 if (!m_FileRecords.empty())
			std::sort(m_FileRecords.begin(), m_FileRecords.end(), [](const FileRecord& a, const FileRecord& b) { return a.m_IsDirectory > b.m_IsDirectory; });

		if (GetCurrentPath() != "/")
			Window::ColoredButton("..", ImVec4{ .0f,.632f,.195f,1.0f }, ImVec2(ImGui::GetWindowSize().x * 0.10f, 0.0f), [&]() {
				TakeLatestFromPath();
				if (m_RecordOnClickCallback)
					m_RecordOnClickCallback({});
			});
		
		int i = 0;
		for (const FileRecord& record : m_FileRecords) {
			Window::ColoredButton(record.GetFull(),
				record.m_IsDirectory ? m_PrimaryColor : m_SecondaryColor,
				ImVec2(ImGui::GetWindowSize().x * 0.70f, 0.0f), [&]() {
					if (m_RecordOnClickCallback)
						m_RecordOnClickCallback(record);
			});

			if (!record.m_IsDirectory) {
				ImGui::SameLine();
				ImGui::PushID(++i);
				Window::ColoredButton(ICON_FA_DOWNLOAD, ImVec4{ .0f,.632f,.195f,1.0f }, ImVec2(ImGui::GetWindowSize().x * 0.10f, 0.0f), [&]() {
					SetSelectedRecord(record);
					m_RecordOnClickDownloadCallback(record);
					});
				ImGui::PopID();
			}

			if (!record.m_IsDirectory) {
				ImGui::SameLine();
				ImGui::PushID(++i);
				Window::ColoredButton(ICON_FA_TRASH_CAN, ImVec4{ 1.0f,.2,.195f,1.0f }, ImVec2(ImGui::GetWindowSize().x * 0.10f, 0.0f), [&]() {
					m_RecordOnClickDeleteCallback(record);
				});
				ImGui::PopID();
			}
		}
		
		float prevcalc = 0;
		Window::BottomButton(ICON_FA_FILE_ARROW_UP " Загрузить файл", [&](float windowWidth, float calc) {
			prevcalc = calc + 10;
			return 10;
			}, [&]() { m_RecordOnClickUploadCallback(); });
		ImGui::SameLine();
		Window::BottomButton(ICON_FA_PLUS " Создать папку", [&](float windowWidth, float calc) {
			return prevcalc + 25;
			}, [&]() { m_OnClickCreateDirCallback(); });
	}
	ImGui::End();
}

std::string FileBrowserWindow::GetStatusLine() {
	std::stringstream ss("");
	ss << m_StatusLineHost << ": ";

	if (!m_Hostname.empty())
		ss << m_Hostname;

	ss << m_CurrentPath;
	std::string temp = ss.str();
	return temp;
}

void FileBrowserWindow::TakeLatestFromPath() {
	m_CurrentPath.pop_back();

	auto pos = m_CurrentPath.rfind('/');
	if (pos == std::string::npos) return;

	m_CurrentPath.erase(pos+1);
}
