#pragma once
#include <curl/curl.h>

#include <memory>
#include <string>
#include <vector>
#include <functional>

struct FTPInfo {
	std::string host;
	std::string username;
	std::string password;
	int port;
};

class FTP
{
	using CurlReadFn = std::size_t(*) (void*, std::size_t, std::size_t, void*);
	using LogCallbackFn = std::function<void(std::string)>;
public:
	FTP(LogCallbackFn);
	~FTP();


	bool Connect(const FTPInfo& connectionInfo);
	bool Cleanup();
	bool CreateDir(const std::string& strNewDir) const;
	bool Upload(std::istream& inputStream, const std::string& strRemoteFile, const bool& bCreateDir, curl_off_t fileSize) const;
	bool Upload(FTP::CurlReadFn readFn, void* userData, const std::string& strRemoteFile,
		const bool& bCreateDir, curl_off_t fileSize) const;
	bool Delete(const std::string& strRemoteFile) const;
	bool DownloadFile(const std::string& strRemoteFile, std::vector<char> &data) const;
	bool List(const std::string& strRemoteFolder, std::string& strList, bool bOnlyNames);
	bool Connected() const;

	inline bool IsActive() const { return m_bActive; }
	inline void SetActive(bool isActive) { m_bActive = isActive; }


private:
	inline CURLcode Perform() const;
	inline std::string ParseURL(const std::string& file) const;
	static void ReplaceString(std::string& strSubject, const std::string& strSearch, const std::string& strReplace);

	static std::string AnsiToUtf8(const std::string& ansiStr);
	static std::wstring Utf8ToUtf16(const std::string& str);

	static size_t WriteToMemory(void* ptr, size_t size, size_t nmemb, void* data);
private:
	mutable CURL *m_Connection;
	FTPInfo m_ConnectionInfo;
	bool m_bActive;
	
	// Callbacks
	static size_t WriteInStringCallback(void* ptr, size_t size, size_t nmemb, void* data);
	static size_t ReadFromStreamCallback(void* ptr, size_t size, size_t nmemb, void* stream);
	static int DebugCallback(CURL* curl, curl_infotype curl_info_type, char* pszTrace, size_t usSize, void* clientP);

	LogCallbackFn m_LogCallback;
};

