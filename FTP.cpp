#include "FTP.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <fstream>

FTP::FTP(LogCallbackFn logFn)
:m_bActive(false),
m_Connection(nullptr),
m_LogCallback(logFn) { }

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

bool FTP::DownloadFile(const std::string& strRemoteFile, std::vector<char>& data) const {
	if (strRemoteFile.empty()) return false;
	if (!m_Connection) return false;

	curl_easy_reset(m_Connection);
	std::string strFile = ParseURL(strRemoteFile);

	data.clear();

	curl_easy_setopt(m_Connection, CURLOPT_URL, strFile.c_str());
	curl_easy_setopt(m_Connection, CURLOPT_WRITEFUNCTION, WriteToMemory);
	curl_easy_setopt(m_Connection, CURLOPT_WRITEDATA, &data);

	CURLcode res = Perform();
	

	return (CURLE_OK == res);
}

size_t FTP::WriteToMemory(void* buff, size_t size, size_t nmemb, void* data) {
	if ((size == 0) || (nmemb == 0) || ((size * nmemb) < 1) || (data == nullptr)) return 0;

	auto* vec = reinterpret_cast<std::vector<char> *>(data);
	size_t ssize = size * nmemb;
	std::copy((char*)buff, (char*)buff + ssize, std::back_inserter(*vec));

	return ssize;
}


bool FTP::Upload(FTP::CurlReadFn readFn, void* userData, const std::string& strRemoteFile,
	const bool& bCreateDir, curl_off_t fileSize) const {
	if (strRemoteFile.empty()) return false;
	if (!m_Connection) return false;

	curl_easy_reset(m_Connection);

	std::string strLocalRemoteFile = ParseURL(strRemoteFile);
	/* specify target */
	curl_easy_setopt(m_Connection, CURLOPT_URL, strLocalRemoteFile.c_str());

	/* we want to use our own read function */
	curl_easy_setopt(m_Connection, CURLOPT_READFUNCTION, readFn);

	/* now specify which file to upload */
	curl_easy_setopt(m_Connection, CURLOPT_READDATA, userData);

	/* Set the size of the file to upload (optional).  If you give a *_LARGE
  option you MUST make sure that the type of the passed-in argument is a
  curl_off_t. If you use CURLOPT_INFILESIZE (without _LARGE) you must
  make sure that to pass in a type 'long' argument. */
	curl_easy_setopt(m_Connection, CURLOPT_INFILESIZE_LARGE, fileSize);

	/* enable uploading */
	curl_easy_setopt(m_Connection, CURLOPT_UPLOAD, 1L);

	if (bCreateDir) curl_easy_setopt(m_Connection, CURLOPT_FTP_CREATE_MISSING_DIRS, CURLFTP_CREATE_DIR);
	CURLcode res = Perform();

	std::cout << curl_easy_strerror(res) << std::endl;

	return (CURLE_OK == res || (res != CURLE_OK && res == CURLE_PARTIAL_FILE));

}
bool FTP::Upload(std::istream& inputStream, const std::string& strRemoteFile, const bool& bCreateDir,
	curl_off_t fileSize) const {
	if (!inputStream)
		return false;

	return Upload(ReadFromStreamCallback, static_cast<void*>(&inputStream), strRemoteFile, bCreateDir, fileSize);
}
size_t FTP::ReadFromStreamCallback(void* ptr, size_t size, size_t nmemb, void* stream) {
	auto* pFileStream = reinterpret_cast<std::istream*>(stream);
	if (!pFileStream->fail()) {
		pFileStream->read(reinterpret_cast<char*>(ptr), size * nmemb);
		return pFileStream->gcount();
	}
	return 0;
 }
bool FTP::Delete(const std::string& strRemoteFile) const {
	if (strRemoteFile.empty()) return false;
	if (!m_Connection) return false;

	curl_easy_reset(m_Connection);
	
	struct curl_slist* headerlist = nullptr;
	std::string strRemoteFolder;
	std::string strRemoteFileName;
	std::string strBuf;

	std::size_t uFound = strRemoteFile.find_last_of("/");
	if (uFound != std::string::npos) {
		strRemoteFolder = ParseURL(strRemoteFile.substr(0, uFound)) + "//";
		strRemoteFileName = strRemoteFile.substr(uFound + 1);
	} else {
		strRemoteFolder = ParseURL("");
		strRemoteFileName = strRemoteFile;
	}

	strBuf += "DELE ";

	// Specify target
	curl_easy_setopt(m_Connection, CURLOPT_URL, strRemoteFolder.c_str());

	strBuf += strRemoteFileName;
	headerlist = curl_slist_append(headerlist, strBuf.c_str());

	curl_easy_setopt(m_Connection, CURLOPT_POSTQUOTE, headerlist);
	curl_easy_setopt(m_Connection, CURLOPT_NOBODY, 1L);
	curl_easy_setopt(m_Connection, CURLOPT_HEADER, 1L);
	CURLcode res = Perform();

	curl_slist_free_all(headerlist);

	return (CURLE_OK == res);
}

bool FTP::CreateDir(const std::string& strNewDir) const {
	if (strNewDir.empty()) return false;
	if (!m_Connection) return false;

	curl_easy_reset(m_Connection);

	struct curl_slist* headerlist = nullptr;
	std::string strRemoteFolder;
	std::string strRemoteNewFolderName;
	std::string strBuf;

	// Splitting folder name
	std::size_t uFound = strNewDir.find_last_of("/");
	if (uFound != std::string::npos) {
		strRemoteFolder = ParseURL(strNewDir.substr(0, uFound)) + "//";
		strRemoteNewFolderName = strNewDir.substr(uFound + 1);
	}
	else  // the dir. to be created is located in the root directory
	{
		strRemoteFolder = ParseURL("");
		strRemoteNewFolderName = strNewDir;
	}
	// Append the MKD command
	strBuf += "MKD ";

	// Specify target
	curl_easy_setopt(m_Connection, CURLOPT_URL, strRemoteFolder.c_str());

	strBuf += strRemoteNewFolderName;
	headerlist = curl_slist_append(headerlist, strBuf.c_str());
	curl_easy_setopt(m_Connection, CURLOPT_POSTQUOTE, headerlist);
	curl_easy_setopt(m_Connection, CURLOPT_NOBODY, 1L);
	curl_easy_setopt(m_Connection, CURLOPT_HEADER, 1L);
	curl_easy_setopt(m_Connection, CURLOPT_FTP_CREATE_MISSING_DIRS, CURLFTP_CREATE_DIR);

	/* enable TCP keep-alive for this transfer */
	curl_easy_setopt(m_Connection, CURLOPT_TCP_KEEPALIVE, 0L);

	CURLcode res = Perform();
	// clean up the FTP commands list
	curl_slist_free_all(headerlist);

	return (CURLE_OK == res);
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
	else
		std::cerr << "Error: " << curl_easy_strerror(res) << std::endl;


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
	std::string baseUrl = m_ConnectionInfo.host + remoteFilename;

	ReplaceString(baseUrl, " ", "%20");

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

int FTP::DebugCallback(CURL* curl, curl_infotype curl_info_type, char* pszTrace, size_t usSize, void* clientP) {
	std::string strText;
	std::string strTrace(pszTrace, usSize);

	switch (curl_info_type) {
	case CURLINFO_TEXT:
		strText = "# Information : ";
		break;
	case CURLINFO_HEADER_OUT:
		strText = "-> Sending header : ";
		break;
	case CURLINFO_DATA_OUT:
		strText = "-> Sending data : ";
		break;
	case CURLINFO_HEADER_IN:
		strText = "<- Receiving header : ";
		break;
	default:
		break;
	}
	// я не знаю как привести void* в LogCallbackFN
	((LogCallbackFn*)clientP)->operator()(strText + strTrace);


	return 0;
}

CURLcode FTP::Perform() const {
	CURLcode res = CURLE_OK;

	curl_easy_setopt(m_Connection, CURLOPT_PORT, m_ConnectionInfo.port);
	curl_easy_setopt(m_Connection, CURLOPT_USERPWD, (m_ConnectionInfo.username + ":" + m_ConnectionInfo.password).c_str());
	curl_easy_setopt(m_Connection, CURLOPT_VERBOSE, 1L);
	curl_easy_setopt(m_Connection, CURLOPT_DEBUGFUNCTION, DebugCallback);
	curl_easy_setopt(m_Connection, CURLOPT_DEBUGDATA, this->m_LogCallback);

	curl_easy_setopt(m_Connection, CURLOPT_FTPPORT, "-");

	// Perform the requested operation
	res = curl_easy_perform(m_Connection);

	return res;
}

std::string FTP::AnsiToUtf8(const std::string& codepage_str) {
	// Transcode Windows ANSI to UTF-16
	int size = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, codepage_str.c_str(), codepage_str.length(), nullptr, 0);
	std::wstring utf16_str(size, '\0');
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, codepage_str.c_str(), codepage_str.length(), &utf16_str[0], size);

	// Transcode UTF-16 to UTF-8
	int utf8_size = WideCharToMultiByte(CP_UTF8, 0, utf16_str.c_str(), utf16_str.length(), nullptr, 0, nullptr, nullptr);
	std::string utf8_str(utf8_size, '\0');
	WideCharToMultiByte(CP_UTF8, 0, utf16_str.c_str(), utf16_str.length(), &utf8_str[0], utf8_size, nullptr, nullptr);

	return utf8_str;
}

std::wstring FTP::Utf8ToUtf16(const std::string& str) {
	std::wstring ret;
	int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), NULL, 0);
	if (len > 0) {
		ret.resize(len);
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), &ret[0], len);
	}
	return ret;
}
