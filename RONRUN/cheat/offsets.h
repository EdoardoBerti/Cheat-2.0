#pragma once
// ============================================================================
//  Game Offsets — Rob Run (Unity IL2CPP)
//  Extracted directly from GameAssembly.dll / Rob Run IL2CPP data
// ============================================================================

#include <cstdint>

namespace Offsets {

    // ========================================================================
    //  Core / Battle Architecture
    // ========================================================================
    namespace UnityEntry {
        constexpr uint64_t _game = 0x28;                         // -> Game
    }

    namespace Game {
        constexpr uint64_t _startGameProcessor = 0x98;           // -> StartGameProcessor
    }

    namespace GameModel {
        constexpr uint64_t _battleModel = 0x10;                  // -> BattleModel
    }

    namespace GlobalModel {
        constexpr uint64_t _sensitivityModel = 0x58;             // -> SensitivityModel
    }

    namespace BattleModel {
        constexpr uint64_t PlayerRotationInput      = 0x40;      // -> PlayerRotationInputModel
        constexpr uint64_t BattleEntitiesModel      = 0x90;      // -> BattleEntitiesModel
        constexpr uint64_t AimTargetModel           = 0x198;     // -> AimTargetModel
        constexpr uint64_t BattleCameraModel        = 0x358;     // -> BattleCameraModel
        constexpr uint64_t BattleCharacterSlotsModel = 0x4D0;    // -> BattleCharacterSlotsModel
        constexpr uint64_t WorldEntitiesModel       = 0x518;     // -> WorldEntitiesModel
        constexpr uint64_t VoiceChatBattleRoomModel = 0x630;     // -> VoiceChatBattleRoomModel
    }

    // ========================================================================
    //  Camera / Aim / Input
    // ========================================================================
    namespace AimTargetModel {
        constexpr uint64_t CharacterId = 0x10;
        constexpr uint64_t TargetType  = 0x14;                   // 3 = character
    }

    namespace BattleAimTargetUpdater {
        constexpr uint64_t _battleEntitiesModel = 0x30;
        constexpr uint64_t _playerRotationInput = 0x38;
        constexpr uint64_t _cinemachineBrain   = 0x48;
    }

    namespace BattleCameraModel {
        constexpr uint64_t LookAtCameraOrientation = 0x2C;
    }

    namespace CameraState {
        constexpr uint64_t FieldOfView           = 0xB0;
        constexpr uint64_t Dutch                 = 0xC0;
        constexpr uint64_t RawPosition           = 0xFC;         // Vector3
        constexpr uint64_t RawOrientation        = 0x108;        // Quaternion
        constexpr uint64_t PositionCorrection    = 0x128;
        constexpr uint64_t OrientationCorrection = 0x134;
    }

    namespace CinemachineBrain {
        constexpr uint64_t mActiveCameraPreviousFrame = 0xA0;
    }

    namespace CinemachineVirtualCamera {
        constexpr uint64_t m_Lens = 0xB8;
    }

    namespace PlayerRotationInputModel {
        constexpr uint64_t _yaw   = 0x10;                        // float
        constexpr uint64_t _pitch = 0x14;                        // float
    }

    namespace SensitivityModel {
        constexpr uint64_t PlayerPreferenceValue    = 0x10;      // float
        constexpr uint64_t PlayerPreferenceAimValue = 0x14;      // float
    }

    // ========================================================================
    //  Entities / Updaters / Characters
    // ========================================================================
    namespace BattleCharacterCommonData {
        constexpr uint64_t ItemInHands = 0xB8;
    }

    namespace BattleCharacterModel {
        constexpr uint64_t LookDirection  = 0x70;                // -> LookDirectionModel
        constexpr uint64_t SkeletonModel  = 0x138;
        constexpr uint64_t InVehicleModel = 0x1C8;               // -> InVehicleModel
        constexpr uint64_t StateModel     = 0x1E0;               // -> BattleCharacterStateModel
    }

    namespace BattleCharacterStateModel {
        constexpr uint64_t _characterHiddenStateModel = 0x20;
    }

    namespace CharacterHideObjectStateModel {
        constexpr uint64_t _hideObjectEntityIdData = 0x10;
    }

    namespace LookDirectionModel {
        constexpr uint64_t _value = 0x10;                        // Vector3
        constexpr uint64_t _yaw   = 0x1C;                        // float
        constexpr uint64_t _pitch = 0x20;                        // float
    }

    namespace InVehicleModel {
        constexpr uint64_t _entityId    = 0x10;
        constexpr uint64_t _vehicleType = 0x20;
    }

    namespace BattleEntitiesModel {
        constexpr uint64_t _models             = 0x50;           // List/Array of BattleCharacterModel
        constexpr uint64_t _worldEntitiesModel = 0x60;           // -> WorldEntitiesModel
        constexpr uint64_t _characterSlotModel = 0x80;           // -> BattleCharacterSlotsModel
        constexpr uint64_t _globalModel        = 0xB0;           // -> GlobalModel
    }

    namespace BattleEntitiesUpdater {
        constexpr uint64_t _battleEntitiesModel   = 0x18;
        constexpr uint64_t _battleModel           = 0x70;
        constexpr uint64_t _characterEntityViews  = 0x110;
        constexpr uint64_t _carEntityViews        = 0x118;
        constexpr uint64_t _helicopterEntityViews = 0x120;
    }

    namespace BattleScreensUpdater {
        constexpr uint64_t _updaters = 0x28;
    }

    namespace BattleScreenUpdater {
        constexpr uint64_t _updater = 0x18;
    }

    namespace BattleUpdater {
        constexpr uint64_t _battleEntitiesUpdater = 0x90;
    }

    namespace CompositeUpdater {
        constexpr uint64_t _updaters = 0x10;
    }

    namespace ScreenManager {
        constexpr uint64_t _currentUpdater = 0x50;
    }

    namespace StartGameProcessor {
        constexpr uint64_t _screenManager = 0x20;
    }

    // ========================================================================
    //  Slots & Player Identification
    // ========================================================================
    namespace BattleCharacterSlotsModel {
        constexpr uint64_t _slots = 0x10;                        // List of slots
    }

    namespace BattleCharacterSlotModel {
        constexpr uint64_t _data_1      = 0x10;                  // -> BattleCharacterSlotData
        constexpr uint64_t Description  = 0x18;
    }

    namespace BattleCharacterSlotData {
        constexpr uint64_t EntityId         = 0x20;              // int
        constexpr uint64_t Nickname         = 0x30;              // -> Il2CppString
        constexpr uint64_t EOSProductUserId = 0x40;              // -> Il2CppString
    }

    // ========================================================================
    //  World & Loot
    // ========================================================================
    namespace WorldEntitiesModel {
        constexpr uint64_t _containers   = 0x20;                 // List<ContainerModel>
        constexpr uint64_t _loots        = 0x38;                 // List<LootModel>
        constexpr uint64_t _vehicleLoots = 0x40;                 // List<VehicleLootModel>
        constexpr uint64_t _doors        = 0x48;                 // List<DoorModel>
        constexpr uint64_t _cars         = 0x60;                 // List<CarModel>
        constexpr uint64_t _helicopters  = 0x68;                 // List<HelicopterModel>
        constexpr uint64_t _hideObjects  = 0x1B0;                // List<HideObjectModel>
    }

    namespace PositionModel {
        constexpr uint64_t X = 0x10;                             // float
        constexpr uint64_t Y = 0x14;                             // float
        constexpr uint64_t Z = 0x18;                             // float
    }

    namespace InteractPositionModel {
        constexpr uint64_t X = 0x10;                             // float
        constexpr uint64_t Y = 0x14;                             // float
        constexpr uint64_t Z = 0x18;                             // float
    }

    namespace ContainerModel {
        constexpr uint64_t PositionModel  = 0x28;                 // -> PositionModel
        constexpr uint64_t IsSearched     = 0x30;                 // bool
        constexpr uint64_t SearchProgress = 0x34;                 // float (1.0f = complete)
        constexpr uint64_t SearchDuration = 0x38;                 // float (0.0f = instant)
    }

    namespace LootModel {
        constexpr uint64_t InteractPositionModel = 0x28;         // -> InteractPositionModel
    }

    namespace VehicleLootModel {
        constexpr uint64_t InteractPositionModel = 0x28;         // -> InteractPositionModel
    }

    namespace DoorModel {
        constexpr uint64_t State    = 0x30;                      // -> DoorStateModel
        constexpr uint64_t Position = 0x38;                      // Vector3 or PositionModel
    }

    namespace DoorStateModel {
        constexpr uint64_t _state = 0x10;                        // int
    }

    namespace HideObjectModel {
        constexpr uint64_t _entityId   = 0x10;
        constexpr uint64_t Description = 0x18;
    }

    namespace LocationHideObjectDescription {
        constexpr uint64_t EntityId      = 0x20;
        constexpr uint64_t Position      = 0x30;
        constexpr uint64_t Orientation   = 0x3C;
        constexpr uint64_t InteractPoint = 0x4C;
    }

    // ========================================================================
    //  Vehicles
    // ========================================================================
    namespace CarModel {
        constexpr uint64_t Id                   = 0x10;
        constexpr uint64_t OwnerRole            = 0x48;
        constexpr uint64_t ControlOwner         = 0x50;          // -> CarControlOwnerModel
        constexpr uint64_t Description          = 0x78;          // -> CarDescription
        constexpr uint64_t TireFrictionSettings = 0x88;
        constexpr uint64_t SteeringSettings     = 0x90;
        constexpr uint64_t TransmissionSettings = 0x98;
        constexpr uint64_t Transform            = 0xC0;          // -> Transform
        constexpr uint64_t Seats                = 0xE0;          // -> VehicleSeatsModel
    }

    namespace CarControlOwnerModel {
        constexpr uint64_t Driver = 0x18;
    }

    namespace CarDescription {
        constexpr uint64_t _physics  = 0xD0;                     // -> CarPhysics
        constexpr uint64_t _handling = 0xD8;                     // -> CarHandling
    }

    namespace CarHandling {
        constexpr uint64_t ForwardMaxVelocity            = 0x10; // float
        constexpr uint64_t AccelerationWindowMaxVelocity = 0x34; // float
        constexpr uint64_t HandBreakAcceleration         = 0x40; // float
    }

    namespace CarPhysics {
        constexpr uint64_t GravityForce          = 0x30;         // float
        constexpr uint64_t DownForceCoefficient  = 0x44;         // float
    }

    namespace CarSteeringDescription {
        constexpr uint64_t MaxSteeringAngle          = 0x20;     // float
        constexpr uint64_t SteeringAngleChangeSpeed  = 0x30;     // float
    }

    namespace CarTireFrictionDescription {
        constexpr uint64_t FwFrictionStiffness = 0x30;           // float
        constexpr uint64_t BwFrictionStiffness = 0x48;           // float
    }

    namespace CarTransmissionDescription {
        constexpr uint64_t DriveForceAcceleration = 0x28;        // float
        constexpr uint64_t BrakeForceAcceleration = 0x30;        // float
    }

    namespace HelicopterModel {
        constexpr uint64_t Id          = 0x10;
        constexpr uint64_t Description = 0x48;
        constexpr uint64_t Transform   = 0x70;                   // -> Transform
        constexpr uint64_t Seats       = 0x90;                   // -> VehicleSeatsModel
        constexpr uint64_t OwnerRole   = 0xE8;
    }

    namespace VehicleSeatsModel {
        constexpr uint64_t _data_1 = 0x10;                       // Array of VehicleSeatData
    }

    namespace VehicleSeatData {
        constexpr uint64_t Occupied = 0x20;                      // bool
        constexpr uint64_t EntityId = 0x28;                      // int
    }

    // ========================================================================
    //  Voice Chat
    // ========================================================================
    namespace VoiceChatBattleRoomModel {
        constexpr uint64_t ParticipantsModel = 0x38;
    }

    namespace VoiceChatParticipantsModel {
        constexpr uint64_t _participantStringIdToModels = 0x18;
    }

    namespace VoiceChatBattleParticipantModel {
        constexpr uint64_t SlotDescription    = 0x38;
        constexpr uint64_t HideObjectEntityId = 0x64;
    }

    namespace VoiceChatRoomParticipantModel {
        constexpr uint64_t ProductUserIdString = 0x18;
        constexpr uint64_t IsSpeaking          = 0x21;           // bool
    }

    // ========================================================================
    //  Function RVAs (GameAssembly.dll Base + RVA)
    // ========================================================================
    namespace RVAs {
        // Camera / Cinemachine
        constexpr uint64_t ProcessActiveCamera    = 0xDCFA68;    // brain push call
        constexpr uint64_t PushStateToUnityCamera = 0xDCFAF0;
        constexpr uint64_t BrainOutputCamera      = 0xDD1780;
        constexpr uint64_t Camera_GetFieldOfView  = 0x61F3340;

        // Aim / Rotation
        constexpr uint64_t SetRotation            = 0xC4A800;    // PlayerRotationInputModel.SetRotation
        constexpr uint64_t RaycastDirection       = 0x48A1EB0;
        constexpr uint64_t RaycastHitPosition     = 0x48A1F80;

        // Sky / Fog / Sun / Light
        constexpr uint64_t Camera_GetClearColour  = 0x61F26E0;
        constexpr uint64_t Camera_GetClearFlags   = 0x61F2AE0;
        constexpr uint64_t Camera_SetClearColour  = 0x61F5DB0;
        constexpr uint64_t Camera_SetClearFlags   = 0x61F6100;
        constexpr uint64_t Fog_Get                = 0x6223D40;
        constexpr uint64_t Sun_Object             = 0x6224060;
        constexpr uint64_t Fog_Set                = 0x62246F0;
        constexpr uint64_t Skybox_GetMaterial     = 0x62252D0;
        constexpr uint64_t Skybox_SetMaterial     = 0x6225B30;
        constexpr uint64_t Light_GetIntensity     = 0x62381A0;
        constexpr uint64_t Light_SetIntensity     = 0x6239FB0;

        // Materials / Shaders / Textures
        constexpr uint64_t Material_HasProperty   = 0x6241600;
        constexpr uint64_t Material_SetTexture    = 0x6245340;
        constexpr uint64_t Material_GetShader     = 0x62468F0;

        // Mesh manipulation
        constexpr uint64_t Mesh_RecalcBounds      = 0x6253110;
        constexpr uint64_t Mesh_Clear             = 0x625AE60;
        constexpr uint64_t Mesh_SetVertices       = 0x625C4E0;
        constexpr uint64_t Mesh_SetTriangles      = 0x625C670;
        constexpr uint64_t Mesh_SetUV             = 0x625CA70;
        constexpr uint64_t Mesh_SetNormals        = 0x625CBA0;

        // GameObjects / Transforms / Components
        constexpr uint64_t Transform_SetLook      = 0x452BC60;
        constexpr uint64_t Transform_SetParent    = 0x62E1340;
        constexpr uint64_t Transform_GetPosition  = 0x62E3960;
        constexpr uint64_t Renderer_GetColour     = 0x62E3B60;
        constexpr uint64_t Renderer_SetEnabled    = 0x6227000;
        constexpr uint64_t GameObject_SetActive   = 0x62A0DC0;

        // Cursor
        constexpr uint64_t CursorLock_Add         = 0xF15C00;
        constexpr uint64_t CursorLock_Remove      = 0xF15C70;
    }

    // Pattern for Camera static reference if needed
    namespace Patterns {
        inline const char* CameraMain = "48 8B 05 ? ? ? ? 48 85 C0 74 ? 48 8B 00";
    }
}
