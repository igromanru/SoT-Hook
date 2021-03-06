#define WIN32_LEAN_AND_MEAN  
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <comdef.h>
#include <d3d11.h>
#include <imgui\imgui.h>
#include <imgui\imgui_impl_win32.h>
#include <imgui\imgui_impl_dx11.h>
#include <stdio.h>
#include <imgui\imgui_internal.h>

HWND window = nullptr;
ID3D11Device* device = nullptr;
ID3D11DeviceContext* context = nullptr;
IDXGISwapChain* swapchain = nullptr;
ID3D11RenderTargetView* view = nullptr;

inline bool CreateView() {
	ID3D11Texture2D* buffer;
	if (FAILED(swapchain->GetBuffer(0, __uuidof(buffer), reinterpret_cast<PVOID*>(&buffer)))) return false;
	if (FAILED(device->CreateRenderTargetView(buffer, nullptr, &view))) return false;
	buffer->Release();
	return true;
}

void ShowErrorMsg(const char* lpszFunction, HRESULT hr)
{	
	char* buf = new char[0x200];
	_com_error err(hr);
	sprintf(buf, "%s failed with error 0x%lX: %s", lpszFunction, hr, err.ErrorMessage());
	MessageBoxA(nullptr, buf, "Error", 0);
	delete[] buf;
}

inline bool InitDX()
{
	DXGI_SWAP_CHAIN_DESC sd{};
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = window;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

	HRESULT hr = 0;
	if (FAILED(hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &sd, &swapchain, &device, nullptr, &context))) {
		ShowErrorMsg("D3D11CreateDeviceAndSwapChain", hr);
		return false;
	}

	if (!CreateView()) 
	{
		if (swapchain) 
		{
			(swapchain)->Release();
			swapchain = nullptr;
		}
		if (device) 
		{
			(device)->Release();
			swapchain = nullptr;
		}
		return false;
	}
	return true;

}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
	{
		return true;
	}
		

	switch (msg)
	{
	case WM_SIZE:
		
		if (device != nullptr && wParam != SIZE_MINIMIZED)
		{
			if (view)
			{
				view->Release();
				view = nullptr;
			}

			swapchain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING);
			CreateView();
		}
		
		
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) return 0;
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProcW(hWnd, msg, wParam, lParam);
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	WNDCLASSEXA wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandleA(NULL), NULL, NULL, NULL, NULL, "ImGui Example", NULL };
	RegisterClassExA(&wc);
	window = CreateWindowExA(0L, wc.lpszClassName, "", WS_POPUP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), NULL, NULL, wc.hInstance, NULL);
	if (!window) 
	{
		UnregisterClassA(wc.lpszClassName, wc.hInstance);
		return 1;
	}

	if (!InitDX()) 
	{
		return 1;
	}

	ShowWindow(window, SW_SHOWDEFAULT);
	UpdateWindow(window);

	ImGui::CreateContext();
	
	ImGuiIO& io = ImGui::GetIO();

	ImFontConfig config;
	config.GlyphRanges = io.Fonts->GetGlyphRangesCyrillic();
	config.RasterizerMultiply = 1.125f;
	io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Arial.ttf", 16.0f, &config);
	//io.Fonts->AddFontFromFileTTF(".\\OpenSans-Regular.ttf", 25.0f, &config);

	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(device, context);

	MSG msg{};
	while (msg.message != WM_QUIT) {

		if (PeekMessageA(&msg, NULL, 0U, 0U, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessageA(&msg);
			continue;
		}

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		static struct Config {
			enum class EBox : int {
				ENone,
				E2DBoxes,
				E3DBoxes,
				EDebugBoxes
			};
			enum class EBar : int {
				ENone,
				ELeft,
				ERight,
				EBottom,
				ETop,
				ETriangle
			};
			enum class EAim {
				ENone,
				EClosest,
				EFOV,
			};
			struct {
				bool bEnable = false;
				struct {
					bool bEnable = false;
					bool bSkeleton = false;
					bool bDrawTeam = false;
					bool bHealth = false;
					bool bName = false;
					EBox boxType = EBox::ENone;
					EBar barType = EBar::ENone;
					ImVec4 enemyColorVis = { 1.f, 0.f, 0.f, 1.f };
					ImVec4 enemyColorInv = { 1.f, 1.f, 0.f, 1.f };
					ImVec4 teamColorVis = { 0.f, 1.f, 0.0f, 1.f };
					ImVec4 teamColorInv = { 0.f, 1.f, 1.f, 1.f };
				} players;
				struct {
					bool bEnable = false;
					bool bSkeleton = false;
					bool bName = false;
					EBox boxType = EBox::ENone;
					EBar barType = EBar::ENone;
					ImVec4 colorVis = { 0.f, 1.f, 0.5f, 1.f };
					ImVec4 colorInv = { 1.f, 0.f, 1.f, 1.f };

				} skeletons;
				struct {
					bool bEnable = false;
					bool bSkeleton = false;
					bool bHealth = false;
					bool bName = false;
					bool bDamage = false;
					ImVec4 damageColor = { 1.f, 1.f, 1.f, 1.f };
				} ships;
				struct {
					bool bEnable = false;
					bool bName = false;
					float fMaxDist = 3500.f;
				} islands;
				struct {
					bool bEnable = false;
					bool bName = false;
				} items;
				struct {
					bool bCrosshair = false;
					bool bOxygen = false;
					bool bCompass = false;
					bool bDebug = false;
					float fCrosshair = 7.f;
					float fDebug = 10.f;
					ImVec4 crosshairColor = { 1.f, 1.f, 1.f, 1.f };
				} client;
			} visuals;
			struct {
				bool bEnable = false;
				struct {
					bool bEnable = false;
					bool bVisibleOnly = false;
					bool bTeam = false;
					float fYaw = 20.f;
					float fPitch = 20.f;
					float fSmoothness = 5.f;
				} players;
				struct {
					bool bEnable = false;
					bool bVisibleOnly = false;
					float fYaw = 20.f;
					float fPitch = 20.f;
					float fSmoothness = 5.f;
				} skeletons;
			} aim;

			struct {
				bool bEnable = false;
				struct {

				} client;
			} misc;

		} cfg;


		static bool bIsOpen = true;
		if (ImGui::IsKeyPressed(VK_INSERT) & 0x1) {
			if (bIsOpen) {
				bIsOpen = false;
			}
			else {
				bIsOpen = true;
			}
		}

		if (bIsOpen) {
			ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.7f), ImGuiCond_Once);
			ImGui::Begin("Menu", 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
			if (ImGui::BeginTabBar("Bars")) {
				if (ImGui::BeginTabItem("Visuals")) {

					ImGui::Text("Global Visuals");
					if (ImGui::BeginChild("Global", ImVec2(0.f, 38.f), true, 0))
					{
						ImGui::Checkbox("Enable", &cfg.visuals.bEnable);
					}
					ImGui::EndChild();


					ImGui::Columns(2, "CLM1", false);
					const char* boxes[] = { "None", "2DBox", "3DBox", "DebugBox" };
					const char* bars[] = { "None", "2DRectLeft", "2DRectRight", "2DRectBottom", "2DRectTop", "2DTriangTop" };
					ImGui::Text("Players");
					if (ImGui::BeginChild("PlayersSettings", ImVec2(0.f, 365.f), true, 0))
					{
						ImGui::Checkbox("Enable", &cfg.visuals.players.bEnable);
						ImGui::Checkbox("Draw teammates", &cfg.visuals.players.bDrawTeam);
						ImGui::Checkbox("Draw name", &cfg.visuals.players.bName);
						ImGui::Checkbox("Draw skeleton", &cfg.visuals.players.bSkeleton);
						ImGui::Combo("Box type", reinterpret_cast<int*>(&cfg.visuals.players.boxType), boxes, IM_ARRAYSIZE(boxes));
						ImGui::Combo("Health bar type", reinterpret_cast<int*>(&cfg.visuals.players.barType), bars, IM_ARRAYSIZE(bars));
						ImGui::ColorEdit4("Visible Enemy color", &cfg.visuals.players.enemyColorVis.x, 0);
						ImGui::ColorEdit4("Invisible Enemy color", &cfg.visuals.players.enemyColorInv.x, 0);
						ImGui::ColorEdit4("Visible Team color", &cfg.visuals.players.teamColorVis.x, 0);
						ImGui::ColorEdit4("Invisible Team color", &cfg.visuals.players.teamColorInv.x, 0);
					}
					ImGui::EndChild();

					ImGui::NextColumn();

					ImGui::Text("Skeletons");
					if (ImGui::BeginChild("SkeletonsSettings", ImVec2(0.f, 365.f), true, 0))
					{
						ImGui::Checkbox("Enable", &cfg.visuals.skeletons.bEnable);
						ImGui::Combo("Box type", reinterpret_cast<int*>(&cfg.visuals.skeletons.boxType), boxes, IM_ARRAYSIZE(boxes));
						ImGui::ColorEdit4("Visible Color", &cfg.visuals.skeletons.colorVis.x, 0);
						ImGui::ColorEdit4("Invisible Color", &cfg.visuals.skeletons.colorInv.x, 0);
						ImGui::Checkbox("Draw skeleton", &cfg.visuals.skeletons.bSkeleton);

					}
					ImGui::EndChild();
					ImGui::Columns();





					ImGui::Columns(2, "CLM2", false);

					ImGui::Text("Ships");
					if (ImGui::BeginChild("ShipsSettings", ImVec2(0.f, 220.f), true, 0)) {

						ImGui::Checkbox("Enable", &cfg.visuals.ships.bEnable);
						ImGui::Checkbox("Draw name", &cfg.visuals.ships.bName);
						ImGui::Checkbox("Show holes", &cfg.visuals.ships.bDamage);

					}
					ImGui::EndChild();

					ImGui::NextColumn();

					ImGui::Text("Islands");
					if (ImGui::BeginChild("IslandsSettings", ImVec2(0.f, 220.f), true, 0)) {
						ImGui::Checkbox("Enable", &cfg.visuals.islands.bEnable);
						ImGui::Checkbox("Draw names", &cfg.visuals.islands.bName);
						ImGui::SliderFloat("Max distance", &cfg.visuals.islands.fMaxDist, 100.f, 10000.f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
					}
					ImGui::EndChild();
					ImGui::Columns();



					ImGui::Columns(2, "CLM3", false);
					ImGui::Text("Items");
					if (ImGui::BeginChild("ItemsSettings", ImVec2(0.f, 220.f), true, 0))
					{
						ImGui::Checkbox("Enable", &cfg.visuals.items.bEnable);
						ImGui::Checkbox("Draw name", &cfg.visuals.items.bName);

					}
					ImGui::EndChild();
					ImGui::NextColumn();
					ImGui::Text("Client");
					if (ImGui::BeginChild("ClientSettings", ImVec2(0.f, 220.f), true, 0))
					{

						ImGui::Checkbox("Crosshair", &cfg.visuals.client.bCrosshair);
						if (cfg.visuals.client.bCrosshair)
						{
							ImGui::SameLine();
							ImGui::SetNextItemWidth(75.f);
							ImGui::SliderFloat("Radius##1", &cfg.visuals.client.fCrosshair, 1.f, 100.f);
						}

						ImGui::ColorEdit4("Crosshair color", &cfg.visuals.client.crosshairColor.x, ImGuiColorEditFlags_DisplayRGB);

						ImGui::Checkbox("Oxygen level", &cfg.visuals.client.bOxygen);
						ImGui::Checkbox("Compass", &cfg.visuals.client.bCompass);

						ImGui::Checkbox("Debug", &cfg.visuals.client.bDebug);
						if (cfg.visuals.client.bDebug)
						{
							ImGui::SameLine();
							ImGui::SetNextItemWidth(150.f);
							ImGui::SliderFloat("Radius##2", &cfg.visuals.client.fDebug, 1.f, 1000.f);
						}

					}
					ImGui::EndChild();
					ImGui::Columns();


					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Aim")) {

					ImGui::Text("Global Aim");
					if (ImGui::BeginChild("Global", ImVec2(0.f, 38.f), true, 0))
					{
						ImGui::Checkbox("Enable", &cfg.aim.bEnable);
					}
					ImGui::EndChild();


					ImGui::Columns(2, "CLM1", false);
					ImGui::Text("Players");
					if (ImGui::BeginChild("PlayersSettings", ImVec2(0.f, 365.f), true, 0))
					{
						// todo: add bones selection
						ImGui::Checkbox("Enable", &cfg.aim.players.bEnable);
						ImGui::Checkbox("Visible only", &cfg.aim.players.bVisibleOnly);
						ImGui::Checkbox("Aim at teammates", &cfg.aim.players.bTeam);
						ImGui::SliderFloat("Yaw", &cfg.aim.players.fYaw, 1.f, 180.f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
						ImGui::SliderFloat("Pitch", &cfg.aim.players.fPitch, 1.f, 180.f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
						ImGui::SliderFloat("Smoothness", &cfg.aim.players.fSmoothness, 1.f, 100.f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
					}
					ImGui::EndChild();

					ImGui::NextColumn();

					ImGui::Text("Skeletons");
					if (ImGui::BeginChild("SkeletonsSettings", ImVec2(0.f, 365.f), true, 0))
					{
						ImGui::Checkbox("Enable", &cfg.aim.skeletons.bEnable);
						ImGui::Checkbox("Visible only", &cfg.aim.skeletons.bVisibleOnly);
						ImGui::SliderFloat("Yaw", &cfg.aim.skeletons.fYaw, 1.f, 180.f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
						ImGui::SliderFloat("Pitch", &cfg.aim.skeletons.fPitch, 1.f, 180.f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
						ImGui::SliderFloat("Smoothness", &cfg.aim.skeletons.fSmoothness, 1.f, 100.f, "%.0f", ImGuiSliderFlags_AlwaysClamp);


					}
					ImGui::EndChild();
					ImGui::Columns();



					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Misc")) {

					ImGui::Text("Global Misc");
					if (ImGui::BeginChild("Global", ImVec2(0.f, 38.f), true, 0))
					{
						ImGui::Checkbox("Enable", &cfg.misc.bEnable);
					}
					ImGui::EndChild();

					ImGui::Columns(2, "CLM1", false);
					ImGui::Text("Client");
					if (ImGui::BeginChild("ClientSettings", ImVec2(0.f, 365.f), true, 0))
					{
						if (ImGui::Button("Tests")) {
							
						}
					}
					ImGui::EndChild();


					ImGui::Columns();


					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			};
			ImGui::End();
		}


		ImGui::Render();
		context->OMSetRenderTargets(1, &view, NULL);
		const float clearColor[] = { 0.f, 0.f, 0.f, 1.f };
		context->ClearRenderTargetView(view, clearColor);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		swapchain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
	}


	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();


	if (view) {
		view->Release();
		view = nullptr;
	}
	if (swapchain)
	{
		swapchain->Release();
		swapchain = nullptr;
	}
	if (context)
	{
		context->Release();
		context = nullptr;
	}
	if (device)
	{
		device->Release();
		device = nullptr;
	}

	DestroyWindow(window);
	UnregisterClassA(wc.lpszClassName, wc.hInstance);


	return 0;
}