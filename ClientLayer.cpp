#include "ClientLayer.h"
#pragma execution_character_set("utf-8")

#include "imgui_stdlib.h"
#include <cpr/cpr.h>
#include "UI/FileBrowserWindow.h"
#include "UI/LogsWindow.h"
#include <vendor/imgui_notify/ImGuiNotify.hpp>
#include <vendor/dirent/dirent.h>
#include <vendor/imgui_dialog/ImGuiFileDialog.h>
#include <utils/Utils.hpp>

#include <thread>
#include <future>
#include <iostream>
#include <algorithm>

void ClientLayer::OnAttach() {
	m_FTPClient = new FTP([&](const std::string& msg) {
		std::cout << msg << std::endl;
		m_LogsWindow->AppendLog(msg);
		m_LogsWindow->ScrollToBottom();
	});

	m_LogsWindow = new LogsWindow("Логи");

	m_ServerFileBrowserWindow = new FileBrowserWindow("Файлы сервера", "Удаленный хост");
	m_ServerFileBrowserWindow->SetRecordOnClickCallback([&](const FileRecord& record) { UI_ServerRecordOnClickHandler(record); });
	m_ServerFileBrowserWindow->SetRecordOnClickUploadCallback([&]() { UI_ServerRecordOnClickUploadHandler(); });
	m_ServerFileBrowserWindow->SetRecordOnClickDownloadCallback([&](const FileRecord& record) { UI_ServerRecordOnClickDownloadHandler(record); });
	m_ServerFileBrowserWindow->SetRecordOnClickDeleteCallback([&](const FileRecord& record) { UI_ServerRecordOnClickDeleteHandler(record); });
	m_ServerFileBrowserWindow->SetRecordOnClickRefreshCallback([&]() { UI_ServerOnClickRefreshHandler(); });
	m_ServerFileBrowserWindow->SetOnClickQuitCallback([&]() { UI_ServerOnClickQuitHandler(); });
	m_ServerFileBrowserWindow->SetOnClickCreateDirCallback([&]() { UI_ServerOnClickCreateDirHandler(); });
}

void ClientLayer::OnUIRender() {
	UI_ConnectionModal();
	UI_FileBrowsers();
	UI_ClientFileBrowser();
	UI_CreateDirModal();
}

void ClientLayer::UI_CreateDirModal() {
	if (!m_CreateDirModalOpen) return;

	ImGui::OpenPopup("Создание новой директории");

	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	m_CreateDirModalOpen = ImGui::BeginPopupModal("Создание новой директории", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);
	if (m_CreateDirModalOpen) {
		ImGui::Text("Название новой директории");
		ImGui::InputText("##dirName", &m_CreateNewDirName);

		ImGui::PushStyleColor(ImGuiCol_Button, { 0,0.5f,0,1 });
		if (ImGui::Button("Создать") && m_CreateNewDirName.size() > 0)
			UI_CreateDir();
		ImGui::PopStyleColor(1);

		ImGui::SameLine();
		if (ImGui::Button("Отменить")) {
			m_CreateNewDirName.clear();
			m_CreateDirModalOpen = false;
		}

		if (!m_CreateDirModalOpen && m_CreateNewDirName.size() < 1)
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}
}

void ClientLayer::UI_CreateDir() {
	if (!m_FTPClient->CreateDir(m_ServerFileBrowserWindow->GetCurrentPath() + m_CreateNewDirName)) {
		ImGui::InsertNotification({ ImGuiToastType::Error, 2000, "Возникла ошибка при создании директории"});
		return;
	}
	
	std::string strOut = "Директория " + m_CreateNewDirName + " успешно создана";
	ImGui::InsertNotification({ ImGuiToastType::Success, 2000, strOut.c_str() });
	m_CreateNewDirName.clear();
	m_CreateDirModalOpen = false;
	UI_RefreshServerFiles();
}

void ClientLayer::UI_ClientFileBrowser() {
	if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKeySave")) {
		if (ImGuiFileDialog::Instance()->IsOk())
			UI_FileSaveCallback();

		ImGuiFileDialog::Instance()->Close();
	}

	if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKeyUpload")) {
		if (ImGuiFileDialog::Instance()->IsOk())
			UI_FileUploadCallback();

		ImGuiFileDialog::Instance()->Close();
	}
}

void ClientLayer::UI_FileSaveCallback() {
	auto record = m_ServerFileBrowserWindow->GetSelectedRecord();
	std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
	std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
	std::vector<char> data;
	m_FTPClient->DownloadFile(m_ServerFileBrowserWindow->GetCurrentPath() + record.m_Name, data);
	Utils::PC::SaveFile(filePath, data);
	std::string outStr = "Файл " + record.m_Name + " успешно загружен";
	ImGui::InsertNotification({ ImGuiToastType::Success, 2000, outStr.c_str()});
}

void ClientLayer::UI_FileUploadCallback() {
	std::ifstream file;
	std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
	std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
	if (!Utils::PC::UploadFile(filePathName, file)) {
		ImGui::InsertNotification({ ImGuiToastType::Error, 2000, "Возникла ошибка при загрузке файла"});
		return;
	}
	
	if (!m_FTPClient->Upload(file, m_ServerFileBrowserWindow->GetCurrentPath() + filePathName, true, file.tellg())) {
		ImGui::InsertNotification({ ImGuiToastType::Error, 2000, "Возникла ошибка при передаче файла"});
		return;
	}
	
	std::string strOut = "Файл " + filePathName + " успешно загружен";
	ImGui::InsertNotification({ ImGuiToastType::Success, 2000, strOut.c_str() });
	UI_RefreshServerFiles();
}

void ClientLayer::UI_ConnectionModal() {
	if (!m_ConnectionModalOpen && !m_FTPClient->IsActive()) {
		ImGui::OpenPopup("Подключение к FTP");
	}

	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	m_ConnectionModalOpen = ImGui::BeginPopupModal("Подключение к FTP", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove );
	if (m_ConnectionModalOpen) {
		ImGui::Text("FTP адресс");
		ImGui::InputText("##ip", &m_IP);

		ImGui::Text("Порт");
		ImGui::InputInt("##port", &m_Port);

		ImGui::Text("Имя пользователя");
		ImGui::InputText("##username", &m_Username);

		ImGui::Text("Пароль");
		ImGui::InputText("##password", &m_Password, ImGuiInputTextFlags_Password);
		ImGui::Text("\n");

		ImGui::PushStyleColor(ImGuiCol_Button, { 0,0.5f,0,1 });
		if (ImGui::Button("Подключиться"))
			UI_ConnectionModalConnectButtonHandler();
		ImGui::PopStyleColor(1);

		if (m_FTPClient->IsActive())
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}
}

void ClientLayer::UI_ConnectionModalConnectButtonHandler() {
	if (UI_ConnectionFormIsEmpty()) {
		ImGui::InsertNotification({ ImGuiToastType::Error, 2000, "Необходимо заполнить все поля" });
		return;
	};

	FTPInfo *info = new FTPInfo{ m_IP, m_Username, m_Password, m_Port };
	std::thread{ [&]() {
		m_FTPClient->Connect(*info);
		if (!m_FTPClient->Connected()) {
			m_FTPClient->Cleanup();
			ImGui::InsertNotification({ ImGuiToastType::Error, 2000, "Ошибка аутентификации" });
			delete info;
			return;
		};
		m_FTPClient->SetActive(true);
		ImGui::InsertNotification({ ImGuiToastType::Success, 2000, "Успешная аутентификация" });
		m_ServerFileBrowserWindow->SetHostname(m_IP);
		UI_RefreshServerFiles();
	} }.detach();
}

void ClientLayer::UI_ServerOnClickRefreshHandler() {
	UI_RefreshServerFiles();
}

void ClientLayer::UI_ServerOnClickQuitHandler() {
	m_FTPClient->Cleanup();
	m_FTPClient->SetActive(false);
	ImGui::InsertNotification({ ImGuiToastType::Warning, 2000, "Успешных выход из сессии"});
}

void ClientLayer::UI_ServerOnClickCreateDirHandler() {
	m_CreateDirModalOpen = true;
}

void ClientLayer::UI_RefreshServerFiles() {
	if (m_FTPClient == nullptr) return;
	
	m_ServerFileBrowserWindow->ClearRecords();
	std::string outStr;
	if (!m_FTPClient->List(m_ServerFileBrowserWindow->GetCurrentPath(), outStr, false)) {
		std::cerr << "Cannot do list request" << std::endl;
		return;
	}
	if (outStr.length() < 1) return;

	std::vector<std::string> recordNames = Utils::String::split(outStr, '\n');
	for (auto& recordStr : recordNames) {
		auto record = FileRecord::Parse(recordStr);
		m_ServerFileBrowserWindow->AppendRecord(record.m_Name, record.m_IsDirectory);
	}
}

void ClientLayer::UI_FileBrowsers() {
	if (!m_FTPClient->IsActive()) return;
	
	if (m_FileBrowsersInit) {
		auto dockspaceID = ImGui::GetID("MyDockspace");
		ImGui::DockBuilderRemoveNode(dockspaceID); // clear any previous layout
		ImGui::DockBuilderAddNode(dockspaceID, ImGuiDockNodeFlags_DockSpace); // add empty node
		ImGui::DockBuilderSetNodeSize(dockspaceID, ImGui::GetMainViewport()->Size); // set node siz
		auto dock_id_top = ImGui::DockBuilderSplitNode(dockspaceID, ImGuiDir_Up, 0.2f, nullptr, &dockspaceID); // split node into top and bottome
		ImGui::DockBuilderDockWindow("Логи", dock_id_top); // dock window C to the top node
		ImGui::DockBuilderDockWindow("Файлы сервера", dockspaceID); // dock window B to the right node
		ImGui::DockBuilderFinish(dockspaceID); // finish the layout	
		m_FileBrowsersInit = true;
	}
	
	m_LogsWindow->Render();
	m_ServerFileBrowserWindow->Render();
}

void ClientLayer::UI_ServerRecordOnClickHandler(const FileRecord& record) {
	if (record.m_IsDirectory)
		m_ServerFileBrowserWindow->AppendPath(record.m_Name);

	UI_RefreshServerFiles();
}


void ClientLayer::UI_ServerRecordOnClickDownloadHandler(const FileRecord& record) {
	IGFD::FileDialogConfig config;
	config.path = ".";
	config.fileName = record.m_Name;
	config.flags = ImGuiFileDialogFlags_Modal;
	ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKeySave", std::string(ICON_FA_FOLDER_OPEN) + " Загрузка файла", ".*", config);
}

void ClientLayer::UI_ServerRecordOnClickDeleteHandler(const FileRecord& record) {
	if (!m_FTPClient->Delete(m_ServerFileBrowserWindow->GetCurrentPath() + record.m_Name)) {
		ImGui::InsertNotification({ ImGuiToastType::Error, 2000, "Ошибка при удалении" });
		return;
	}
	std::string strOut = "Файл " + record.m_Name + " успешно удален";
	ImGui::InsertNotification({ ImGuiToastType::Success, 2000, strOut.c_str()});
	UI_RefreshServerFiles();
}


void ClientLayer::UI_ServerRecordOnClickUploadHandler() {
	IGFD::FileDialogConfig config;
	config.path = ".";
	config.flags = ImGuiFileDialogFlags_Modal;
	ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKeyUpload", std::string(ICON_FA_FOLDER_OPEN) + " Загрузка файла", ".*", config);
}
