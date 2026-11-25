#pragma once
// ============================================================================
//  ImGui Menu — Cheat settings interface
//  Toggle with INSERT key
// ============================================================================

#include "imgui.h"
#include "features.h"
#include "driver_comm.h"

extern DriverComm* g_driver;

class Menu {
public:
    static void Render(CheatSettings& cfg) {
        ImGui::SetNextWindowSize(ImVec2(560, 520), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);

        ImGui::Begin("ROB RUN /// RONRUN v1.0", nullptr,
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);

        // Active Backend Indicator
        if (g_driver) {
            if (g_driver->mode == DriverComm::MODE_KERNEL) {
                ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "[BACKEND: KERNEL DRIVER]");
            } else {
                ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "[BACKEND: USERMODE DIRECT]");
            }
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "| PID: %u | GA: 0x%llX", g_driver->pid, g_driver->ga_base);
            ImGui::Separator();
        }

        if (ImGui::BeginTabBar("##tabs")) {
            TabESP(cfg);
            TabAimbot(cfg);
            TabSilentAim(cfg);
            TabMovement(cfg);
            TabLoot(cfg);
            TabExploit(cfg);
            TabMisc(cfg);
            ImGui::EndTabBar();
        }

        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f),
            "[INSERT] Menu  [END] Exit  [E] Fast Loot  [F8] Fence Exploit  [F9] Restore");

        ImGui::End();
    }

private:
    static void TabESP(CheatSettings& cfg) {
        if (!ImGui::BeginTabItem("ESP")) return;

        ImGui::Checkbox("Enable Players ESP", &cfg.esp_enabled);
        ImGui::Separator();

        if (cfg.esp_enabled) {
            ImGui::Checkbox("Box",        &cfg.esp_box);
            ImGui::Checkbox("Name",       &cfg.esp_name);
            ImGui::Checkbox("Health Bar", &cfg.esp_health);
            ImGui::Checkbox("Distance",   &cfg.esp_distance);
            ImGui::Checkbox("Skeleton",   &cfg.esp_bones);
            ImGui::Checkbox("Snap Lines", &cfg.esp_snapline);
            ImGui::Checkbox("Team Check", &cfg.esp_team_check);

            ImGui::Separator();
            ImGui::SliderFloat("Max Distance", &cfg.esp_max_distance, 50.0f, 1000.0f, "%.0f m");

            ImGui::Separator();
            ImGui::Text("Colors");
            ImVec4 enemy_col = ImGui::ColorConvertU32ToFloat4(cfg.esp_enemy_color);
            if (ImGui::ColorEdit4("Enemy", (float*)&enemy_col, ImGuiColorEditFlags_NoInputs))
                cfg.esp_enemy_color = ImGui::ColorConvertFloat4ToU32(enemy_col);

            ImVec4 team_col = ImGui::ColorConvertU32ToFloat4(cfg.esp_team_color);
            if (ImGui::ColorEdit4("Team", (float*)&team_col, ImGuiColorEditFlags_NoInputs))
                cfg.esp_team_color = ImGui::ColorConvertFloat4ToU32(team_col);
        }

        ImGui::EndTabItem();
    }

    static void TabAimbot(CheatSettings& cfg) {
        if (!ImGui::BeginTabItem("Aimbot")) return;

        ImGui::Checkbox("Enable Aimbot", &cfg.aim_enabled);
        ImGui::Separator();

        if (cfg.aim_enabled) {
            ImGui::SliderFloat("FOV",       &cfg.aim_fov,    1.0f, 90.0f, "%.1f°");
            ImGui::SliderFloat("Smoothing", &cfg.aim_smooth, 1.0f, 20.0f, "%.1f");

            const char* bones[] = { "Head", "Neck", "Chest", "Spine", "Pelvis" };
            ImGui::Combo("Target Bone", &cfg.aim_bone, bones, IM_ARRAYSIZE(bones));

            ImGui::Checkbox("Team Check", &cfg.aim_team_check);

            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                "Hold RIGHT MOUSE BUTTON to aim");
        }

        ImGui::EndTabItem();
    }

    static void TabSilentAim(CheatSettings& cfg) {
        if (!ImGui::BeginTabItem("Silent Aim")) return;

        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f),
            "SILENT AIM (AimTargetModel)");
        ImGui::Separator();

        ImGui::Checkbox("Enable Silent Aim", &cfg.silent_enabled);

        if (cfg.silent_enabled) {
            ImGui::SliderFloat("FOV", &cfg.silent_fov, 1.0f, 45.0f, "%.1f°");

            const char* bones[] = { "Head", "Chest" };
            ImGui::Combo("Target Bone", &cfg.silent_bone, bones, IM_ARRAYSIZE(bones));

            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                "Locks AimTargetModel without moving camera view.");
        }

        ImGui::EndTabItem();
    }

    static void TabMovement(CheatSettings& cfg) {
        if (!ImGui::BeginTabItem("Movement")) return;

        // Speed hack
        ImGui::Text("SPEED HACK");
        ImGui::Checkbox("Enable Speed Hack", &cfg.speed_enabled);
        if (cfg.speed_enabled) {
            ImGui::SliderFloat("Speed Multiplier", &cfg.speed_multiplier, 1.0f, 10.0f, "%.1fx");
            ImGui::Text("Hold SHIFT to activate");
        }

        ImGui::Separator();

        // Noclip
        ImGui::Text("NOCLIP / FLY");
        ImGui::Checkbox("Enable Noclip", &cfg.noclip_enabled);
        if (cfg.noclip_enabled) {
            ImGui::SliderFloat("Fly Speed", &cfg.noclip_speed, 1.0f, 50.0f, "%.1f");
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                "Hold Mouse4 + WASD to fly");
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                "Space = Up, Ctrl = Down");
        }

        ImGui::EndTabItem();
    }

    static void TabLoot(CheatSettings& cfg) {
        if (!ImGui::BeginTabItem("Loot")) return;

        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.7f, 1.0f), "FAST LOOT / LOOT VACUUM");
        ImGui::Separator();

        ImGui::Checkbox("Enable Fast Loot (Vacuum)", &cfg.fastloot_enabled);
        if (cfg.fastloot_enabled) {
            ImGui::Checkbox("Trigger Only On Key (Hold E)", &cfg.fastloot_on_key);
            ImGui::SliderFloat("Vacuum Radius", &cfg.fastloot_radius, 5.0f, 150.0f, "%.0f m");
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                "Teleports all ground items to your feet instantly.");
        }

        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "LOOT ESP");
        ImGui::Checkbox("Enable Loot ESP", &cfg.esp_loot);
        if (cfg.esp_loot) {
            ImGui::SliderFloat("Loot ESP Max Distance", &cfg.esp_loot_max_dist, 20.0f, 300.0f, "%.0f m");
        }

        ImGui::Separator();
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "FAST SEARCH / INSTANT OPEN");
        ImGui::Checkbox("Instant Search (No Loading)", &cfg.fastsearch_enabled);
        if (cfg.fastsearch_enabled) {
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                "Opens crates & containers instantly without search progress wheel.");
        }

        ImGui::EndTabItem();
    }

    static void TabExploit(CheatSettings& cfg) {
        if (!ImGui::BeginTabItem("Exploit")) return;

        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f),
            "FENCE MONEY EXPLOIT");
        ImGui::Separator();

        ImGui::Checkbox("Enable Exploit", &cfg.exploit_enabled);

        if (cfg.exploit_enabled) {
            ImGui::InputInt("Sell Value Override", &cfg.exploit_sell_value, 100000, 1000000);
            if (cfg.exploit_sell_value < 0) cfg.exploit_sell_value = 0;

            ImGui::Separator();

            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f),
                "How it works:");
            ImGui::BulletText("Press F8 to modify all item values in inventory");
            ImGui::BulletText("Sell items to Fence while values are modified");
            ImGui::BulletText("Press F9 to restore original values");

            ImGui::Separator();

            static int money_amount = 10000000;
            ImGui::InputInt("Set Money (client)", &money_amount, 100000, 1000000);
            if (ImGui::Button("Apply Money (F10)")) {
                // Handled in main loop
            }
        }

        ImGui::EndTabItem();
    }

    static void TabMisc(CheatSettings& cfg) {
        if (!ImGui::BeginTabItem("Misc")) return;

        ImGui::Text("HOTKEYS");
        ImGui::BulletText("INSERT  — Toggle menu");
        ImGui::BulletText("END     — Exit cheat");
        ImGui::BulletText("RMB     — Aimbot (hold)");
        ImGui::BulletText("SHIFT   — Speed hack (hold)");
        ImGui::BulletText("Mouse4  — Noclip (hold)");
        ImGui::BulletText("E       — Fast Loot / Vacuum (hold)");
        ImGui::BulletText("F8      — Execute Fence exploit");
        ImGui::BulletText("F9      — Restore item values");

        ImGui::Separator();
        ImGui::Text("STATUS & BACKEND");
        if (g_driver) {
            ImGui::Text("Engine: %s", g_driver->GetModeName());
            ImGui::Text("Target PID: %u", g_driver->pid);
            ImGui::Text("GameAssembly: 0x%llX", g_driver->ga_base);
            ImGui::Text("UnityPlayer:  0x%llX", g_driver->unity_base);
        }

        static float fps_timer = 0;
        static int fps_count = 0;
        static int fps_display = 0;
        fps_count++;
        fps_timer += ImGui::GetIO().DeltaTime;
        if (fps_timer >= 1.0f) {
            fps_display = fps_count;
            fps_count = 0;
            fps_timer = 0;
        }
        ImGui::Text("FPS: %d", fps_display);
        ImGui::Text("Resolution: %dx%d", (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);

        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f),
            "RONRUN v1.0 — Rob Run External");

        ImGui::EndTabItem();
    }
};
