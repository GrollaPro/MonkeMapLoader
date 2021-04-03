#pragma once

#include "Models/MapInfo.hpp"
#include "Models/GlobalData.hpp"
#include "Behaviours/MapDescriptor.hpp"


#include "custom-types/shared/macros.hpp"
#include "typedefs.h"

namespace MapLoader
{
    enum class LoadState {
        LoadingBundle,
        LoadingScene,
        InitializingMap
    };

    struct MapLoadData
    {
        MapInfo info;
        bool moveNext = false;
        LoadState loadState = LoadState::LoadingBundle;
        Scene scene;
        Il2CppObject* bundle = nullptr;
        Il2CppObject* root = nullptr;
    };
}

DECLARE_CLASS(MapLoader, Loader, "UnityEngine", "MonoBehaviour", sizeof(Il2CppObject) + sizeof(void*) + sizeof(MapLoadData),
    DECLARE_STATIC_FIELD(Il2CppObject*, mapInstance);
    DECLARE_STATIC_FIELD(bool, isLoading);
    DECLARE_STATIC_FIELD(bool, isMoved);

    DECLARE_METHOD(static void, ColorTreeTeleporter, Color color);
    DECLARE_METHOD(static void, JoinGame);
    DECLARE_METHOD(static void, ForceRespawn);
    DECLARE_METHOD(static void, ResetMapProperties);
    
    DECLARE_METHOD(void, Awake);
    DECLARE_METHOD(void, Update);
    DECLARE_METHOD(void, UnloadMap);
    DECLARE_METHOD(void, Initialize);
    DECLARE_METHOD(void, LoadBundle);
    DECLARE_METHOD(void, LoadScene);
    DECLARE_METHOD(static void, SceneComplete, void*, Il2CppObject* sceneLoadRequest);
    DECLARE_METHOD(void, InitializeMap);
    DECLARE_METHOD(void, InitializeGlobalData);
    DECLARE_METHOD(void, ProcessMap, Il2CppObject* map);
    DECLARE_METHOD(void, ProcessChildren, Il2CppObject* parent);
    DECLARE_METHOD(void, SetupCollisions, Il2CppObject* child);

    public:
        static inline std::string lobbyName = "";
        static inline GlobalData* globalData = nullptr;
        static inline MapDescriptor* mapDescriptor = nullptr;

        void LoadMap(MapInfo info);
    private:
        MapLoadData mapLoadData;

    REGISTER_FUNCTION(Loader,
        REGISTER_METHOD(Awake);
        REGISTER_METHOD(Update);
        REGISTER_METHOD(UnloadMap);
        REGISTER_METHOD(Initialize);
        REGISTER_METHOD(LoadBundle);
        REGISTER_METHOD(LoadScene);
        REGISTER_METHOD(SceneComplete);
        REGISTER_METHOD(InitializeMap);
        REGISTER_METHOD(InitializeGlobalData);
        REGISTER_METHOD(ColorTreeTeleporter);
        REGISTER_METHOD(JoinGame);
        REGISTER_METHOD(ForceRespawn);
        REGISTER_METHOD(ResetMapProperties);
        REGISTER_METHOD(ProcessMap);
        REGISTER_METHOD(ProcessChildren);
        REGISTER_METHOD(SetupCollisions);
        REGISTER_FIELD(mapInstance);
        REGISTER_FIELD(isLoading);
        REGISTER_FIELD(isMoved);
    )
)