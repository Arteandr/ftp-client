#include "FTP.h"

#include <algorithm>
#include <iostream>

FTP::FTP()
:m_bActive(true),
m_Connection(nullptr) { }

FTP::~FTP() {
	if (m_Connection != nullptr)
		Cleanup();
}

bool FTP::Connected() const {
	if (!m_Connection) return false;
	CURLcode res = CURLE_GOT_NOTHING;

	curl_easy_reset(m_Connection);
	
	std::string baseUrl = "ftp://" + m_ConnectionInfo.host;
	curl_easy_setopt(m_Connection, CURLOPT_URL, baseUrl.c_str());
	res = Perform();

	return (CURLE_OK == res);
}

bool FTP::Cleanup() {
	if (!m_Connection) return false;

	curl_easy_cleanup(m_Connection);
	m_Connection = nullptr;

	return true;
}

bool FTP::List(const std::string& strRemoteFolder, std::string& strList, bool bOnlyNames) {
	if (strRemoteFolder.empty()) return false;
	if (!m_Connection) return false;

	curl_easy_reset(m_Connection);

	bool bRet = false;
	std::string strRemoteFile = ParseURL(strRemoteFolder);
	curl_easy_setopt(m_Connection, CURLOPT_URL, strRemoteFile.c_str());
	if (bOnlyNames) curl_easy_setopt(m_Connection, CURLOPT_DIRLISTONLY, 1L);

	curl_easy_setopt(m_Connection, CURLOPT_WRITEFUNCTION, WriteInStringCallback);
	curl_easy_setopt(m_Connection, CURLOPT_WRITEDATA, &strList);

	CURLcode res = Perform();

	if (CURLE_OK == res)
		bRet = true;

	return bRet;

}

size_t FTP::WriteInStringCallback(void* ptr, size_t size, size_t nmemb, void* data) {
	std::string* strWriteHere = reinterpret_cast<std::string*>(data);
	if (strWriteHere != nullptr) {
		strWriteHere->append(reinterpret_cast<char*>(ptr), size * nmemb);
		return size * nmemb;
	}
	return 0;
}

std::string FTP::ParseURL(const std::string& remoteFilename) const {
	std::string baseUrl = m_ConnectionInfo.host + "/" + remoteFilename;

	ReplaceString(baseUrl, "/", "//");
	ReplaceString(baseUrl, " ", "%20"); //fixes folders with spaces not working

	return "ftp://" + baseUrl;
}

bool FTP::Connect(const FTPInfo& connectionInfo) {
	if (m_Connection)
		return false;

	m_Connection = curl_easy_init();
	m_ConnectionInfo = connectionInfo;

	return (m_Connection != nullptr);
}

void FTP::ReplaceString(std::string& strSubject, const std::string& strSearch, const std::string& strReplace) {
	if (strSearch.empty()) return;

	size_t pos = 0;
	while ((pos = strSubject.find(strSearch, pos)) != std::string::npos) {
		strSubject.replace(pos, strSearch.length(), strReplace);
		pos += strReplace.length();
	}
}

CURLcode FTP::Perform() const {
	CURLcode res = CURLE_OK;

	curl_easy_setopt(m_Connection, CURLOPT_PORT, m_ConnectionInfo.port);
	curl_easy_setopt(m_Connection, CURLOPT_USERPWD, (m_ConnectionInfo.username + ":" + m_ConnectionInfo.password).c_str());

	if (m_bActive) curl_easy_setopt(m_Connection, CURLOPT_FTPPORT, "-");

#ifdef DEBUG_CURL
	StartCurlDebug();
#endif

	// Perform the requested operation
	res = curl_easy_perform(m_Connection);

#ifdef DEBUG_CURL
	EndCurlDebug();
#endif

	return res;
}
