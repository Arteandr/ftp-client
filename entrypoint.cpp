#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <UI/Application.h>
#include <ClientLayer.h>

#include <vendor/imgui_notify/IconsFontAwesome6.h>
#include <vendor/imgui_notify/ImGuiNotify.hpp>
#include <iostream>
#include <fstream>

int main(void)
{
    GLFWwindow* window;

    if (!glfwInit())
        return -1;

    window = glfwCreateWindow(1920 / 2, 1080 / 2, "FTP Client", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        return -1;

    /* Initialize ImGui */
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

    ImFontConfig fontConfig;
    fontConfig.FontDataOwnedByAtlas = false;
    ImFont* f = io.Fonts->AddFontFromFileTTF(".\\resources\\fonts\\JetBrainsMonoNL-Regular.ttf", 18, nullptr, io.Fonts->GetGlyphRangesCyrillic());
    io.FontDefault = f;


    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowPadding = ImVec2(10.0f, 10.0f);
    style.FramePadding = ImVec2(8.0f, 6.0f);
    style.ItemSpacing = ImVec2(6.0f, 6.0f);
    style.ChildRounding = 6.0f;
    style.PopupRounding = 6.0f;
    style.FrameRounding = 6.0f;
    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg] = ImVec4{ .5f, .5f, .5f, 1.0f };
        style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4{ 0.565f, 0.538f, 0.711f, .1f };
    }

    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460 core");

    /**
     * FontAwesome setup START (required for icons)
    */

    float baseFontSize = 16.0f;
    float iconFontSize = baseFontSize * 2.0f / 3.0f; // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly

    std::ifstream fontAwesomeFile(".\\resources\\fonts\\fa-solid-900.ttf");

    if (!fontAwesomeFile.good())
    {
        // If it's not good, then we can't find the font and should abort
        std::cerr << "Could not find the FontAwesome font file." << std::endl;
        abort();
    }

    static const ImWchar iconsRanges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
    ImFontConfig iconsConfig;
    iconsConfig.MergeMode = true;
    iconsConfig.PixelSnapH = true;
    iconsConfig.GlyphMinAdvanceX = iconFontSize;
    io.Fonts->AddFontFromFileTTF(".\\resources\\fonts\\fa-solid-900.ttf", iconFontSize, &iconsConfig, iconsRanges);

    /**
     * FontAwesome setup END
    */

    auto app = new UI::Application();
    app->m_Running = true;
    app->PushLayer<ClientLayer>();
    while (!glfwWindowShouldClose(window) && app->m_Running)
    {
        glfwPollEvents();

		for (auto& layer : app->m_LayerStack)
			layer->OnUpdate();


        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        {
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;

            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->Pos);
            ImGui::SetNextWindowSize(viewport->Size);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(1.0f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
            ImGui::Begin("DockSpaceWindow", nullptr, window_flags);
            ImGui::PopStyleVar(2);
            ImGui::PopStyleVar(2);

            // Dockspace
            ImGuiIO& io = ImGui::GetIO();
            ImGuiStyle& style = ImGui::GetStyle();
            float minWinSizeX = style.WindowMinSize.x;
            ImGui::DockSpace(ImGui::GetID("MyDockspace"));
            for (auto& layer : app->m_LayerStack)
                layer->OnUIRender();

            ImGui::End();
        }

        // Notifications style setup
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f); // Disable round borders
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f); // Disable borders

        // Notifications color setup
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f, 0.10f, 0.10f, 1.00f)); // Background color


        // Main rendering function
        ImGui::RenderNotifications();


        //——————————————————————————————— WARNING ———————————————————————————————
        // Argument MUST match the amount of ImGui::PushStyleVar() calls 
        ImGui::PopStyleVar(2);
        // Argument MUST match the amount of ImGui::PushStyleColor() calls 
        ImGui::PopStyleColor(1);

        ImGui::Render();
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.0f, 0.2f, 0.2f, 1.0f);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_ctx = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_ctx);
        }
    }

    glfwTerminate();
    return 0;
}

