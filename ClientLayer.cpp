#include "ClientLayer.h"
#pragma execution_character_set("utf-8")

#include <iostream>
#include "imgui_stdlib.h"
#include <cpr/cpr.h>
#include <vendor/imgui_notify/ImGuiNotify.hpp>
#include <thread>
#include <future>

void ClientLayer::OnUIRender() {

	UI_ConnectionModal();
}

void ClientLayer::UI_ConnectionModal() {
	if (!m_ConnectionModalOpen) {
		ImGui::OpenPopup("Подключение к FTP");
	}

	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	m_ConnectionModalOpen = ImGui::BeginPopupModal("Подключение к FTP", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);
	if (m_ConnectionModalOpen) {
		ImGui::Text("FTP адресс");
		ImGui::InputText("##ip", &m_IP);

		ImGui::Text("Порт");
		ImGui::InputInt("##port", &m_Port);

		ImGui::Text("Имя пользователя");
		ImGui::InputText("##username", &m_Username);

		ImGui::Text("Пароль");
		ImGui::InputText("##password", &m_Password, ImGuiInputTextFlags_Password);
		ImGui::Text("\n");

		ImGui::PushStyleColor(ImGuiCol_Button, { 0,0.5f,0,1 });
		if (ImGui::Button("Подключиться"))
			UI_ConnectionModalConnectButtonHandler();

		ImGui::PopStyleColor(1);

		ImGui::EndPopup();
	}
}

void ClientLayer::UI_ConnectionModalConnectButtonHandler() {
	if (UI_ConnectionFormIsEmpty()) {
		ImGui::InsertNotification({ ImGuiToastType::Error, 2000, "Необходимо заполнить все поля" });
		return;
	};

	FTPInfo *info = new FTPInfo{ m_IP, m_Username, m_Password, m_Port };
	/*
	auto fut = std::async(std::launch::async, [&]() {
		m_FTPClient = new FTP();
		m_FTPClient->Connect(*info);
		if (!m_FTPClient->Connected()) {
			m_FTPClient->Cleanup();
			ImGui::InsertNotification({ ImGuiToastType::Error, 2000, "Ошибка аутентификации" });
			delete info;
			delete m_FTPClient;
			return;
		};
		ImGui::CloseCurrentPopup();
		m_ConnectionModalOpen = false;
	}); */

	std::thread{ [&]() {
		m_FTPClient = new FTP();
		m_FTPClient->Connect(*info);
		if (!m_FTPClient->Connected()) {
			m_FTPClient->Cleanup();
			ImGui::InsertNotification({ ImGuiToastType::Error, 2000, "Ошибка аутентификации" });
			delete info;
			delete m_FTPClient;
			return;
		};
		ImGui::CloseCurrentPopup();
		m_ConnectionModalOpen = false;
	} }.detach();
}
