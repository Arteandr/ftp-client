#pragma once
#include <UI/Layer.h>
#include <UI/FileBrowserWindow.h>
#include <UI/LogsWindow.h>
#include "FTP.h"

#include "imgui.h"
#include <string>

class ClientLayer : public UI::Layer
{
public:
	virtual void OnUIRender() override;
	virtual void OnAttach() override;
private:
	void UI_ConnectionModal();
	void UI_ConnectionModalConnectButtonHandler();
	inline bool UI_ConnectionFormIsEmpty() const { return m_IP.size() < 0 || m_Username.size() < 1 || m_Password.size() < 1; }

	void UI_FileBrowsers();
	void UI_ClientFileBrowser();
	void UI_RefreshServerFiles();

	void UI_CreateDirModal();

	void UI_ServerRecordOnClickHandler(const FileRecord& record);
	void UI_ServerRecordOnClickDownloadHandler(const FileRecord& record);
	void UI_ServerRecordOnClickDeleteHandler(const FileRecord& record);
	void UI_ServerRecordOnClickUploadHandler();
	void UI_ServerOnClickRefreshHandler();
	void UI_ServerOnClickQuitHandler();
	void UI_ServerOnClickCreateDirHandler();
	void UI_FileSaveCallback();
	void UI_FileUploadCallback();

	void UI_CreateDir();
private:
	bool m_ConnectionModalOpen = false;
	bool m_CreateDirModalOpen = false;
	bool m_FileBrowsersInit = false;

	std::string m_IP = "localhost";
	int m_Port = 21;
	std::string m_Username = "root";
	std::string m_Password = "root";

	std::string m_CreateNewDirName;

	FTP *m_FTPClient;

	LogsWindow *m_LogsWindow;
	FileBrowserWindow *m_ServerFileBrowserWindow;
};

