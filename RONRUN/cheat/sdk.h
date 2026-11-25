#pragma once
// ============================================================================
//  Game SDK — Runtime representations for Rob Run (Unity IL2CPP)
//  Built from reverse engineering of GameAssembly.dll
// ============================================================================

#include "offsets.h"
#include "cheat_math.h"
#include "driver_comm.h"
#include <string>
#include <vector>

// Forward declare the global driver
extern DriverComm* g_driver;

// ---- IL2CPP String ----

struct Il2CppString {
    static std::wstring Read(uint64_t addr) {
        if (!addr) return L"";
        int32_t len = g_driver->Read<int32_t>(addr + 0x10);
        if (len <= 0 || len > 256) return L"";

        std::wstring str(len, L'\0');
        g_driver->ReadRaw(addr + 0x14, str.data(), len * sizeof(wchar_t));
        return str;
    }
};

// ---- Unity Transform ----

struct CTransform {
    uint64_t address;

    CTransform() : address(0) {}
    CTransform(uint64_t addr) : address(addr) {}

    Vec3 GetPosition() const {
        if (!address) return {};
        return g_driver->Read<Vec3>(address + 0x90);
    }

    void SetPosition(const Vec3& pos) const {
        if (!address) return;
        g_driver->Write<Vec3>(address + 0x90, pos);
    }

    bool IsValid() const { return address != 0; }
};

// ---- Camera ----

struct CCamera {
    uint64_t address;

    CCamera() : address(0) {}
    CCamera(uint64_t addr) : address(addr) {}

    Matrix4x4 GetViewProjectionMatrix() const {
        if (!address) return {};
        return g_driver->Read<Matrix4x4>(address + 0x2E4);
    }

    float GetScreenWidth() const {
        if (!address) return 1920.0f;
        int32_t w = g_driver->Read<int32_t>(address + 0x1A8);
        return (w > 0) ? (float)w : 1920.0f;
    }

    float GetScreenHeight() const {
        if (!address) return 1080.0f;
        int32_t h = g_driver->Read<int32_t>(address + 0x1AC);
        return (h > 0) ? (float)h : 1080.0f;
    }

    Vec3 GetPosition() const {
        if (!address) return {};
        return g_driver->Read<Vec3>(address + Offsets::CameraState::RawPosition);
    }

    bool IsValid() const { return address != 0; }
};

// ---- Player Rotation Input ----

struct CPlayerRotationInput {
    uint64_t address;
    CPlayerRotationInput(uint64_t addr = 0) : address(addr) {}
    bool IsValid() const { return address != 0; }

    float GetYaw() const {
        return g_driver->Read<float>(address + Offsets::PlayerRotationInputModel::_yaw);
    }
    void SetYaw(float yaw) const {
        g_driver->Write<float>(address + Offsets::PlayerRotationInputModel::_yaw, yaw);
    }

    float GetPitch() const {
        return g_driver->Read<float>(address + Offsets::PlayerRotationInputModel::_pitch);
    }
    void SetPitch(float pitch) const {
        g_driver->Write<float>(address + Offsets::PlayerRotationInputModel::_pitch, pitch);
    }

    Vec2 GetAngles() const {
        return { GetPitch(), GetYaw() };
    }

    void SetAngles(const Vec2& angles) const {
        SetPitch(angles.x);
        SetYaw(angles.y);
    }
};

// ---- Aim Target Model (Silent Aim) ----

struct CAimTargetModel {
    uint64_t address;
    CAimTargetModel(uint64_t addr = 0) : address(addr) {}
    bool IsValid() const { return address != 0; }

    int GetCharacterId() const {
        return g_driver->Read<int32_t>(address + Offsets::AimTargetModel::CharacterId);
    }
    void SetCharacterId(int id) const {
        g_driver->Write<int32_t>(address + Offsets::AimTargetModel::CharacterId, id);
    }

    int GetTargetType() const {
        return g_driver->Read<int32_t>(address + Offsets::AimTargetModel::TargetType);
    }
    void SetTargetType(int type) const {
        g_driver->Write<int32_t>(address + Offsets::AimTargetModel::TargetType, type);
    }
};

// ---- Player Entity ----

struct CPlayer {
    uint64_t address;

    CPlayer() : address(0) {}
    CPlayer(uint64_t addr) : address(addr) {}

    bool IsValid() const { return address != 0; }

    float GetHealth() const {
        return 100.0f;
    }

    float GetMaxHealth() const {
        return 100.0f;
    }

    bool IsAlive() const {
        return address != 0;
    }

    bool IsLocal() const {
        return false;
    }

    int GetTeamID() const {
        return 1;
    }

    std::wstring GetName() const {
        return L"Player";
    }

    CTransform GetTransform() const {
        return CTransform(address);
    }

    Vec3 GetPosition() const {
        if (!address) return {};
        uint64_t look_dir = g_driver->Read<uint64_t>(address + Offsets::BattleCharacterModel::LookDirection);
        if (look_dir) {
            return g_driver->Read<Vec3>(look_dir + Offsets::LookDirectionModel::_value);
        }
        return {};
    }

    Vec3 GetHeadPosition() const {
        Vec3 pos = GetPosition();
        pos.y += 1.8f;
        return pos;
    }

    Vec3 GetBonePosition(int bone_index) const {
        Vec3 pos = GetPosition();
        if (bone_index == 0) pos.y += 1.8f;      // Head
        else if (bone_index == 1) pos.y += 1.6f; // Neck
        else if (bone_index == 2) pos.y += 1.3f; // Chest
        else if (bone_index == 3) pos.y += 1.0f; // Spine
        else if (bone_index == 4) pos.y += 0.8f; // Pelvis
        return pos;
    }

    Vec2 GetViewAngles() const {
        if (!address) return {};
        uint64_t look_dir = g_driver->Read<uint64_t>(address + Offsets::BattleCharacterModel::LookDirection);
        if (look_dir) {
            float yaw   = g_driver->Read<float>(look_dir + Offsets::LookDirectionModel::_yaw);
            float pitch = g_driver->Read<float>(look_dir + Offsets::LookDirectionModel::_pitch);
            return { pitch, yaw };
        }
        return {};
    }

    void SetViewAngles(const Vec2& angles) const {
        uint64_t look_dir = g_driver->Read<uint64_t>(address + Offsets::BattleCharacterModel::LookDirection);
        if (look_dir) {
            g_driver->Write<float>(look_dir + Offsets::LookDirectionModel::_pitch, angles.x);
            g_driver->Write<float>(look_dir + Offsets::LookDirectionModel::_yaw,   angles.y);
        }
    }

    Vec3 GetAimRotation() const { return {}; }
    void SetAimRotation(const Vec3&) const {}

    float GetMoveSpeed() const { return 5.0f; }
    void SetMoveSpeed(float) const {}

    Vec3 GetVelocity() const { return {}; }
    void SetVelocity(const Vec3&) const {}

    bool IsGrounded() const { return true; }
    bool IsFiring() const { return false; }
};

// ---- Loot Model (Fast Loot & Loot ESP) ----

struct CLoot {
    uint64_t address;

    CLoot(uint64_t addr = 0) : address(addr) {}
    bool IsValid() const { return address != 0; }

    Vec3 GetPosition() const {
        if (!address) return {};
        uint64_t pos_model = g_driver->Read<uint64_t>(address + Offsets::LootModel::InteractPositionModel);
        if (!pos_model) return {};
        float x = g_driver->Read<float>(pos_model + Offsets::InteractPositionModel::X);
        float y = g_driver->Read<float>(pos_model + Offsets::InteractPositionModel::Y);
        float z = g_driver->Read<float>(pos_model + Offsets::InteractPositionModel::Z);
        return { x, y, z };
    }

    void SetPosition(const Vec3& pos) const {
        if (!address) return;
        uint64_t pos_model = g_driver->Read<uint64_t>(address + Offsets::LootModel::InteractPositionModel);
        if (!pos_model) return;
        g_driver->Write<float>(pos_model + Offsets::InteractPositionModel::X, pos.x);
        g_driver->Write<float>(pos_model + Offsets::InteractPositionModel::Y, pos.y);
        g_driver->Write<float>(pos_model + Offsets::InteractPositionModel::Z, pos.z);
    }
};

// ---- Container Model (Fast Search) ----

struct CContainer {
    uint64_t address;

    CContainer(uint64_t addr = 0) : address(addr) {}
    bool IsValid() const { return address != 0; }

    bool IsSearched() const {
        return g_driver->Read<bool>(address + Offsets::ContainerModel::IsSearched);
    }

    void InstantSearch() const {
        if (!address) return;
        g_driver->Write<bool>(address + Offsets::ContainerModel::IsSearched, true);
        g_driver->Write<float>(address + Offsets::ContainerModel::SearchProgress, 1.0f);
        g_driver->Write<float>(address + Offsets::ContainerModel::SearchDuration, 0.0f);
    }
};

// ---- Item (Fence Exploit) ----

struct CItem {
    uint64_t address;

    CItem() : address(0) {}
    CItem(uint64_t addr) : address(addr) {}

    std::wstring GetName() const {
        return L"LootItem";
    }

    int GetValue() const {
        return g_driver->Read<int32_t>(address + 0x18);
    }

    void SetValue(int value) const {
        g_driver->Write<int32_t>(address + 0x18, value);
    }

    int GetQuantity() const {
        return 1;
    }

    void SetQuantity(int) const {}
};

// ---- Game World ----

struct CGameWorld {
    uint64_t manager_addr;
    uint64_t battle_model;
    uint64_t entities_model;
    uint64_t world_entities_model;
    uint64_t rotation_input;
    uint64_t aim_target;

    CGameWorld() : manager_addr(0), battle_model(0), entities_model(0),
                   world_entities_model(0), rotation_input(0), aim_target(0) {}

    bool Init(uint64_t ga_base) {
        if (!ga_base) return false;
        manager_addr = ga_base;

        // Traverse UnityEntry -> Game -> GameModel -> BattleModel
        uint64_t game_ptr = g_driver->Read<uint64_t>(ga_base + Offsets::UnityEntry::_game);
        if (game_ptr) {
            uint64_t bm = g_driver->Read<uint64_t>(game_ptr + Offsets::GameModel::_battleModel);
            if (bm) {
                battle_model         = bm;
                entities_model       = g_driver->Read<uint64_t>(bm + Offsets::BattleModel::BattleEntitiesModel);
                world_entities_model = g_driver->Read<uint64_t>(bm + Offsets::BattleModel::WorldEntitiesModel);
                rotation_input       = g_driver->Read<uint64_t>(bm + Offsets::BattleModel::PlayerRotationInput);
                aim_target           = g_driver->Read<uint64_t>(bm + Offsets::BattleModel::AimTargetModel);
                return true;
            }
        }
        return false;
    }

    bool IsInGame() const {
        return battle_model != 0;
    }

    CPlayer GetLocalPlayer() const {
        return CPlayer(battle_model);
    }

    CPlayerRotationInput GetRotationInput() const {
        return CPlayerRotationInput(rotation_input);
    }

    CAimTargetModel GetAimTarget() const {
        return CAimTargetModel(aim_target);
    }

    int GetPlayerCount() const {
        return 10;
    }

    std::vector<CPlayer> GetPlayers() const {
        std::vector<CPlayer> players;
        if (!entities_model) return players;

        uint64_t list_ptr = g_driver->Read<uint64_t>(entities_model + Offsets::BattleEntitiesModel::_models);
        if (!list_ptr) return players;

        uint64_t items_array = g_driver->Read<uint64_t>(list_ptr + 0x10);
        int32_t count        = g_driver->Read<int32_t>(list_ptr + 0x18);

        if (!items_array || count <= 0 || count > 64) return players;

        players.reserve(count);
        for (int i = 0; i < count; i++) {
            uint64_t player_ptr = g_driver->Read<uint64_t>(items_array + 0x20 + (i * sizeof(uint64_t)));
            if (player_ptr)
                players.emplace_back(player_ptr);
        }

        return players;
    }

    std::vector<CLoot> GetLoots() const {
        std::vector<CLoot> loots;
        if (!world_entities_model) return loots;

        uint64_t loot_list = g_driver->Read<uint64_t>(world_entities_model + Offsets::WorldEntitiesModel::_loots);
        if (!loot_list) return loots;

        uint64_t items_array = g_driver->Read<uint64_t>(loot_list + 0x10);
        int32_t count        = g_driver->Read<int32_t>(loot_list + 0x18);

        if (!items_array || count <= 0 || count > 256) return loots;

        loots.reserve(count);
        for (int i = 0; i < count; i++) {
            uint64_t loot_ptr = g_driver->Read<uint64_t>(items_array + 0x20 + (i * sizeof(uint64_t)));
            if (loot_ptr)
                loots.emplace_back(loot_ptr);
        }
        return loots;
    }

    std::vector<CContainer> GetContainers() const {
        std::vector<CContainer> containers;
        if (!world_entities_model) return containers;

        uint64_t list_ptr = g_driver->Read<uint64_t>(world_entities_model + Offsets::WorldEntitiesModel::_containers);
        if (!list_ptr) return containers;

        uint64_t items_array = g_driver->Read<uint64_t>(list_ptr + 0x10);
        int32_t count        = g_driver->Read<int32_t>(list_ptr + 0x18);

        if (!items_array || count <= 0 || count > 256) return containers;

        containers.reserve(count);
        for (int i = 0; i < count; i++) {
            uint64_t ptr = g_driver->Read<uint64_t>(items_array + 0x20 + (i * sizeof(uint64_t)));
            if (ptr)
                containers.emplace_back(ptr);
        }
        return containers;
    }

    int GetMoney() const {
        return 0;
    }

    void SetMoney(int) const {}

    std::vector<CItem> GetInventoryItems() const {
        std::vector<CItem> items;
        if (!world_entities_model) return items;

        uint64_t loot_list = g_driver->Read<uint64_t>(world_entities_model + Offsets::WorldEntitiesModel::_loots);
        if (!loot_list) return items;

        uint64_t items_array = g_driver->Read<uint64_t>(loot_list + 0x10);
        int32_t count        = g_driver->Read<int32_t>(loot_list + 0x18);

        if (!items_array || count <= 0 || count > 128) return items;

        items.reserve(count);
        for (int i = 0; i < count; i++) {
            uint64_t item_ptr = g_driver->Read<uint64_t>(items_array + 0x20 + (i * sizeof(uint64_t)));
            if (item_ptr)
                items.emplace_back(item_ptr);
        }
        return items;
    }
};
