#include "Behaviours/MapLoader.hpp"
#include "libzip/shared/zip.h"
#include "static-defines.hpp"
#include "Behaviours/TagZone.hpp"
#include "Behaviours/Teleporter.hpp"
#include "Behaviours/Player.hpp"
#include "Behaviours/ObjectTrigger.hpp"

#include "Behaviours/PreviewOrb.hpp"
#include "Behaviours/RotateByHand.hpp"

#include "Utils/RoomUtils.hpp"
#include "quest-cosmetic-loader/shared/CosmeticLoader.hpp"

DEFINE_CLASS(MapLoader::Loader);

extern bool getSceneName(Scene scene, std::string& out);
extern Logger& getLogger();

static Color green = {0.0f, 1.0f, 0.0f};
static Color red = {1.0f, 0.0f, 0.0f};
static Color blue = {0.0f, 0.0f, 1.0f};

namespace MapLoader
{
    Il2CppObject* Loader::mapInstance = nullptr;
    bool Loader::isLoading = false;
    bool Loader::isMoved = false;
    void Loader::Awake()
    {}

    void Loader::Update()
    {
        if (!mapLoadData.moveNext) return;
        mapLoadData.moveNext = false;
        switch (mapLoadData.loadState)
        {
            case LoadState::LoadingBundle:
                LoadBundle();
                break;
            case LoadState::LoadingScene:
                LoadScene();
                break;
            case LoadState::InitializingMap:
                InitializeMap();
                break;
            default:
                break;
        }
    }

    void Loader::UnloadMap()
    {
        if (mapInstance)
        {
            il2cpp_utils::RunMethod("UnityEngine", "Object", "Destroy", mapInstance);
            
            mapInstance = nullptr;
            
            ColorTreeTeleporter(red);
            /*
            using UnloadUnusedAssets = function_ptr_t<void>;
            static UnloadUnusedAssets unloadUnusedAssets = reinterpret_cast<UnloadUnusedAssets>(il2cpp_functions::resolve_icall("UnityEngine.Resources::UnloadUnusedAssets"));
            unloadUnusedAssets();
            */
        }
    }

    void Loader::Initialize()
    {
        InitializeGlobalData();
    }

    void Loader::InitializeGlobalData()
    {
        if (!globalData)
            globalData = new GlobalData();

        static Il2CppString* slideName = il2cpp_utils::createcsstr("slide", il2cpp_utils::StringType::Manual);
        Il2CppObject* slide = *il2cpp_utils::RunMethod("UnityEngine", "GameObject", "Find", slideName);
        Il2CppObject* slideTransform = *il2cpp_utils::RunMethod(slide, "get_transform"); 
        Vector3 origin = *il2cpp_utils::RunMethod<Vector3>(slideTransform, "get_position");
        globalData->set_origin(origin);

        static Il2CppString* stoolName = il2cpp_utils::createcsstr("stool", il2cpp_utils::StringType::Manual);
        Il2CppObject* stool = *il2cpp_utils::RunMethod("UnityEngine", "GameObject", "Find", stoolName);
        Il2CppObject* stoolTransform = *il2cpp_utils::RunMethod(stool, "get_transform"); 
        Vector3 stoolPos = *il2cpp_utils::RunMethod<Vector3>(stoolTransform, "get_position");

        globalData->treeOrigin = stoolPos;

        using namespace CosmeticsLoader;
        // create map teleporter
        if (!globalData->bigTreeTeleportToMap)
        {
            std::string treeTeleporterPath = "/sdcard/ModData/com.AnotherAxiom.GorillaTag/Mods/MonkeMapLoader/Teleporter";
            auto* loader = new CosmeticLoader(treeTeleporterPath, [&](std::string name, Il2CppObject* teleporter){
                globalData->bigTreeTeleportToMap = teleporter;
                il2cpp_utils::RunMethod(teleporter, "set_layer", MASKLAYER_PLAYERTRIGGER);
                il2cpp_utils::RunMethod(teleporter, "DontDestroyOnLoad", teleporter);
            }, "_Teleporter", il2cpp_utils::GetSystemType("UnityEngine", "GameObject"));
        }
        
        std::string orbPath = "/sdcard/ModData/com.AnotherAxiom.GorillaTag/Mods/MonkeMapLoader/Orb";
        auto* loader = new CosmeticLoader(orbPath, [&](std::string name, Il2CppObject* orb){
            static std::vector<Il2CppClass*> orbKlass = {classof(PreviewOrb*)};
            static std::vector<Il2CppClass*> rotateKlass = {classof(RotateByHand*)};

            Il2CppObject* orbVisuals = *il2cpp_utils::RunMethod("UnityEngine", "Object", "Instantiate", orb);

            RotateByHand* rotateByHand = *il2cpp_utils::RunGenericMethod<RotateByHand*>(orb, "AddComponent", rotateKlass);

            static std::vector<Il2CppClass*> rendererKlass = {il2cpp_utils::GetClassFromName("UnityEngine", "Renderer")};
            Il2CppObject* renderer = *il2cpp_utils::RunGenericMethod<Il2CppObject*>(orb, "GetComponent", rendererKlass);
            il2cpp_utils::RunMethod(renderer, "set_enabled", false);
            il2cpp_utils::RunMethod("UnityEngine", "Object", "Destroy", renderer);
            il2cpp_utils::RunMethod(orb, "set_layer", 18);

            Il2CppObject* orbTransform = *il2cpp_utils::RunMethod(orb, "get_transform");
            Il2CppObject* visualsTransform = *il2cpp_utils::RunMethod(orbVisuals, "get_transform");
            
            il2cpp_utils::RunMethod(visualsTransform, "SetParent", orbTransform);
            
            static Vector3 zero = {0.0f, 0.0f, 0.0f};
            static Vector3 one = {1.0f, 1.0f, 1.0f};
            static Quaternion identity = *il2cpp_utils::RunMethod<Quaternion>("UnityEngine", "Quaternion", "get_identity");

            il2cpp_utils::RunMethod(visualsTransform, "set_localPosition", zero);
            il2cpp_utils::RunMethod(visualsTransform, "set_localScale", one);
            il2cpp_utils::RunMethod(visualsTransform, "set_localRotation", identity);

            PreviewOrb* previewOrb = *il2cpp_utils::RunGenericMethod<PreviewOrb*>(orbVisuals, "AddComponent", orbKlass);

            static std::vector<Il2CppClass*> colliderKlass = {il2cpp_utils::GetClassFromName("UnityEngine", "Collider")};
            Il2CppObject* collider = *il2cpp_utils::RunGenericMethod(orbVisuals, "GetComponent", colliderKlass);
            il2cpp_utils::RunMethod(collider, "set_enabled", false);
            il2cpp_utils::RunMethod("UnityEngine", "Object", "Destroy", collider);

        }, "_Orb", il2cpp_utils::GetSystemType("UnityEngine", "GameObject"));
        

        static std::vector<Il2CppClass*> teleporterKlass = {classof(Teleporter*)};
        Teleporter* treeTeleporter = *il2cpp_utils::RunGenericMethod<Teleporter*>(globalData->bigTreeTeleportToMap, "AddComponent", teleporterKlass);
        treeTeleporter->teleporterType = TeleporterType::Map;
        treeTeleporter->joinGameOnTeleport = true;
        treeTeleporter->Delay = 1.5f;
        treeTeleporter->touchType = TouchType::Head;

        il2cpp_utils::RunMethod("UnityEngine", "Object", "DontDestroyOnLoad", treeTeleporter);
        Il2CppObject* treePointTransform = nullptr;
        static Il2CppClass* goKlass = il2cpp_utils::GetClassFromName("UnityEngine", "GameObject");
        if (!globalData->bigTreePoint)
        {
            static Il2CppString* treeHomeTargetObjectName = il2cpp_utils::createcsstr("TreeHomeTargetObject", il2cpp_utils::StringType::Manual);
            globalData->bigTreePoint = *il2cpp_utils::New(goKlass);
            il2cpp_utils::RunMethod(globalData->bigTreePoint, "set_name", treeHomeTargetObjectName);
            treePointTransform = *il2cpp_utils::RunMethod(globalData->bigTreePoint, "get_transform");
            static Vector3 pos = {-66.0f, 12.3f, -83.0f};
            static Quaternion rot = *il2cpp_utils::RunMethod<Quaternion>("UnityEngine", "Quaternion", "Euler", 0.0f, -90.0f, 0.0f);

            il2cpp_utils::RunMethod(treePointTransform, "set_position", pos);
            il2cpp_utils::RunMethod(treePointTransform, "set_rotation", rot);
            il2cpp_utils::RunMethod("UnityEngine", "Object", "DontDestroyOnLoad", globalData->bigTreePoint);
        }
        else
        {
            treePointTransform = *il2cpp_utils::RunMethod(globalData->bigTreePoint, "get_transform");
        }

        il2cpp_utils::RunMethod(treeTeleporter->teleportPoints, "Add", treePointTransform);

        ColorTreeTeleporter(red);

        static Il2CppString* fallEmergencyTeleportName = il2cpp_utils::createcsstr("FallEmergencyTeleport", il2cpp_utils::StringType::Manual);

        globalData->fallEmergencyTeleport = *il2cpp_utils::New(goKlass);
        il2cpp_utils::RunMethod(globalData->fallEmergencyTeleport, "set_name", fallEmergencyTeleportName);

        il2cpp_utils::RunMethod(globalData->fallEmergencyTeleport, "set_layer", MASKLAYER_HANDTRIGGER);
        static std::vector<Il2CppClass*> boxColliderKlass = {il2cpp_utils::GetClassFromName("UnityEngine", "BoxCollider")};
        Il2CppObject* boxCollider = *il2cpp_utils::RunGenericMethod(globalData->fallEmergencyTeleport, "AddComponent", boxColliderKlass);
        il2cpp_utils::RunMethod(boxCollider, "set_isTrigger", true);

        Il2CppObject* emergencyTeleportTransform = *il2cpp_utils::RunMethod(globalData->fallEmergencyTeleport, "get_transform");
        Vector3 localScale = {1000.0f, 1.0f, 1000.0f};
        Vector3 position = {0.0f, -200.0f, 0.0f};
        Vector3 treeOrigin = globalData->treeOrigin;

        position.x += treeOrigin.x;
        position.y += treeOrigin.y;
        position.z += treeOrigin.z;

        il2cpp_utils::RunMethod(emergencyTeleportTransform, "set_localScale", localScale);
        il2cpp_utils::RunMethod(emergencyTeleportTransform, "set_position", position);


        Teleporter* emergencyTeleporter = *il2cpp_utils::RunGenericMethod<Teleporter*>(globalData->fallEmergencyTeleport, "AddComponent", teleporterKlass);
        il2cpp_utils::RunMethod(emergencyTeleporter->teleportPoints, "Add", treePointTransform);
        emergencyTeleporter->teleporterType = TeleporterType::Treehouse;
    }

    void Loader::ColorTreeTeleporter(Color color)
    {
        static std::vector<Il2CppClass*> rendererKlass = {il2cpp_utils::GetClassFromName("UnityEngine", "Renderer")};
        Array<Il2CppObject*>* renderers = *il2cpp_utils::RunGenericMethod<Array<Il2CppObject*>*>(globalData->bigTreeTeleportToMap, "GetComponentsInChildren", rendererKlass);

        for (int i = 0; i < renderers->Length(); i++)
        {
            Il2CppObject* renderer = renderers->values[i];
            Array<Il2CppObject*>* materials = *il2cpp_utils::RunMethod<Array<Il2CppObject*>*>(renderer, "get_materials");

            for (int j = 0; j < materials->Length(); j++)
            {
                Il2CppObject* material = materials->values[j];
                Il2CppString* csName = *il2cpp_utils::RunMethod<Il2CppString*>(material, "get_name");
                std::string name = to_utf8(csstrtostr(csName));

                if (name.find("Center") != std::string::npos)
                    il2cpp_utils::RunMethod(material, "set_color", color);
            }
        }
    }

    void Loader::JoinGame()
    {
        if (lobbyName != "")
        {
            RoomUtils::JoinModdedLobby(lobbyName);
            Vector3 gravity = {0.0, mapDescriptor->gravity, 0.0f};
            using SetGravity = function_ptr_t<void, Vector3&>;
            static SetGravity set_gravity = reinterpret_cast<SetGravity>(il2cpp_functions::resolve_icall("UnityEngine.Physics::set_gravity_Injected"));
            set_gravity(gravity);

            /*
            #warning Added return so we can play cross platform for now
    	    return;
            */
            /// Get All Objects Of Type GameObject
            static std::vector<Il2CppClass*> goKlass = {il2cpp_utils::GetClassFromName("UnityEngine", "GameObject")};
            Array<Il2CppObject*>* allObjects = *il2cpp_utils::RunGenericMethod<Array<Il2CppObject*>*>("UnityEngine", "Resources", "FindObjectsOfTypeAll", goKlass);
            // if not null
            if (allObjects && !isMoved)
            {
                isMoved = true;
                // for each
                for (int j = 0; j < allObjects->Length(); j++)
                {
                
                    Il2CppObject* gameObject = allObjects->values[j];
                    if (!gameObject) continue;
    
                    Il2CppObject* objTransform = *il2cpp_utils::RunMethod(gameObject, "get_transform");
                    Il2CppObject* objParent = *il2cpp_utils::RunMethod(objTransform, "get_parent");
                    
                    // check if parent null
                    if (!objParent)
                    {
                        // if parent null it's a root, subtract 5000 of the position
                        Vector3 pos = *il2cpp_utils::RunMethod<Vector3>(objTransform, "get_position");
                        pos.y -= 5000.0f;
                        il2cpp_utils::RunMethod(objTransform, "set_position", pos);
                    }
                }
            }
        }
        
    }

    void Loader::ForceRespawn()
    {
        static std::vector<Il2CppClass*> teleporterKlass = {classof(Teleporter*)};
        Teleporter* teleporter = *il2cpp_utils::RunGenericMethod<Teleporter*>(globalData->bigTreeTeleportToMap, "GetComponent", teleporterKlass);

        static Il2CppString* spawnPointContainerName = il2cpp_utils::createcsstr("SpawnPointContainer", il2cpp_utils::StringType::Manual);
        Il2CppObject* spawnPointContainer = *il2cpp_utils::RunMethod("UnityEngine", "GameObject", "Find", spawnPointContainerName);
        Il2CppObject* containerTransform = *il2cpp_utils::RunMethod(spawnPointContainer, "get_transform");

        int childCount = *il2cpp_utils::RunMethod<int>(containerTransform, "get_childCount");
        List<Il2CppObject*>* teleportPoints = teleporter->teleportPoints;
        
        for (int i = 0; i < childCount; i++)
        {
            Il2CppObject* child = *il2cpp_utils::RunMethod(containerTransform, "GetChild", i);
            il2cpp_utils::RunMethod(teleportPoints, "Add", child);
        }

        int index = teleportPoints->size > 1 ? rand() % teleportPoints->size : 0;
        Il2CppObject* dest = teleportPoints->items->values[index];

        Player::TeleportPlayer(dest);
    }

    void Loader::ResetMapProperties()
    {
        Vector3 gravity = {0.0, -9.8f, 0.0f};
        using SetGravity = function_ptr_t<void, Vector3&>;
        static SetGravity set_gravity = reinterpret_cast<SetGravity>(il2cpp_functions::resolve_icall("UnityEngine.Physics::set_gravity_Injected"));
        set_gravity(gravity);
        /*
        #warning Added return so we can play cross platform for now
        return;
        */
        /// Get All Objects Of Type GameObject
        static std::vector<Il2CppClass*> goKlass = {il2cpp_utils::GetClassFromName("UnityEngine", "GameObject")};
        Array<Il2CppObject*>* allObjects = *il2cpp_utils::RunGenericMethod<Array<Il2CppObject*>*>("UnityEngine", "Resources", "FindObjectsOfTypeAll", goKlass);
        // if not null
        if (allObjects && isMoved)
        {
            isMoved = false;
            // for each
            for (int j = 0; j < allObjects->Length(); j++)
            {

                Il2CppObject* gameObject = allObjects->values[j];
                if (!gameObject) continue;

                Il2CppObject* objTransform = *il2cpp_utils::RunMethod(gameObject, "get_transform");
                Il2CppObject* objParent = *il2cpp_utils::RunMethod(objTransform, "get_parent");
                
                // check if parent null
                if (!objParent)
                {
                    // if parent null it's a root, add 5000 to the position
                    Vector3 pos = *il2cpp_utils::RunMethod<Vector3>(objTransform, "get_position");
                    pos.y += 5000.0f;
                    il2cpp_utils::RunMethod(objTransform, "set_position", pos);
                }
            }
        }
    }

    void Loader::LoadMap(MapInfo info)
    {
        if (isLoading) return;
        if (mapLoadData.info.filePath == info.filePath) return;
        isLoading = true;
        UnloadMap();

        lobbyName = "";

        mapLoadData.info = info;
        mapLoadData.loadState = LoadState::LoadingBundle;
        mapLoadData.moveNext = true;
    }

    void Loader::LoadBundle()
    {
        int err = 0;
        zip *z = zip_open(mapLoadData.info.filePath.c_str(), 0, &err);
        
        struct zip_stat st;
        zip_stat_init(&st);
        zip_stat(z, mapLoadData.info.packageInfo->androidFileName.c_str(), 0, &st);

        zip_file* f = zip_fopen(z, st.name, 0);
        char* bundle = new char[st.size];
        zip_fread(f, bundle, st.size);
        zip_fclose(f);
        zip_close(z);
        std::vector<uint8_t> byteVector;
        for (int i = 0; i < st.size; i++)
        {
            byteVector.push_back((uint8_t)bundle[i]);
        }
        delete[](bundle);
        Array<uint8_t>* byteArray = il2cpp_utils::vectorToArray(byteVector);

        using LoadFromMemory = function_ptr_t<Il2CppObject*, Array<uint8_t>*, unsigned int>;
        static LoadFromMemory loadFromMemory = reinterpret_cast<LoadFromMemory>(il2cpp_functions::resolve_icall("UnityEngine.AssetBundle::LoadFromMemory_Internal"));
    
        mapLoadData.bundle = loadFromMemory(byteArray, 0);
        mapLoadData.loadState = LoadState::LoadingScene;
        mapLoadData.moveNext = true;
    }

    void Loader::LoadScene()
    {
        Array<Il2CppString*>* scenePaths = *il2cpp_utils::RunMethod<Array<Il2CppString*>*>(mapLoadData.bundle, "GetAllScenePaths");
        if (!scenePaths) return;
        Il2CppString* scenePath = scenePaths->values[0];

        LoadSceneParameters params = {LoadSceneMode::Additive, LocalPhysicsMode::Physics3D};
        /*
        using LoadScene = function_ptr_t<Scene, Il2CppString*, LoadSceneParameters>;
        static LoadScene loadScene = *il2cpp_functions::resolve_icall("UnityEngine.SceneManagement.SceneManager::LoadScene_Internal");
        
        //mapLoadData.scene = *il2cpp_utils::RunMethod<Scene>("UnityEngine.SceneManagement", "SceneManager", "LoadScene", scenePath, params);
        mapLoadData.scene = loadScene(scenePath, params);
        */

        auto* sceneAsync = *il2cpp_utils::RunMethod("UnityEngine.SceneManagement", "SceneManager", "LoadSceneAsync", scenePath, params);

        auto method = il2cpp_utils::FindMethodUnsafe(sceneAsync, "add_completed", 1);
        std::function<void(Scene)> fun = [&](Scene scene){
            mapLoadData.scene = scene;

            mapLoadData.loadState = LoadState::InitializingMap;
            mapLoadData.moveNext = true;
        };

        auto action = il2cpp_utils::MakeDelegate(method, 0, new std::function<void(Scene)>(fun), SceneComplete);

        il2cpp_utils::RunMethod(sceneAsync, method, action);

    }

    void Loader::SceneComplete(void* callback, Il2CppObject* sceneLoadRequest)
    {
        Scene scene = *il2cpp_utils::RunMethod<Scene>(sceneLoadRequest, "get_scene");

        il2cpp_utils::RunMethod(sceneLoadRequest, "Finalize");

        ((std::function<void(Scene)>*)callback)->operator()(scene);
    }

    void Loader::InitializeMap()
    {
        static std::vector<Il2CppClass*> goKlass = {il2cpp_utils::GetClassFromName("UnityEngine", "GameObject")};
        Array<Il2CppObject*>* allObjects = *il2cpp_utils::RunGenericMethod<Array<Il2CppObject*>*>("UnityEngine", "Resources", "FindObjectsOfTypeAll", goKlass);
        
        Il2CppObject* mapInstance = *il2cpp_utils::New(goKlass[0]);
        il2cpp_utils::RunMethod("UnityEngine", "Object", "DontDestroyOnLoad", mapInstance);
        Il2CppObject* mapTransform = *il2cpp_utils::RunMethod(mapInstance, "get_transform");

        if (allObjects)
        {
            for (int j = 0; j < allObjects->Length(); j++)
            {
                Il2CppObject* gameObject = allObjects->values[j];
                if (!gameObject) continue;
                Scene theScene = *il2cpp_utils::RunMethod<Scene>(gameObject, "get_scene");
                std::string objScene = "";
                getSceneName(theScene, objScene);

                if (objScene != "GorillaTagNewVisuals" && objScene != "DontDestroyOnLoad" && objScene != "")
                {   
                    Il2CppString* objName = *il2cpp_utils::RunMethod<Il2CppString*>(gameObject, "get_name");
                    std::string cppName = to_utf8(csstrtostr(objName));
                    getLogger().info("obj '%s' belongs to scene '%s'", cppName.c_str(), objScene.c_str());
                    Il2CppObject* objTransform = *il2cpp_utils::RunMethod(gameObject, "get_transform");
                    Il2CppObject* objParent = *il2cpp_utils::RunMethod(objTransform, "get_parent");
                    if (!objParent)
                    {
                        il2cpp_utils::RunMethod(objTransform, "SetParent", mapTransform);
                    }
                }
            }
        }
        ProcessMap(mapInstance);
        ColorTreeTeleporter(green);
        mapDescriptor->gravity = mapLoadData.info.packageInfo->config.gravity;
        
        il2cpp_utils::RunMethod(mapLoadData.bundle, "Unload", false);
        mapLoadData.bundle = nullptr;
        isLoading = false;

        lobbyName = mapLoadData.info.packageInfo->descriptor.author + "_" + mapLoadData.info.packageInfo->descriptor.mapName;
        
        if (mapLoadData.info.packageInfo->config.guid != "")
        {
            lobbyName = string_format("%s_%d", mapLoadData.info.packageInfo->config.guid.c_str(), mapLoadData.info.packageInfo->config.version);
        }
    }

    void Loader::ProcessMap(Il2CppObject* map)
    {
        mapInstance = map;
        Il2CppObject* mapTransform = *il2cpp_utils::RunMethod(map, "get_transform");

        static std::vector<Il2CppClass*> descriptorKlass = {classof(MapDescriptor*)};
        mapDescriptor = *il2cpp_utils::RunGenericMethod<MapDescriptor*>(mapInstance, "AddComponent", descriptorKlass);
        // initializes all the components to be added (due to quest bundles not getting MBs)
        mapDescriptor->Initialize();
     
        ProcessChildren(mapTransform);

        static Il2CppString* fakeSkyBoxString = il2cpp_utils::createcsstr("FakeSkyBox", il2cpp_utils::StringType::Manual);
        int childCount = *il2cpp_utils::RunMethod<int>(mapTransform, "get_childCount");
        Il2CppObject* fakeSkyBox = nullptr;
        for (int i = 0; i < childCount; i++)
        {
            if (fakeSkyBox) break;
            Il2CppObject* child = *il2cpp_utils::RunMethod(mapTransform, "GetChild", i);
            fakeSkyBox = *il2cpp_utils::RunMethod(child, "Find", fakeSkyBoxString);
        }

        // skybox setup
        if (fakeSkyBox)
        {
            static std::vector<Il2CppClass*> rendererKlass = {il2cpp_utils::GetClassFromName("UnityEngine", "Renderer")};
            Il2CppObject* fakeSkyBoxGO = *il2cpp_utils::RunMethod(fakeSkyBox, "get_gameObject");
            Il2CppObject* renderer = *il2cpp_utils::RunGenericMethod(fakeSkyBoxGO, "GetComponent", rendererKlass);
            Il2CppObject* oldMat = *il2cpp_utils::RunMethod(renderer, "get_material");
            static Il2CppString* _Tex = il2cpp_utils::createcsstr("_Tex", il2cpp_utils::StringType::Manual);
            
            bool hasTex = *il2cpp_utils::RunMethod<bool>(oldMat, "HasProperty", _Tex);

            if (hasTex)
            {
                static std::vector<Il2CppClass*> textureKlass = {il2cpp_utils::GetClassFromName("UnityEngine", "Texture2D")};
                static Il2CppString* texturePath = il2cpp_utils::createcsstr("objects/forest/materials/sky", il2cpp_utils::StringType::Manual);
                Il2CppObject* texture = *il2cpp_utils::RunGenericMethod("UnityEngine", "Resources", "Load", textureKlass, texturePath);

                il2cpp_utils::RunMethod(oldMat, "SetTexture", _Tex, texture);
                il2cpp_utils::RunMethod(oldMat, "set_color", (Color){1.0f, 1.0f, 1.0f, 1.0f});
            }
        }
        static Il2CppClass* goKlass = il2cpp_utils::GetClassFromName("UnityEngine", "GameObject");
        static Il2CppString* fallEmergencyTeleportName = il2cpp_utils::createcsstr("FallEmergencyTeleport", il2cpp_utils::StringType::Manual);

        globalData->fallEmergencyTeleport = *il2cpp_utils::New(goKlass);
        il2cpp_utils::RunMethod(globalData->fallEmergencyTeleport, "set_name", fallEmergencyTeleportName);
        
        il2cpp_utils::RunMethod(globalData->fallEmergencyTeleport, "set_layer", MASKLAYER_HANDTRIGGER);
        static std::vector<Il2CppClass*> boxColliderKlass = {il2cpp_utils::GetClassFromName("UnityEngine", "BoxCollider")};
        Il2CppObject* boxCollider = *il2cpp_utils::RunGenericMethod(globalData->fallEmergencyTeleport, "AddComponent", boxColliderKlass);
        il2cpp_utils::RunMethod(boxCollider, "set_isTrigger", true);

        Il2CppObject* emergencyTeleportTransform = *il2cpp_utils::RunMethod(globalData->fallEmergencyTeleport, "get_transform");
        il2cpp_utils::RunMethod(emergencyTeleportTransform, "SetParent", mapTransform);
        Vector3 localScale = {2000.0f, 1.0f, 2000.0f};
        Vector3 localPosition = {0.0f, 4700.0f, 0.0f};

        il2cpp_utils::RunMethod(emergencyTeleportTransform, "set_localScale", localScale);
        il2cpp_utils::RunMethod(emergencyTeleportTransform, "set_localPosition", localPosition);

        static std::vector<Il2CppClass*> teleporterKlass = {classof(Teleporter*)};
        Teleporter* emergencyTeleporter = *il2cpp_utils::RunGenericMethod<Teleporter*>(globalData->fallEmergencyTeleport, "AddComponent", teleporterKlass);

        static Il2CppString* spawnPointContainerName = il2cpp_utils::createcsstr("SpawnPointContainer", il2cpp_utils::StringType::Manual);
        Il2CppObject* spawnPointContainer = *il2cpp_utils::RunMethod("UnityEngine", "GameObject", "Find", spawnPointContainerName);
        Il2CppObject* containerTransform = *il2cpp_utils::RunMethod(spawnPointContainer, "get_transform");

        childCount = *il2cpp_utils::RunMethod<int>(containerTransform, "get_childCount");
        il2cpp_utils::RunMethod(emergencyTeleporter->teleportPoints, "Clear");
        
        for (int i = 0; i < childCount; i++)
        {
            Il2CppObject* child = *il2cpp_utils::RunMethod(containerTransform, "GetChild", i);
            il2cpp_utils::RunMethod(emergencyTeleporter->teleportPoints, "Add", child);
        }

        emergencyTeleporter->tagOnTeleport = true;
        emergencyTeleporter->teleporterType = TeleporterType::Map;


    }

    void Loader::ProcessChildren(Il2CppObject* parent)
    {
        if (!parent) return;

        int childCount = *il2cpp_utils::RunMethod<int>(parent, "get_childCount");
        for (int i = 0; i < childCount; i++)
        {
            Il2CppObject* child = *il2cpp_utils::RunMethod(parent, "GetChild", i);
            Il2CppObject* go = *il2cpp_utils::RunMethod(child, "get_gameObject");
            SetupCollisions(go);

            int childChildCount = *il2cpp_utils::RunMethod<int>(child, "get_childCount");

            if (childChildCount > 0)
            {
                ProcessChildren(child);
            }
        }

        // skipping the teleporter point count > 0 here because that's already in MapDescriptor::Initialize()
    }

    void Loader::SetupCollisions(Il2CppObject* child)
    {
        if (!child) return;
        static std::vector<Il2CppClass*> colliderKlass = {il2cpp_utils::GetClassFromName("UnityEngine", "Collider")};
        Array<Il2CppObject*>* colliders = *il2cpp_utils::RunGenericMethod<Array<Il2CppObject*>*>(child, "GetComponents", colliderKlass);
        if (!colliders) return;

        for (int i = 0; i < colliders->Length(); i++)
        {
            Il2CppObject* collider = colliders->values[i];
            if (!collider) return;

            bool isTrigger = *il2cpp_utils::RunMethod<bool>(collider, "get_isTrigger");
            if (isTrigger)
            {
                il2cpp_utils::RunMethod(child, "set_layer", MASKLAYER_GORILLATRIGGER);
                continue;
            }
            else if (*il2cpp_utils::RunMethod<int>(child, "get_layer") == 0)
            {
                il2cpp_utils::RunMethod(child, "set_layer", 9);
            }

            static std::vector<Il2CppClass*> teleporterKlass = {classof(Teleporter*)};
            static std::vector<Il2CppClass*> tagZoneKlass = {classof(TagZone*)};
            static std::vector<Il2CppClass*> triggerKlass = {classof(ObjectTrigger*)};

            Teleporter* teleporter = *il2cpp_utils::RunGenericMethod<Teleporter*>(child, "GetComponent", teleporterKlass);
            TagZone* tagZone = *il2cpp_utils::RunGenericMethod<TagZone*>(child, "GetComponent", tagZoneKlass);
            ObjectTrigger* objectTrigger = *il2cpp_utils::RunGenericMethod<ObjectTrigger*>(child, "GetComponent", triggerKlass);

            if (teleporter || tagZone || objectTrigger)
            {
                il2cpp_utils::RunMethod(collider, "set_isTrigger", true);
                il2cpp_utils::RunMethod(child, "set_layer", MASKLAYER_HANDTRIGGER);
            }
        }
    }
}