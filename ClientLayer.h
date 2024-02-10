#pragma once
#include <UI/Layer.h>
#include "FTP.h"

#include "imgui.h"
#include <string>

class ClientLayer : public UI::Layer
{
public:
	virtual void OnUIRender() override;
private:
	void UI_ConnectionModal();
	void UI_ConnectionModalConnectButtonHandler();
private:
	bool m_ConnectionModalOpen = false;
	float m_ColorBuffer[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	std::string m_IP;
	int m_Port = 21;
	std::string m_Username;
	std::string m_Password;

	inline bool UI_ConnectionFormIsEmpty() const { return m_IP.size() < 0 || m_Username.size() < 1 || m_Password.size() < 1; }

	FTP *m_FTPClient;
};

