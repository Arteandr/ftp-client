#pragma once


#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <windows.h>


namespace fs = std::filesystem;

constexpr int INFO_BUFFER_SIZE = 32767;

namespace Utils {
	static class String {
	public:
		static inline std::vector<std::string> split(const std::string& s, char delim) {
			std::stringstream ss(s);
			std::string item;
			std::vector<std::string> elems;
			while (std::getline(ss, item, delim)) {
				elems.push_back(item);
				// elems.push_back(std::move(item)); // if C++11 (based on comment from @mchiasson)
			}
			return elems;
		}
	};

	static class PC {
	public:
		static inline std::string GetHostname() {
			char buffer[MAX_COMPUTERNAME_LENGTH + 1];
			DWORD size = sizeof(buffer);
			if (GetComputerNameA(buffer, &size)) {
				std::string name(buffer, size);
				return name;
			}

			return "";
		}

		static inline bool SaveFile(const std::string& path, std::vector<char>& buffer) {
			std::ofstream f;
			f.open(path, std::ios::ios_base::trunc);
			for (const auto& c : buffer)
				f << c;

			return 0;
		}

		static inline bool UploadFile(const std::string& path, std::ifstream& file) {
			file.open(path);

			return file.is_open();
		}
	};
}

namespace ImGui {
	static inline ImVec4 RGBAtoIV4(int r, int g, int b, int a) {
		float newr = r / static_cast<float>(255);
		float newg = g / static_cast<float>(255);
		float newb = b / static_cast<float>(255);
		float newa = a;
		return ImVec4(newr, newg, newb, newa);
	}
}
