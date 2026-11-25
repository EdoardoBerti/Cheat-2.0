// ============================================================================
//  RONRUN — Rob Run External Cheat
//  Main entry point — initializes driver, overlay, game, and runs main loop
// ============================================================================

#include <Windows.h>
#include <cstdio>

#include "driver_comm.h"
#include "sdk.h"
#include "features.h"
#include "overlay.h"
#include "menu.h"

// ---- Globals ----
DriverComm      g_driver_instance;
DriverComm*     g_driver = &g_driver_instance;
Overlay         g_overlay;
CGameWorld      g_world;
CheatSettings   g_settings;

// ---- Camera Resolution ----
// Find the main Camera pointer via pattern scan on UnityPlayer.dll

uint64_t g_camera_addr = 0;

bool ResolveCamera() {
    // Try pattern scan for Camera::get_main
    uint64_t pattern_addr = g_driver->PatternScanRIP(
        g_driver->unity_base, g_driver->unity_size,
        Offsets::Patterns::CameraMain,
        3, 7  // RIP relative: opcode(3 bytes) + rip_offset(4 bytes) = 7
    );

    if (pattern_addr) {
        // This gives us a pointer to the Camera* static field
        g_camera_addr = g_driver->Read<uint64_t>(pattern_addr);
        if (g_camera_addr) {
            g_camera_addr = g_driver->Read<uint64_t>(g_camera_addr);
            printf("[+] Camera found via pattern: 0x%llX\n", g_camera_addr);
            return true;
        }
    }

    printf("[!] Camera not found — ESP will not work until camera offset is set\n");
    printf("[!] Use Il2CppDumper to find Camera::main offset in UnityPlayer.dll\n");
    return false;
}

// ---- Main Loop ----

void MainLoop() {
    printf("[*] Main loop started. Press INSERT to toggle menu, END to exit.\n\n");

    while (g_overlay.running) {
        // Process Windows messages
        if (!g_overlay.ProcessMessages())
            break;

        // Handle hotkeys
        g_overlay.HandleInput();

        // Hotkeys for exploit
        if (GetAsyncKeyState(VK_F8) & 1) {
            FenceExploit::Execute(g_world);
        }
        if (GetAsyncKeyState(VK_F9) & 1) {
            FenceExploit::Restore(g_world);
        }
        if (GetAsyncKeyState(VK_F10) & 1) {
            FenceExploit::SetMoney(g_world, g_settings.exploit_sell_value);
        }

        // Try to refresh game world if not initialized
        if (!g_world.manager_addr || !g_world.IsInGame()) {
            g_world.Init(g_driver->ga_base);
        }

        // Update features
        if (g_world.manager_addr) {
            Aimbot::Update(g_world, CCamera(g_camera_addr));
            SilentAim::Update(g_world);
            SpeedHack::Update(g_world);
            Noclip::Update(g_world);
            FastLoot::Update(g_world);
            FastSearch::Update(g_world);
        }

        // ---- Render Frame ----
        g_overlay.BeginFrame();

        // Set the draw list for feature rendering
        Draw::SetDrawList(g_overlay.GetDrawList());

        // Draw ESP
        if (g_world.manager_addr && g_camera_addr) {
            ESP::Render(g_world, CCamera(g_camera_addr));
        }

        // Draw aimbot FOV circle
        if (g_settings.aim_enabled) {
            // Convert FOV degrees to screen pixels (approximate)
            float fov_px = tanf(DEG2RAD(g_settings.aim_fov)) *
                           (g_overlay.screen_w * 0.5f);
            Draw::FOVCircle(
                (float)g_overlay.screen_w,
                (float)g_overlay.screen_h,
                fov_px, 0x40FFFFFF
            );
        }

        // Draw crosshair
        Draw::Crosshair(
            (float)g_overlay.screen_w,
            (float)g_overlay.screen_h,
            8.0f, COL_WHITE
        );

        // Draw menu
        if (g_overlay.menu_visible) {
            Menu::Render(g_settings);
        }

        // Watermark
        {
            char info[160];
            snprintf(info, sizeof(info), "RONRUN v1.0 | [%s] | PID: %u | GA: 0x%llX",
                g_driver->GetModeName(), g_driver->pid, g_driver->ga_base);
            Draw::Text(10.0f, 10.0f, info, 0x80FFFFFF);
        }

        g_overlay.EndFrame();

        // Small sleep to not destroy CPU
        Sleep(1);
    }
}

// ---- Entry Point ----

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    // Allocate console for debug output
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);

    printf("============================================\n");
    printf("  RONRUN v1.0 — Rob Run External Suite\n");
    printf("  Single Standalone Executable (Kernel + Usermode)\n");
    printf("============================================\n\n");

    // Step 1: Initialize Memory Backend (Dual-Engine: Kernel or Usermode Direct)
    printf("[*] Inizializzazione motore di memoria...\n");
    if (!g_driver->EnsureDriverLoaded()) {
        printf("[!] Impossibile inizializzare il motore di memoria.\n");
        printf("\nPremi un tasto per uscire...\n");
        getchar();
        return 1;
    }
    printf("[+] Backend selezionato: [%s]\n\n", g_driver->GetModeName());

    // Step 2: Wait for game process and modules
    if (!g_driver->WaitForGame()) {
        printf("\nPremi un tasto per uscire...\n");
        g_driver->Disconnect();
        getchar();
        return 1;
    }
    printf("\n");

    // Step 3: Resolve camera
    printf("[*] Resolving camera...\n");
    ResolveCamera();
    printf("\n");

    // Step 4: Initialize game world
    printf("[*] Initializing game world...\n");
    if (g_world.Init(g_driver->ga_base)) {
        printf("[+] Game world initialized!\n");
    } else {
        printf("[!] Game world init failed — will retry in main loop\n");
        printf("[!] Make sure you're in a match\n");
    }
    printf("\n");

    // Step 5: Create overlay
    printf("[*] Creating overlay...\n");
    if (!g_overlay.Init()) {
        printf("[!] FAILED: Could not create overlay.\n");
        printf("[!] Make sure the game is running and visible.\n");
        printf("\nPress any key to exit...\n");
        g_driver->Disconnect();
        getchar();
        return 1;
    }
    printf("\n");

    // Step 6: Run
    MainLoop();

    // Cleanup
    printf("\n[*] Shutting down...\n");
    SpeedHack::Reset();
    g_overlay.Shutdown();
    g_driver->Disconnect();

    printf("[+] Clean exit.\n");
    FreeConsole();
    return 0;
}
