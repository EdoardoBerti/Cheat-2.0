#pragma once
// ============================================================================
//  DirectX 11 Overlay — transparent, topmost, click-through window
//  Uses ImGui for rendering ESP and menu
// ============================================================================

#include <Windows.h>
#include <d3d11.h>
#include <dwmapi.h>
#include <string>
#include <cstdio>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dwmapi.lib")

// ImGui includes — make sure vendor/imgui is in your include path
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "driver_comm.h"

extern DriverComm* g_driver;

// Forward declare ImGui's WndProc handler
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

class Overlay {
public:
    HWND                    overlay_hwnd    = nullptr;
    HWND                    game_hwnd       = nullptr;

    ID3D11Device*           device          = nullptr;
    ID3D11DeviceContext*    context         = nullptr;
    IDXGISwapChain*         swap_chain      = nullptr;
    ID3D11RenderTargetView* render_target   = nullptr;

    bool                    menu_visible    = true;
    bool                    running         = true;
    int                     screen_w        = 0;
    int                     screen_h        = 0;

    // ---- Initialize ----

    bool Init() {
        // Wait for game window to become visible (up to 45 seconds)
        printf("[*] In attesa che la finestra del gioco diventi visibile...\n");
        for (int i = 0; i < 45; ++i) {
            game_hwnd = FindGameWindow();
            if (game_hwnd && IsWindowVisible(game_hwnd)) break;
            Sleep(1000);
        }

        if (!game_hwnd) {
            printf("[!] Finestra del gioco non trovata.\n");
            return false;
        }

        // Get game window dimensions
        RECT rect;
        GetClientRect(game_hwnd, &rect);
        screen_w = rect.right - rect.left;
        screen_h = rect.bottom - rect.top;

        POINT pos = { 0, 0 };
        ClientToScreen(game_hwnd, &pos);

        printf("[+] Game window: %dx%d at (%d,%d)\n", screen_w, screen_h, pos.x, pos.y);

        // Create overlay window
        WNDCLASSEXW wc = {};
        wc.cbSize        = sizeof(wc);
        wc.style         = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc   = WndProc;
        wc.hInstance      = GetModuleHandle(nullptr);
        wc.lpszClassName  = L"NvOverlay";
        RegisterClassExW(&wc);

        overlay_hwnd = CreateWindowExW(
            WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
            wc.lpszClassName,
            L"NVIDIA GeForce Overlay",
            WS_POPUP,
            pos.x, pos.y, screen_w, screen_h,
            nullptr, nullptr, wc.hInstance, nullptr
        );

        if (!overlay_hwnd) {
            printf("[!] Failed to create overlay window\n");
            return false;
        }

        // Make window transparent
        SetLayeredWindowAttributes(overlay_hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

        // Extend frame into client area for transparency
        MARGINS margins = { -1 };
        DwmExtendFrameIntoClientArea(overlay_hwnd, &margins);

        ShowWindow(overlay_hwnd, SW_SHOWNOACTIVATE);
        UpdateWindow(overlay_hwnd);

        // Initialize D3D11
        if (!InitD3D()) {
            printf("[!] Failed to initialize D3D11\n");
            return false;
        }

        // Initialize ImGui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = nullptr; // Don't save ini
        io.LogFilename = nullptr;

        // Style
        ImGui::StyleColorsDark();
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding    = 6.0f;
        style.FrameRounding     = 4.0f;
        style.GrabRounding      = 4.0f;
        style.ScrollbarRounding = 4.0f;
        style.WindowBorderSize  = 1.0f;
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.08f, 0.94f);
        style.Colors[ImGuiCol_TitleBg]  = ImVec4(0.10f, 0.10f, 0.14f, 1.0f);
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.16f, 0.22f, 1.0f);
        style.Colors[ImGuiCol_FrameBg]  = ImVec4(0.14f, 0.14f, 0.18f, 1.0f);
        style.Colors[ImGuiCol_CheckMark] = ImVec4(0.45f, 0.72f, 1.0f, 1.0f);
        style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.45f, 0.72f, 1.0f, 1.0f);
        style.Colors[ImGuiCol_Button]   = ImVec4(0.20f, 0.20f, 0.28f, 1.0f);
        style.Colors[ImGuiCol_Header]   = ImVec4(0.20f, 0.20f, 0.28f, 1.0f);

        ImGui_ImplWin32_Init(overlay_hwnd);
        ImGui_ImplDX11_Init(device, context);

        printf("[+] Overlay initialized\n");
        return true;
    }

    // ---- Frame Begin/End ----

    void BeginFrame() {
        // Keep overlay aligned with game window
        SyncWithGameWindow();

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }

    void EndFrame() {
        ImGui::Render();

        const float clear_color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        context->OMSetRenderTargets(1, &render_target, nullptr);
        context->ClearRenderTargetView(render_target, clear_color);

        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        swap_chain->Present(1, 0); // VSync on
    }

    // ---- Input Handling ----

    void HandleInput() {
        // Toggle menu with INSERT key
        if (GetAsyncKeyState(VK_INSERT) & 1) {
            menu_visible = !menu_visible;
            UpdateClickThrough();
        }

        // ESC to close (when menu is visible)
        if (menu_visible && (GetAsyncKeyState(VK_ESCAPE) & 1)) {
            menu_visible = false;
            UpdateClickThrough();
        }

        // END to exit cheat
        if (GetAsyncKeyState(VK_END) & 1) {
            running = false;
        }
    }

    // ---- Process Window Messages ----

    bool ProcessMessages() {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                running = false;
                return false;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        return true;
    }

    // ---- Cleanup ----

    void Shutdown() {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        if (render_target) render_target->Release();
        if (swap_chain)    swap_chain->Release();
        if (context)       context->Release();
        if (device)        device->Release();

        if (overlay_hwnd)  DestroyWindow(overlay_hwnd);
    }

    // ---- Get ImGui DrawList for custom rendering ----

    ImDrawList* GetDrawList() {
        return ImGui::GetBackgroundDrawList();
    }

private:
    HWND FindGameWindow() {
        // 1. If PID is known, find window belonging to PID
        if (g_driver && g_driver->pid != 0) {
            HWND temp = nullptr;
            while ((temp = FindWindowExW(nullptr, temp, nullptr, nullptr)) != nullptr) {
                DWORD win_pid = 0;
                GetWindowThreadProcessId(temp, &win_pid);
                if (win_pid == g_driver->pid && IsWindowVisible(temp)) {
                    RECT rc;
                    GetClientRect(temp, &rc);
                    if ((rc.right - rc.left) > 100 && (rc.bottom - rc.top) > 100) {
                        return temp;
                    }
                }
            }
        }

        // 2. Try exact window title
        HWND hwnd = FindWindowW(nullptr, L"ROB RUN");
        if (hwnd) return hwnd;

        // 3. Try partial title
        hwnd = FindWindowW(nullptr, L"Rob Run");
        if (hwnd) return hwnd;

        // 4. Enumerate windows
        HWND temp = nullptr;
        while ((temp = FindWindowExW(nullptr, temp, nullptr, nullptr)) != nullptr) {
            wchar_t title[256];
            GetWindowTextW(temp, title, 256);
            if (wcsstr(title, L"ROB") || wcsstr(title, L"Rob")) {
                return temp;
            }
        }

        return nullptr;
    }

    bool InitD3D() {
        DXGI_SWAP_CHAIN_DESC scd = {};
        scd.BufferCount       = 2;
        scd.BufferDesc.Width  = screen_w;
        scd.BufferDesc.Height = screen_h;
        scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        scd.BufferDesc.RefreshRate.Numerator   = 0;
        scd.BufferDesc.RefreshRate.Denominator = 1;
        scd.BufferUsage       = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scd.OutputWindow      = overlay_hwnd;
        scd.SampleDesc.Count  = 1;
        scd.Windowed          = TRUE;
        scd.SwapEffect        = DXGI_SWAP_EFFECT_DISCARD;

        D3D_FEATURE_LEVEL feature_level;
        HRESULT hr = D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
            0, nullptr, 0, D3D11_SDK_VERSION,
            &scd, &swap_chain, &device, &feature_level, &context
        );

        if (FAILED(hr)) return false;

        // Create render target view
        ID3D11Texture2D* back_buffer = nullptr;
        swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&back_buffer);
        device->CreateRenderTargetView(back_buffer, nullptr, &render_target);
        back_buffer->Release();

        return true;
    }

    void SyncWithGameWindow() {
        if (!IsWindow(game_hwnd)) {
            running = false;
            return;
        }

        RECT rect;
        GetClientRect(game_hwnd, &rect);
        POINT pos = { 0, 0 };
        ClientToScreen(game_hwnd, &pos);

        int w = rect.right - rect.left;
        int h = rect.bottom - rect.top;

        if (w != screen_w || h != screen_h) {
            screen_w = w;
            screen_h = h;
            // Resize swap chain would go here
        }

        SetWindowPos(overlay_hwnd, HWND_TOPMOST,
            pos.x, pos.y, w, h, SWP_NOACTIVATE);
    }

    void UpdateClickThrough() {
        LONG_PTR style = GetWindowLongPtr(overlay_hwnd, GWL_EXSTYLE);

        if (menu_visible) {
            // Remove click-through so we can interact with menu
            style &= ~WS_EX_TRANSPARENT;
            SetForegroundWindow(overlay_hwnd);
        } else {
            // Make click-through again
            style |= WS_EX_TRANSPARENT;
        }

        SetWindowLongPtr(overlay_hwnd, GWL_EXSTYLE, style);
    }

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
            return true;

        switch (msg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
};

// ============================================================================
//  Draw Namespace — wraps ImGui drawing for use by features
// ============================================================================

namespace Draw {

    static ImDrawList* dl = nullptr;

    inline void SetDrawList(ImDrawList* draw_list) {
        dl = draw_list;
    }

    inline void Box2D(float x, float y, float w, float h, uint32_t color, float thickness) {
        if (!dl) return;
        dl->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), color, 0.0f, 0, thickness);
    }

    inline void FilledBox(float x, float y, float w, float h, uint32_t color) {
        if (!dl) return;
        dl->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), color);
    }

    inline void Line(float x1, float y1, float x2, float y2, uint32_t color, float thickness) {
        if (!dl) return;
        dl->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), color, thickness);
    }

    inline void Circle(float x, float y, float radius, uint32_t color, int segments, float thickness) {
        if (!dl) return;
        dl->AddCircle(ImVec2(x, y), radius, color, segments, thickness);
    }

    inline void Text(float x, float y, const char* text, uint32_t color) {
        if (!dl) return;
        // Shadow
        dl->AddText(ImVec2(x + 1, y + 1), 0xFF000000, text);
        dl->AddText(ImVec2(x, y), color, text);
    }

    inline void TextW(float x, float y, const wchar_t* text, uint32_t color) {
        if (!dl) return;
        // Convert to UTF-8 for ImGui
        char utf8[512];
        WideCharToMultiByte(CP_UTF8, 0, text, -1, utf8, sizeof(utf8), nullptr, nullptr);
        Text(x, y, utf8, color);
    }

    inline void HealthBar(float x, float y, float h, float health_pct, float width) {
        if (!dl) return;
        // Background
        FilledBox(x - 1, y - 1, width + 2, h + 2, 0xFF000000);

        // Health color gradient: green -> yellow -> red
        uint32_t color;
        if (health_pct > 0.6f)      color = COL_HEALTH_HI;
        else if (health_pct > 0.3f) color = COL_HEALTH_MID;
        else                        color = COL_HEALTH_LO;

        float bar_h = h * health_pct;
        FilledBox(x, y + (h - bar_h), width, bar_h, color);
    }

    inline void BoneLine(const Vec2& a, const Vec2& b, uint32_t color, float thickness) {
        Line(a.x, a.y, b.x, b.y, color, thickness);
    }

    // Draw FOV circle for aimbot
    inline void FOVCircle(float screen_w, float screen_h, float fov_px, uint32_t color) {
        Circle(screen_w * 0.5f, screen_h * 0.5f, fov_px, color, 64, 1.0f);
    }

    // Draw crosshair
    inline void Crosshair(float screen_w, float screen_h, float size, uint32_t color) {
        float cx = screen_w * 0.5f;
        float cy = screen_h * 0.5f;
        Line(cx - size, cy, cx + size, cy, color, 1.5f);
        Line(cx, cy - size, cx, cy + size, color, 1.5f);
    }
}
