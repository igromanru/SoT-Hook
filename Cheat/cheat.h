#pragma once
#include <windows.h>
#include <Psapi.h>
#include <d3d11.h>
#include <mutex>
#include <imgui/imgui.h>
#include "SDK.h"

class Cheat {
private:
	class Renderer {
	private:
		struct Drawing {
			static void RenderText(const char* text, const FVector2D& pos, const ImVec4& color, const bool outlined, const bool centered);
			static void Render2DBox(const FVector2D& top, const FVector2D& bottom, const float height, const float width, const ImVec4& color);
			static bool Render3DBox(AController* constcontroller, const FVector& origin, const FVector& extent, const FRotator& rotation, const ImVec4& color);
			static bool RenderSkeleton(AController* const controller, USkeletalMeshComponent* const mesh, const FMatrix& comp2world, const std::pair<const BYTE*, const BYTE>* skeleton, int size, const ImVec4& color);
			static bool RenderColorBar(FVector& origin, float size,  ImU32*& colors, int n);
		};
	private:
		static inline void** vtable;
		static inline HRESULT(*PresentOriginal)(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags) = nullptr;
		static inline HRESULT(*ResizeOriginal)(IDXGISwapChain* swapChain, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags) = nullptr;
		static inline decltype(SetCursorPos)* SetCursorPosOriginal = nullptr;
		static inline ID3D11Device* device = nullptr;
		static inline ID3D11DeviceContext* context = nullptr;
		static inline ID3D11RenderTargetView* renderTargetView = nullptr;
		static inline bool bGameInput = true;
		static inline WNDPROC WndProcOriginal = nullptr;
		static inline HWND gameWindow;
		static inline ImFont* Arial;
		//static inline ImDrawList* drawList = nullptr;
	private:
		static LRESULT WINAPI WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		static BOOL WINAPI SetCursorPosHook(int X, int Y);
		static BOOL WINAPI ShowCursorHook(BOOL bShow);
		//static HCURSOR WINAPI SetCursorHook(HCURSOR hCursor);
		static void HookInput();
		static void RemoveInput();
		static HRESULT PresentHook(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags);
		static HRESULT ResizeHook(IDXGISwapChain* swapChain, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags);
	public:
		static inline bool Init();
		static inline bool Remove();
	};
	class Tools {
	private:
		static inline bool CompareByteArray(BYTE* data, BYTE* sig, SIZE_T size);
		static inline BYTE* FindSignature(BYTE* start, BYTE* end, BYTE* sig, SIZE_T size);
		static void* FindPointer(BYTE* sig, SIZE_T size, int addition);
	public:
		static inline BYTE* FindFn(HMODULE mod, BYTE* sig, SIZE_T sigSize);
		static inline bool PatchMem(void* address, void* bytes, SIZE_T size);
		static inline BYTE* PacthFn(HMODULE mod, BYTE* sig, SIZE_T sigSize, BYTE* bytes, SIZE_T bytesSize);
		//static inline bool HookVT(void** vtable, UINT64 index, void* FuncH, void** FuncO);
		//static inline void ShowErrorMsg(const CHAR* lpszFunction);
		static inline bool FindNameArray();
		static inline bool FindObjectsArray();
		static inline bool FindWorld();
		static inline bool InitSDK();
		
	};

	class Logger {
	private:
		static inline HANDLE file = nullptr;
		static inline std::mutex mutex;
	public:
		static inline bool Init();
		static inline bool Remove();
		static void Log(const char* format, ...);
	};

public:
	static bool Init(HINSTANCE _hinstDLL);
	static void ClearingThread();
	static void Tests();
	static bool Remove();
private:
	inline static MODULEINFO gBaseMod;
	inline static HINSTANCE hinstDLL;
};


