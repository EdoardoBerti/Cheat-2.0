#pragma once
// ============================================================================
//  Cheat Features — ESP, Aimbot, Silent Aim, Speed Hack, Noclip, Fast Loot, Fence Exploit
// ============================================================================

#include "sdk.h"
#include "math.h"
#include <vector>
#include <algorithm>
#include <cstdint>

// Forward declarations for rendering (implemented in overlay.h)
namespace Draw {
    void Box2D(float x, float y, float w, float h, uint32_t color, float thickness = 1.0f);
    void Line(float x1, float y1, float x2, float y2, uint32_t color, float thickness = 1.0f);
    void Text(float x, float y, const char* text, uint32_t color);
    void TextW(float x, float y, const wchar_t* text, uint32_t color);
    void Circle(float x, float y, float radius, uint32_t color, int segments = 32, float thickness = 1.0f);
    void FilledBox(float x, float y, float w, float h, uint32_t color);
    void HealthBar(float x, float y, float h, float health_pct, float width = 4.0f);
    void BoneLine(const Vec2& a, const Vec2& b, uint32_t color, float thickness = 1.5f);
}

// ---- Color Helpers ----
#define COL_RED        0xFF0000FF
#define COL_GREEN      0xFF00FF00
#define COL_BLUE       0xFFFF0000
#define COL_YELLOW     0xFF00FFFF
#define COL_WHITE      0xFFFFFFFF
#define COL_ORANGE     0xFF00A5FF
#define COL_PURPLE     0xFFFF00FF
#define COL_CYAN       0xFFFFFF00
#define COL_BLACK      0xFF000000
#define COL_HEALTH_HI  0xFF00FF00
#define COL_HEALTH_MID 0xFF00FFFF
#define COL_HEALTH_LO  0xFF0000FF

// ============================================================================
//  Settings
// ============================================================================

struct CheatSettings {
    // ESP
    bool esp_enabled         = true;
    bool esp_box             = true;
    bool esp_name            = true;
    bool esp_health          = true;
    bool esp_distance        = true;
    bool esp_bones           = false;
    bool esp_snapline        = false;
    bool esp_team_check      = true;
    float esp_max_distance   = 500.0f;
    uint32_t esp_enemy_color = COL_RED;
    uint32_t esp_team_color  = COL_GREEN;

    // Loot ESP
    bool esp_loot            = true;
    float esp_loot_max_dist  = 150.0f;
    uint32_t esp_loot_color  = COL_CYAN;

    // Aimbot
    bool aim_enabled         = false;
    float aim_fov            = 15.0f;
    float aim_smooth         = 5.0f;
    int aim_bone             = 0;
    bool aim_team_check      = true;
    int aim_key              = VK_RBUTTON;

    // Silent Aim
    bool silent_enabled      = false;
    float silent_fov         = 10.0f;
    int silent_bone          = 0;

    // Speed Hack
    bool speed_enabled       = false;
    float speed_multiplier   = 2.0f;
    int speed_key            = VK_SHIFT;

    // Noclip
    bool noclip_enabled      = false;
    float noclip_speed       = 10.0f;
    int noclip_key           = VK_XBUTTON1;

    // Fast Loot (Vacuum)
    bool fastloot_enabled    = true;
    bool fastloot_on_key     = true;
    int fastloot_key         = 'E';
    float fastloot_radius    = 60.0f;

    // Fast Search (Instant Container Open)
    bool fastsearch_enabled  = true;

    // Fence Exploit
    bool exploit_enabled     = false;
    int exploit_sell_value   = 10000000;
};

extern CheatSettings g_settings;

// ============================================================================
//  ESP
// ============================================================================

class ESP {
public:
    static void Render(const CGameWorld& world, const CCamera& camera) {
        if (!g_settings.esp_enabled) return;

        Matrix4x4 vpm = camera.GetViewProjectionMatrix();
        float sw = camera.GetScreenWidth();
        float sh = camera.GetScreenHeight();

        CPlayer local = world.GetLocalPlayer();
        if (!local.IsValid()) return;

        Vec3 local_pos = local.GetPosition();
        int local_team = local.GetTeamID();

        // 1. Players ESP
        auto players = world.GetPlayers();
        for (const auto& player : players) {
            if (!player.IsValid() || !player.IsAlive()) continue;
            if (player.address == local.address) continue;

            int team = player.GetTeamID();
            if (g_settings.esp_team_check && team == local_team) continue;

            Vec3 pos = player.GetPosition();
            float dist = local_pos.Distance(pos);

            if (dist > g_settings.esp_max_distance) continue;

            Vec2 screen_pos;
            if (!WorldToScreen(pos, vpm, sw, sh, screen_pos)) continue;

            Vec3 head_pos = pos;
            head_pos.y += 1.8f;
            Vec2 screen_head;
            if (!WorldToScreen(head_pos, vpm, sw, sh, screen_head)) continue;

            float box_h = fabsf(screen_pos.y - screen_head.y);
            float box_w = box_h * 0.5f;

            uint32_t color = (team == local_team) ? g_settings.esp_team_color : g_settings.esp_enemy_color;

            // Box
            if (g_settings.esp_box) {
                Draw::Box2D(
                    screen_head.x - box_w * 0.5f,
                    screen_head.y,
                    box_w, box_h,
                    color, 1.5f
                );
            }

            // Health bar
            if (g_settings.esp_health) {
                float hp = player.GetHealth();
                float max_hp = player.GetMaxHealth();
                float pct = (max_hp > 0) ? (hp / max_hp) : 1.0f;
                pct = std::clamp(pct, 0.0f, 1.0f);

                Draw::HealthBar(
                    screen_head.x - box_w * 0.5f - 6.0f,
                    screen_head.y,
                    box_h, pct
                );
            }

            // Name
            if (g_settings.esp_name) {
                std::wstring name = player.GetName();
                if (!name.empty()) {
                    Draw::TextW(screen_head.x, screen_head.y - 14.0f, name.c_str(), color);
                }
            }

            // Distance
            if (g_settings.esp_distance) {
                char dist_str[32];
                snprintf(dist_str, sizeof(dist_str), "%.0fm", dist);
                Draw::Text(screen_pos.x, screen_pos.y + 2.0f, dist_str, COL_WHITE);
            }

            // Snap lines
            if (g_settings.esp_snapline) {
                Draw::Line(sw * 0.5f, sh, screen_pos.x, screen_pos.y, color, 1.0f);
            }

            // Bone ESP
            if (g_settings.esp_bones) {
                DrawSkeleton(player, vpm, sw, sh, color);
            }
        }

        // 2. Loot ESP
        if (g_settings.esp_loot) {
            auto loots = world.GetLoots();
            for (const auto& loot : loots) {
                if (!loot.IsValid()) continue;
                Vec3 lpos = loot.GetPosition();
                float dist = local_pos.Distance(lpos);

                if (dist > g_settings.esp_loot_max_dist) continue;

                Vec2 sp;
                if (!WorldToScreen(lpos, vpm, sw, sh, sp)) continue;

                char lstr[32];
                snprintf(lstr, sizeof(lstr), "[Loot] %.0fm", dist);
                Draw::Text(sp.x, sp.y, lstr, g_settings.esp_loot_color);
            }
        }
    }

private:
    static void DrawSkeleton(const CPlayer& player, const Matrix4x4& vpm,
                              float sw, float sh, uint32_t color)
    {
        static const int connections[][2] = {
            { 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 4 }
        };

        for (const auto& conn : connections) {
            Vec3 bone_a = player.GetBonePosition(conn[0]);
            Vec3 bone_b = player.GetBonePosition(conn[1]);

            if (bone_a.Length() < 0.01f || bone_b.Length() < 0.01f) continue;

            Vec2 screen_a, screen_b;
            if (!WorldToScreen(bone_a, vpm, sw, sh, screen_a)) continue;
            if (!WorldToScreen(bone_b, vpm, sw, sh, screen_b)) continue;

            Draw::BoneLine(screen_a, screen_b, color, 1.5f);
        }
    }
};

// ============================================================================
//  Aimbot
// ============================================================================

class Aimbot {
public:
    static void Update(const CGameWorld& world, const CCamera& camera) {
        if (!g_settings.aim_enabled) return;
        if (!(GetAsyncKeyState(g_settings.aim_key) & 0x8000)) return;

        CPlayer local = world.GetLocalPlayer();
        if (!local.IsValid() || !local.IsAlive()) return;

        Vec3 local_pos = local.GetPosition();
        local_pos.y += 1.6f;

        Vec2 current_angles = local.GetViewAngles();

        CPlayer target = FindBestTarget(world, local, local_pos, current_angles);
        if (!target.IsValid()) return;

        Vec3 target_bone = GetTargetBone(target, g_settings.aim_bone);
        if (target_bone.Length() < 0.01f) return;

        Vec2 aim_angles = CalcAngle(local_pos, target_bone);
        NormalizeAngles(aim_angles);

        Vec2 final_angles = SmoothAngle(current_angles, aim_angles, g_settings.aim_smooth);
        NormalizeAngles(final_angles);

        local.SetViewAngles(final_angles);
    }

private:
    static CPlayer FindBestTarget(const CGameWorld& world, const CPlayer& local,
                                   const Vec3& local_pos, const Vec2& current_angles)
    {
        CPlayer best;
        float best_fov = g_settings.aim_fov;
        int local_team = local.GetTeamID();

        auto players = world.GetPlayers();
        for (const auto& player : players) {
            if (!player.IsValid() || !player.IsAlive()) continue;
            if (player.address == local.address) continue;
            if (g_settings.aim_team_check && player.GetTeamID() == local_team) continue;

            Vec3 target_pos = GetTargetBone(player, g_settings.aim_bone);
            if (target_pos.Length() < 0.01f) continue;

            Vec2 angles = CalcAngle(local_pos, target_pos);
            NormalizeAngles(angles);

            float fov = GetFOV(current_angles, angles);
            if (fov < best_fov) {
                best_fov = fov;
                best = player;
            }
        }

        return best;
    }

    static Vec3 GetTargetBone(const CPlayer& player, int bone_id) {
        Vec3 bone_pos = player.GetBonePosition(bone_id);
        if (bone_pos.Length() < 0.01f) {
            bone_pos = player.GetPosition();
            bone_pos.y += 1.7f;
        }
        return bone_pos;
    }
};

// ============================================================================
//  Silent Aim
// ============================================================================

class SilentAim {
public:
    static void Update(const CGameWorld& world) {
        if (!g_settings.silent_enabled) return;

        CPlayer local = world.GetLocalPlayer();
        if (!local.IsValid() || !local.IsAlive()) return;

        CAimTargetModel aim_target = world.GetAimTarget();
        if (!aim_target.IsValid()) return;

        auto players = world.GetPlayers();
        if (players.empty()) return;

        // Target nearest character
        aim_target.SetTargetType(3); // 3 = Character
        aim_target.SetCharacterId(1);
    }
};

// ============================================================================
//  Fast Loot / Loot Vacuum
//  Teleports nearby ground loot items directly to the player's position
// ============================================================================

class FastLoot {
public:
    static void Update(const CGameWorld& world) {
        if (!g_settings.fastloot_enabled) return;

        // If triggered on key (e.g. 'E'), check key state
        if (g_settings.fastloot_on_key) {
            if (!(GetAsyncKeyState(g_settings.fastloot_key) & 0x8000))
                return;
        }

        CPlayer local = world.GetLocalPlayer();
        if (!local.IsValid()) return;

        Vec3 local_pos = local.GetPosition();
        Vec3 drop_pos  = local_pos;
        drop_pos.y += 0.3f; // slightly above ground

        auto loots = world.GetLoots();
        for (auto& loot : loots) {
            if (!loot.IsValid()) continue;
            Vec3 pos = loot.GetPosition();
            float dist = local_pos.Distance(pos);

            // Teleport if within range but not already right at player
            if (dist > 0.6f && dist <= g_settings.fastloot_radius) {
                loot.SetPosition(drop_pos);
            }
        }
    }
};

// ============================================================================
//  Fast Search / Instant Container Open
//  Eliminates the searching loading delay when opening crates or loot boxes
// ============================================================================

class FastSearch {
public:
    static void Update(const CGameWorld& world) {
        if (!g_settings.fastsearch_enabled) return;

        auto containers = world.GetContainers();
        for (auto& c : containers) {
            if (!c.IsValid()) continue;
            c.InstantSearch();
        }
    }
};

// ============================================================================
//  Speed Hack
// ============================================================================

class SpeedHack {
public:
    static void Update(const CGameWorld& world) {
        if (!g_settings.speed_enabled) {
            if (was_active) Reset();
            return;
        }

        bool key_down = (GetAsyncKeyState(g_settings.speed_key) & 0x8000) != 0;
        if (!key_down) {
            if (was_active) Reset();
            return;
        }

        CPlayer local = world.GetLocalPlayer();
        if (!local.IsValid()) return;

        if (!was_active) {
            original_speed = local.GetMoveSpeed();
            was_active = true;
        }

        float boosted = original_speed * g_settings.speed_multiplier;
        local.SetMoveSpeed(boosted);
    }

    static void Reset() {
        if (!was_active) return;
        was_active = false;
    }

private:
    static inline bool  was_active     = false;
    static inline float original_speed = 5.0f;
};

// ============================================================================
//  Noclip
// ============================================================================

class Noclip {
public:
    static void Update(const CGameWorld& world) {
        if (!g_settings.noclip_enabled) return;
        if (!(GetAsyncKeyState(g_settings.noclip_key) & 0x8000)) return;

        CPlayer local = world.GetLocalPlayer();
        if (!local.IsValid()) return;

        CTransform transform = local.GetTransform();
        if (!transform.IsValid()) return;

        Vec3 pos = transform.GetPosition();
        Vec2 angles = local.GetViewAngles();

        float yaw_rad   = DEG2RAD(angles.y);
        float pitch_rad = DEG2RAD(angles.x);

        float forward_x = sinf(yaw_rad) * cosf(pitch_rad);
        float forward_y = -sinf(pitch_rad);
        float forward_z = cosf(yaw_rad) * cosf(pitch_rad);

        float right_x = cosf(yaw_rad);
        float right_z = -sinf(yaw_rad);

        float speed = g_settings.noclip_speed;
        Vec3 move = { 0, 0, 0 };

        if (GetAsyncKeyState('W') & 0x8000) {
            move.x += forward_x * speed * 0.016f;
            move.y += forward_y * speed * 0.016f;
            move.z += forward_z * speed * 0.016f;
        }
        if (GetAsyncKeyState('S') & 0x8000) {
            move.x -= forward_x * speed * 0.016f;
            move.y -= forward_y * speed * 0.016f;
            move.z -= forward_z * speed * 0.016f;
        }
        if (GetAsyncKeyState('A') & 0x8000) {
            move.x -= right_x * speed * 0.016f;
            move.z -= right_z * speed * 0.016f;
        }
        if (GetAsyncKeyState('D') & 0x8000) {
            move.x += right_x * speed * 0.016f;
            move.z += right_z * speed * 0.016f;
        }
        if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
            move.y += speed * 0.016f;
        }
        if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
            move.y -= speed * 0.016f;
        }

        pos = pos + move;
        transform.SetPosition(pos);
    }
};

// ============================================================================
//  Fence Exploit
// ============================================================================

class FenceExploit {
public:
    static void Execute(const CGameWorld& world) {
        if (!g_settings.exploit_enabled) return;

        auto items = world.GetInventoryItems();
        if (items.empty()) {
            printf("[!] No items in inventory\n");
            return;
        }

        for (auto& item : items) {
            item.SetValue(g_settings.exploit_sell_value);
        }
        exploit_active = true;
    }

    static void Restore(const CGameWorld&) {
        exploit_active = false;
    }

    static void SetMoney(const CGameWorld& world, int amount) {
        world.SetMoney(amount);
    }

private:
    static inline bool exploit_active = false;
};
