#pragma once

#include "Window.h"
#include <string>
#include <vector>
#include <functional>
#include <utils/Utils.hpp>

struct FileRecord {
	std::string m_Name;
	bool m_IsDirectory = false;

	std::string GetFull() const;

	static FileRecord Parse(std::string &recordString);
};

using OnClickCallback_t = std::function<void()>;
using RecordOnClickCallback_t = std::function<void(const FileRecord&)>;
using RecordOnClickDeleteCallback_t = std::function<void(const FileRecord&)>;
using RecordOnClickDownloadCallback_t = std::function<void(const FileRecord&)>;
using RecordOnClickUploadCallback_t = std::function<void()>;
using RecordOnClickRefreshCallback_t = std::function<void()>;

class FileBrowserWindow : public Window {
public:
	FileBrowserWindow(std::string title, std::string status, std::string defaultPath = "/") :
		m_StatusLineHost(status),
		m_CurrentPath(defaultPath),
		Window(title) {}
	virtual void Render() override;

	inline std::string GetCurrentPath() const { return m_CurrentPath; }
	inline void AppendPath(const std::string& path) { m_CurrentPath += path + "/"; }
	void TakeLatestFromPath();
	inline void SetHostname(const std::string& hostname) { m_Hostname = hostname; }
	inline void AppendRecord(const std::string& record, bool isDir) { m_FileRecords.push_back({ record, isDir }); }
	inline void ClearRecords() { m_FileRecords.clear(); }
	inline const FileRecord& GetSelectedRecord() const { return m_SelectedRecord; }
	inline void SetSelectedRecord(FileRecord selectedRecord) { m_SelectedRecord = selectedRecord; };

	inline void SetRecordOnClickCallback(RecordOnClickCallback_t callback) { m_RecordOnClickCallback = callback; }
	inline void SetRecordOnClickUploadCallback(RecordOnClickUploadCallback_t callback) { m_RecordOnClickUploadCallback = callback; }
	inline void SetRecordOnClickDeleteCallback(RecordOnClickDeleteCallback_t callback) { m_RecordOnClickDeleteCallback = callback; }
	inline void SetRecordOnClickDownloadCallback(RecordOnClickDownloadCallback_t callback) { m_RecordOnClickDownloadCallback = callback; }
	inline void SetRecordOnClickRefreshCallback(RecordOnClickRefreshCallback_t callback) { m_RecordOnClickRefreshCallback = callback; }
	inline void SetOnClickQuitCallback(OnClickCallback_t callback) { m_OnClickQuitCallback = callback; }
	inline void SetOnClickCreateDirCallback(OnClickCallback_t callback) { m_OnClickCreateDirCallback = callback; }
private:
	std::string GetStatusLine();
private:
	std::string m_StatusLineHost;
	std::string m_Hostname = "";
	std::string m_CurrentPath;
	std::vector<FileRecord> m_FileRecords;
	FileRecord m_SelectedRecord;

	RecordOnClickCallback_t m_RecordOnClickCallback;
	RecordOnClickRefreshCallback_t m_RecordOnClickRefreshCallback;
	RecordOnClickUploadCallback_t m_RecordOnClickUploadCallback;
	RecordOnClickDeleteCallback_t m_RecordOnClickDeleteCallback;
	RecordOnClickDownloadCallback_t m_RecordOnClickDownloadCallback;
	OnClickCallback_t m_OnClickQuitCallback;
	OnClickCallback_t m_OnClickCreateDirCallback;

	ImVec4 m_PrimaryColor = ImGui::RGBAtoIV4(51, 32, 171, 1.0f);
	ImVec4 m_SecondaryColor = {.0f, .332f, .195f, 1.0f };
};

