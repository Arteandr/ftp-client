#pragma once
#include <curl/curl.h>

#include <memory>
#include <string>

struct FTPInfo {
	std::string host;
	std::string username;
	std::string password;
	int port;
};

class FTP
{
public:
	FTP();
	~FTP();

	bool Connect(const FTPInfo& connectionInfo);
	bool Cleanup();

	void put(); // write
	void get(); // read
	void pwd(); // pwd
	void cwd(); // change wd
	void mkdir(); // ls
	bool List(const std::string& strRemoteFolder, std::string& strList, bool bOnlyNames);
	bool Connected() const;

private:
	inline CURLcode Perform() const;
	inline std::string ParseURL(const std::string& file) const;
	static void ReplaceString(std::string& strSubject, const std::string& strSearch, const std::string& strReplace);

private:
	mutable CURL *m_Connection;
	FTPInfo m_ConnectionInfo;
	bool m_bActive;
	
	// Callbacks
	static size_t WriteInStringCallback(void* ptr, size_t size, size_t nmemb, void* data);

};

