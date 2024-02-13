#pragma once

#include <string>
#include <functional>
#include "imgui.h"

using ClickButtonCallback = std::function<void()>;

using WidthPredicate = std::function<float(float, float)>;

class Window {
public:
	explicit Window(std::string title) : m_Title(title) { };
	virtual ~Window() {};

	virtual void Render() = 0;

	static inline void ColoredButton(const std::string& label, ImVec4 color, ImVec2 size, ClickButtonCallback callback = nullptr) {
		ImGui::PushStyleColor(ImGuiCol_Button, color);
		if (ImGui::Button(label.c_str(), size))
			if (callback != nullptr)
				callback();
		ImGui::PopStyleColor(1);
	}

	static inline void CenteredText(const std::string& label) {
		auto width = ImGui::GetWindowWidth();
		auto calc = ImGui::CalcTextSize(label.c_str());
		ImGui::SetCursorPosX(width / 2 - calc.x / 2);
		ImGui::Text(label.c_str());
	}
	
	static inline void BottomButton(const std::string& label, WidthPredicate pred, ClickButtonCallback callback = nullptr) {
		auto width = ImGui::GetWindowWidth();
		auto height = ImGui::GetWindowHeight();
		auto calc = ImGui::CalcTextSize(label.c_str());

		ImGui::SetCursorPosY(height - calc.y * 2.5);
		ImGui::SetCursorPosX(pred(width, calc.x));
		if (ImGui::Button(label.c_str()))
			if (callback != nullptr)
				callback();
	}

protected:
	std::string m_Title;
};
