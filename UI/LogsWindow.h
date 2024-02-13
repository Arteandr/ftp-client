#pragma once
#include "Window.h"

#include <deque>
#include <string>
#include "utils/Utils.hpp"

class LogsWindow : public Window {
public:
	LogsWindow(std::string title) : Window(title) {}
	virtual void Render() override;
	
	inline void AppendLog(const std::string& logMessage) { 
		m_Logs.push_front(logMessage);
	}

	void ScrollToBottom();


private:
	std::deque<std::string> m_Logs;
};
