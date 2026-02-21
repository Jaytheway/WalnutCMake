#pragma once

#include "Walnut/Layer.h"
#include "Walnut/Image.h"

#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <memory>
#include <functional>
#include <filesystem>
#include <span>

#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"

#include "vulkan/vulkan.h"

void check_vk_result(VkResult err);

struct GLFWwindow;

namespace Walnut {

	struct ApplicationSpecification
	{
		std::string Name = "Walnut App";
		uint32_t Width = 1600;
		uint32_t Height = 900;

		// Icon used for taskbar and native titlebar
		std::filesystem::path IconPath;

		// If IconPath is not provided, or the file
		// does not exist, icon will be attempted to
		// get loaded from IconData.
		// Format must be suitable for glfwSetWindowIcon:
		//	little-endian; RGBA; aspect ratio square;
		//	Width & Height will be implied from the 
		//	size of the data.
		std::span<const unsigned char> IconData;

		bool WindowResizeable = true;

		// Uses custom Walnut titlebar instead
		// of Windows default
		bool CustomTitlebar = false;

		// Uses a dockspace which takes up the
		// entire window
		bool UseDockspace = true;

		// Window will be created in the center
		// of primary monitor
		bool CenterWindow = false;

		// Settings file to store imgui settings.
		// Can be relative to working directory.
		std::string SettingsFile = "imgui.ini";

		// Settings data that will be loaded if
		// settings file wasn't found.
		// If nullptr, new file will be created
		// in the working directory.
		// This data can be created by calling
		// ImGui::SaveIniSettingsToMemory(),
		// or emedding contents of an existing
		// settings file into a string literal.
		const char* DefaultSettingsData = nullptr;
	};

	class Application
	{
	public:
		Application(const ApplicationSpecification& applicationSpecification = ApplicationSpecification());
		~Application();

		static Application& Get();

		void Run();
		void SetMenubarCallback(const std::function<void()>& menubarCallback);
		void SetDrawLogoCallback(const std::function<void(ImVec2 titlebarMin, ImVec2 titlebarMax)>& drawLogoCallback);
		void SetDrawWindowButtonsCallback(const std::function<void(ImVec2 titlebarMin, ImVec2 titlebarMax)>& drawWindowButtonsCallback);

		template<typename T>
		void PushLayer()
		{
			static_assert(std::is_base_of<Layer, T>::value, "Pushed type is not subclass of Layer!");
			m_LayerStack.emplace_back(std::make_shared<T>())->OnAttach();
		}

		void PushLayer(const std::shared_ptr<Layer>& layer) { m_LayerStack.emplace_back(layer); layer->OnAttach(); }

		const std::vector<std::shared_ptr<Layer>>& GetLayerStack() const { return m_LayerStack; }

		void Close();
		void ToggleMaximize();
		void Minimize();

		bool IsMaximized() const;
		std::shared_ptr<Image> GetApplicationIcon() const { return m_AppHeaderIcon; }

		float GetTime();
		GLFWwindow* GetWindowHandle() const { return m_WindowHandle; }
		bool IsTitleBarHovered() const { return m_TitleBarHovered; }

		static VkInstance GetInstance();
		static VkPhysicalDevice GetPhysicalDevice();
		static VkDevice GetDevice();

		static VkCommandBuffer GetCommandBuffer(bool begin);
		static void FlushCommandBuffer(VkCommandBuffer commandBuffer);

		static void SubmitResourceFree(std::function<void()>&& func);

		static ImFont* GetFont(const std::string& name);

		template<typename Func>
		void QueueEvent(Func&& func)
		{
			m_EventQueue.push(func);
		}

		static ImGui_ImplVulkanH_Window* GetMainWindowData();
		static VkCommandBuffer GetActiveCommandBuffer();
	private:
		void Init();
		void Shutdown();

		// For custom titlebars
		void UI_DrawTitlebar(float& outTitlebarHeight);
		void UI_DrawMenubar();
	private:
		ApplicationSpecification m_Specification;
		GLFWwindow* m_WindowHandle = nullptr;
		bool m_Running = false;

		float m_TimeStep = 0.0f;
		float m_FrameTime = 0.0f;
		float m_LastFrameTime = 0.0f;

		bool m_TitleBarHovered = false;

		std::vector<std::shared_ptr<Layer>> m_LayerStack;
		std::function<void()> m_MenubarCallback;
		std::function<void(ImVec2, ImVec2)> m_DrawLogoCallback;
		std::function<void(ImVec2, ImVec2)> m_DrawWindowButtonsCallback;

		std::mutex m_EventQueueMutex;
		std::queue<std::function<void()>> m_EventQueue;

		// Resources
		// TODO(Yan): move out of application class since this can't be tied
		//            to application lifetime
		std::shared_ptr<Walnut::Image> m_AppHeaderIcon;
		std::shared_ptr<Walnut::Image> m_IconClose;
		std::shared_ptr<Walnut::Image> m_IconMinimize;
		std::shared_ptr<Walnut::Image> m_IconMaximize;
		std::shared_ptr<Walnut::Image> m_IconRestore;
	};

	// Implemented by CLIENT
	Application* CreateApplication(int argc, char** argv);
}