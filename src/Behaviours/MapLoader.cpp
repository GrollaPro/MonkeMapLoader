#include "Behaviours/MapLoader.hpp"
#include "zip/shared/zip.h"
#include "static-defines.hpp"
#include "Behaviours/TagZone.hpp"
#include "Behaviours/Teleporter.hpp"
#include "Behaviours/Player.hpp"
#include "Behaviours/ObjectTrigger.hpp"

#include "Behaviours/PreviewOrb.hpp"
#include "Behaviours/RotateByHand.hpp"

#include "Utils/RoomUtils.hpp"
#include "quest-cosmetic-loader/shared/CosmeticLoader.hpp"

#include "UnityEngine/SceneManagement/SceneManager.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/BoxCollider.hpp"
#include "UnityEngine/Collider.hpp"
#include "UnityEngine/Renderer.hpp"
#include "UnityEngine/MeshRenderer.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/SceneManagement/Scene.hpp"
#include "UnityEngine/SceneManagement/LoadSceneMode.hpp"
#include "UnityEngine/SceneManagement/LoadSceneParameters.hpp"
#include "UnityEngine/SceneManagement/LocalPhysicsMode.hpp"
#include "Utils/LightingUtils.hpp"

DEFINE_TYPE(MapLoader::Loader);

extern Logger& getLogger();


using namespace UnityEngine;
using namespace UnityEngine::SceneManagement;

extern bool getSceneName(Scene scene, std::string& out);

namespace MapLoader
{
    GameObject* Loader::mapInstance = nullptr;
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
            case LoadState::LoadingData:
                LoadData();
                break;
            case LoadState::LoadingBundle:
                LoadBundle();
                break;
            case LoadState::LoadingScene:
                LoadScene();
                break;
            case LoadState::InitializingMap:
                InitializeMap();
                break;
            case LoadState::FixLighting:
                FixLighting();
            default:
                break;
        }
    }

    void Loader::UnloadMap()
    {
        if (mapInstance)
        {
            Object::Destroy(mapInstance);
            
            mapInstance = nullptr;
            
            ColorTreeTeleporter(Color::get_red());

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
        GameObject* slide = GameObject::Find(slideName);
        Transform* slideTransform = slide->get_transform(); 
        Vector3 origin = slideTransform->get_position();
        globalData->set_origin(origin);

        static Il2CppString* stoolName = il2cpp_utils::createcsstr("stool", il2cpp_utils::StringType::Manual);
        GameObject* stool = GameObject::Find(stoolName);
        Transform* stoolTransform = stool->get_transform(); 
        Vector3 stoolPos = stoolTransform->get_position();

        globalData->treeOrigin = stoolPos;

        using namespace CosmeticsLoader;
        // create map teleporter
        if (!globalData->bigTreeTeleportToMap)
        {
            std::string treeTeleporterPath = "/sdcard/ModData/com.AnotherAxiom.GorillaTag/Mods/MonkeMapLoader/Teleporter";
            auto* loader = new CosmeticLoader(treeTeleporterPath, [&](std::string name, Il2CppObject* teleporter){
                globalData->bigTreeTeleportToMap = (GameObject*)teleporter;
                Vector3 pos = globalData->bigTreeTeleportToMap->get_transform()->get_position();
                pos.z += 2.9f;
                pos.y -= 0.05f;
                globalData->bigTreeTeleportToMap->get_transform()->set_position(pos);

                Vector3 rotate(0.0f, 20.0f, 0.0f);

                globalData->bigTreeTeleportToMap->get_transform()->Rotate(rotate);
            }, "_Teleporter", il2cpp_utils::GetSystemType("UnityEngine", "GameObject"));
            globalData->bigTreeTeleportToMap->set_layer(MASKLAYER_PLAYERTRIGGER);
            Object::DontDestroyOnLoad(globalData->bigTreeTeleportToMap);
        }
        
        std::string orbPath = "/sdcard/ModData/com.AnotherAxiom.GorillaTag/Mods/MonkeMapLoader/Orb";
        GameObject* orb = nullptr;
        auto* loader = new CosmeticLoader(orbPath, [&](std::string name, Il2CppObject* theOrb){
                orb = (GameObject*)theOrb;
        }, "_Orb", il2cpp_utils::GetSystemType("UnityEngine", "GameObject"));

        GameObject* orbVisuals = Object::Instantiate(orb);

        RotateByHand* rotateByHand = orb->AddComponent<RotateByHand*>();

        Renderer* renderer = orb->GetComponent<Renderer*>();
        renderer->set_enabled(false);
        Object::Destroy(renderer);
        orb->set_layer(18);

        Transform* orbTransform = orb->get_transform();
        Transform* visualsTransform = orbVisuals->get_transform();
            
        visualsTransform->SetParent(orbTransform);
            
        visualsTransform->set_localPosition(Vector3::get_zero());
        visualsTransform->set_localScale(Vector3::get_one());
        visualsTransform->set_localRotation(Quaternion::get_identity());

        PreviewOrb* previewOrb = orbVisuals->AddComponent<PreviewOrb*>();

        Collider* collider = orbVisuals->GetComponent<Collider*>();
        collider->set_enabled(false);
        Object::Destroy(collider);

        static std::vector<Il2CppClass*> teleporterKlass = {classof(Teleporter*)};
        Teleporter* treeTeleporter = globalData->bigTreeTeleportToMap->AddComponent<Teleporter*>();
        treeTeleporter->teleporterType = TeleporterType::Map;
        treeTeleporter->joinGameOnTeleport = true;
        treeTeleporter->Delay = 1.5f;
        treeTeleporter->touchType = TouchType::Head;

        Object::DontDestroyOnLoad(treeTeleporter);
        Transform* treePointTransform = nullptr;
        static Il2CppClass* goKlass = il2cpp_utils::GetClassFromName("UnityEngine", "GameObject");
        if (!globalData->bigTreePoint)
        {
            Il2CppString* treeHomeTargetObjectName = il2cpp_utils::createcsstr("TreeHomeTargetObject");
            globalData->bigTreePoint = GameObject::New_ctor();

            globalData->bigTreePoint->set_name(treeHomeTargetObjectName);
            treePointTransform = globalData->bigTreePoint->get_transform();

            Vector3 pos = {-66.0f, 12.3f, -83.0f};
            Quaternion rot = Quaternion::Euler(0.0f, -90.0f, 0.0f);

            treePointTransform->set_position(pos);
            treePointTransform->set_rotation(rot);

            Object::DontDestroyOnLoad(globalData->bigTreePoint);
        }
        else
        {
            treePointTransform = globalData->bigTreePoint->get_transform();
        }

        treeTeleporter->teleportPoints->Add(treePointTransform);

        ColorTreeTeleporter(Color::get_red());

        static Il2CppString* fallEmergencyTeleportName = il2cpp_utils::createcsstr("FallEmergencyTeleport", il2cpp_utils::StringType::Manual);

        globalData->fallEmergencyTeleport = GameObject::New_ctor();
        globalData->fallEmergencyTeleport->set_name(fallEmergencyTeleportName);

        globalData->fallEmergencyTeleport->set_layer(MASKLAYER_HANDTRIGGER);
        
        BoxCollider* boxCollider = globalData->fallEmergencyTeleport->AddComponent<BoxCollider*>();
        boxCollider->set_isTrigger(true);

        Transform* emergencyTeleportTransform = globalData->fallEmergencyTeleport->get_transform();
        Vector3 localScale = {1000.0f, 1.0f, 1000.0f};
        Vector3 position = {0.0f, -200.0f, 0.0f};
        Vector3 treeOrigin = globalData->treeOrigin;

        position.x += treeOrigin.x;
        position.y += treeOrigin.y;
        position.z += treeOrigin.z;

        emergencyTeleportTransform->set_localScale(localScale);
        emergencyTeleportTransform->set_position(position);

        Teleporter* emergencyTeleporter = globalData->fallEmergencyTeleport->AddComponent<Teleporter*>();
        emergencyTeleporter->teleportPoints->Add(treePointTransform);
        emergencyTeleporter->teleporterType = TeleporterType::Treehouse;
    }

    void Loader::ColorTreeTeleporter(Color color)
    {
        Array<Renderer*>* renderers = globalData->bigTreeTeleportToMap->GetComponentsInChildren<Renderer*>();

        // for each renderer
        for (int i = 0; i < renderers->Length(); i++)
        {
            Renderer* renderer = renderers->values[i];
            Array<Material*>* materials = renderer->get_materials();

            // for each material
            for (int j = 0; j < materials->Length(); j++)
            {
                Material* material = materials->values[j];
                Il2CppString* csName = material->get_name();
                std::string name = to_utf8(csstrtostr(csName));
                
                // Only the materials with name center should change
                if (name.find("Center") != std::string::npos)
                    material->set_color(color);
            }
        }
    }

    void Loader::JoinGame()
    {
        // if lobbyName defined
        if (lobbyName != "")
        {
            RoomUtils::JoinModdedLobby(lobbyName);
            Vector3 gravity = {0.0, mapDescriptor->gravity, 0.0f};
            using SetGravity = function_ptr_t<void, Vector3&>;
            static SetGravity set_gravity = reinterpret_cast<SetGravity>(il2cpp_functions::resolve_icall("UnityEngine.Physics::set_gravity_Injected"));
            set_gravity(gravity);

            /// Get All Objects Of Type GameObject
            Array<GameObject*>* allObjects = Resources::FindObjectsOfTypeAll<GameObject*>();
            // if not null
            if (allObjects && !isMoved)
            {
                isMoved = true;
                // for each
                for (int j = 0; j < allObjects->Length(); j++)
                {
                    GameObject* gameObject = allObjects->values[j];
                    if (!gameObject) continue;
    
                    Transform* objTransform = gameObject->get_transform();
                    Transform* objParent = objTransform->get_parent();
                    
                    // check if parent null
                    if (!objParent)
                    {
                        // if parent null it's a root, subtract 5000 of the position
                        Vector3 pos = objTransform->get_position();
                        pos.y -= 5000.0f;
                        objTransform->set_position(pos);
                    }
                }
            }
        }
        
    }

    void Loader::ForceRespawn()
    {
        Teleporter* teleporter = globalData->bigTreeTeleportToMap->GetComponent<Teleporter*>();
        
        static Il2CppString* spawnPointContainerName = il2cpp_utils::createcsstr("SpawnPointContainer", il2cpp_utils::StringType::Manual);
        GameObject* spawnPointContainer = GameObject::Find(spawnPointContainerName);
        Transform* containerTransform = spawnPointContainer->get_transform();

        int childCount = containerTransform->get_childCount();
        List<Transform*>* teleportPoints = teleporter->teleportPoints;
        
        for (int i = 0; i < childCount; i++)
        {
            Transform* child = containerTransform->GetChild(i);
            teleportPoints->Add(child);
        }

        int index = teleportPoints->size > 1 ? rand() % teleportPoints->size : 0;
        Transform* dest = teleportPoints->get_Item(index);

        Player::TeleportPlayer(dest);
    }

    void Loader::ResetMapProperties()
    {
        Vector3 gravity = {0.0, -9.8f, 0.0f};
        using SetGravity = function_ptr_t<void, Vector3&>;
        static SetGravity set_gravity = reinterpret_cast<SetGravity>(il2cpp_functions::resolve_icall("UnityEngine.Physics::set_gravity_Injected"));
        set_gravity(gravity);
        /// Get All Objects Of Type GameObject

        /// Get All Objects Of Type GameObject
        Array<GameObject*>* allObjects = Resources::FindObjectsOfTypeAll<GameObject*>();
        // if not null
        if (allObjects && isMoved)
        {
            isMoved = false;
            // for each
            for (int j = 0; j < allObjects->Length(); j++)
            {
                GameObject* gameObject = allObjects->values[j];
                if (!gameObject) continue;

                Transform* objTransform = gameObject->get_transform();
                Transform* objParent = objTransform->get_parent();
                
                // check if parent null
                if (!objParent)
                {
                    // if parent null it's a root, subtract 5000 of the position
                    Vector3 pos = objTransform->get_position();
                    pos.y += 5000.0f;
                    objTransform->set_position(pos);
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
        mapLoadData.loadState = (LoadState)0;
        mapLoadData.moveNext = true;
    }

    void Loader::LoadData()
    {
        std::thread bundleLoad([&]{
            int err = 0;
            zip *z = zip_open(mapLoadData.info.filePath.c_str(), 0, &err);

            struct zip_stat st;
            zip_stat_init(&st);
            zip_stat(z, mapLoadData.info.packageInfo->androidFileName.c_str(), 0, &st);

            zip_file* f = zip_fopen(z, st.name, 0);
            uint8_t* bundle = new uint8_t[st.size];
            zip_fread(f, (char*)bundle, st.size);
            zip_fclose(f);
            zip_close(z);
            this->mapLoadData.data.clear();
            
            this->mapLoadData.data = std::vector<uint8_t>(bundle, bundle + st.size);

            delete[](bundle);

            this->mapLoadData.loadState = LoadState::LoadingBundle;
            this->mapLoadData.moveNext = true;
        });

        bundleLoad.detach();
    }

    void Loader::LoadBundle()
    {
        Array<uint8_t>* byteArray = il2cpp_utils::vectorToArray(mapLoadData.data);

        using LoadFromMemory = function_ptr_t<AssetBundle*, Array<uint8_t>*, unsigned int>;
        static LoadFromMemory loadFromMemory = reinterpret_cast<LoadFromMemory>(il2cpp_functions::resolve_icall("UnityEngine.AssetBundle::LoadFromMemory_Internal"));

        mapLoadData.bundle = loadFromMemory(byteArray, 0);
        mapLoadData.loadState = LoadState::LoadingScene;
        mapLoadData.moveNext = true;
    }

    void Loader::LoadScene()
    {
        Array<Il2CppString*>* scenePaths = mapLoadData.bundle->GetAllScenePaths();
        if (!scenePaths) return;
        Il2CppString* scenePath = scenePaths->values[0];

        LoadSceneParameters params = {LoadSceneMode::Additive, LocalPhysicsMode::Physics3D};

        auto* sceneAsync = SceneManager::LoadSceneAsync(scenePath, params);

        std::function<void(void)> fun = [&]{
            mapLoadData.loadState = LoadState::InitializingMap;
            mapLoadData.moveNext = true;
        };

        auto method = il2cpp_utils::FindMethodUnsafe(sceneAsync, "add_completed", 1);
        auto action = il2cpp_utils::MakeDelegate(method, 0, new std::function<void(void)>(fun), SceneComplete);

        il2cpp_utils::RunMethod(sceneAsync, method, action);

    }

    void Loader::SceneComplete(void* callback, AsyncOperation* sceneLoadRequest)
    {
        ((std::function<void(void)>*)callback)->operator()();
        delete(((std::function<void(void)>*)callback));
        sceneLoadRequest->Finalize();
    }

    void Loader::InitializeMap()
    {
        Array<GameObject*>* allObjects = Resources::FindObjectsOfTypeAll<GameObject*>();

        GameObject* mapInstance = GameObject::New_ctor();
        Object::DontDestroyOnLoad(mapInstance);
        Transform* mapTransform = mapInstance->get_transform();

        if (allObjects)
        {
            // for each object
            for (int j = 0; j < allObjects->Length(); j++)
            {
                GameObject* gameObject = allObjects->values[j];
                if (!gameObject) continue;
                Scene theScene = gameObject->get_scene();

                // get scene name
                std::string objScene = "";
                getSceneName(theScene, objScene);

                // if its not any of the scenes that already exist
                if (objScene != "GorillaTagNewVisuals" && objScene != "DontDestroyOnLoad" && objScene != "")
                {   
                    Transform* objTransform = gameObject->get_transform();
                    Transform* objParent = objTransform->get_parent();
                    if (!objParent)
                    {
                        objTransform->SetParent(mapTransform);
                    }
                }
            }
        }

        ProcessMap(mapInstance);

        ColorTreeTeleporter(Color::get_green());
        mapDescriptor->gravity = mapLoadData.info.packageInfo->config.gravity;
        
        mapLoadData.bundle->Unload(false);
        mapLoadData.bundle = nullptr;
        isLoading = false;
        
        // long ass string concat to make the lobby name, if guid exists use that, else use map author and map name
        if (mapLoadData.info.packageInfo->config.guid != "")
        {
            lobbyName = string_format("%s_%d", mapLoadData.info.packageInfo->config.guid.c_str(), mapLoadData.info.packageInfo->config.version);
        }
        else 
        {
            lobbyName = mapLoadData.info.packageInfo->descriptor.author + "_" + mapLoadData.info.packageInfo->descriptor.mapName;
        }

        mapLoadData.loadState = LoadState::FixLighting;
        mapLoadData.moveNext = true;
    }

    void Loader::ProcessMap(GameObject* map)
    {
        mapInstance = map;
        Transform* mapTransform = map->get_transform();

        mapDescriptor = mapInstance->AddComponent<MapDescriptor*>();
        // initializes all the components to be added (due to quest bundles not getting MBs)
        mapDescriptor->Initialize();
     
        ProcessChildren(mapTransform);

        static Il2CppString* fakeSkyBoxString = il2cpp_utils::createcsstr("FakeSkyBox", il2cpp_utils::StringType::Manual);
        int childCount = mapTransform->get_childCount();
        Transform* fakeSkyBox = nullptr;
        for (int i = 0; i < childCount; i++)
        {
            if (fakeSkyBox) break;
            Transform* child = mapTransform->GetChild(i);
            fakeSkyBox = child->Find(fakeSkyBoxString);
        }

        // skybox setup
        if (fakeSkyBox)
        {
            GameObject* fakeSkyBoxGO = fakeSkyBox->get_gameObject();
            Renderer* renderer = fakeSkyBoxGO->GetComponent<Renderer*>();
            Material* oldMat = renderer->get_material();
            static Il2CppString* _Tex = il2cpp_utils::createcsstr("_Tex", il2cpp_utils::StringType::Manual);
            
            bool hasTex = oldMat->HasProperty(_Tex);

            if (hasTex)
            {
                static Il2CppString* texturePath = il2cpp_utils::createcsstr("objects/forest/materials/sky", il2cpp_utils::StringType::Manual);
                Texture2D* texture = Resources::Load<Texture2D*>(texturePath);

                oldMat->SetTexture(_Tex, texture);
                oldMat->set_color(Color::get_white());
            }
        }
        static Il2CppString* fallEmergencyTeleportName = il2cpp_utils::createcsstr("FallEmergencyTeleport", il2cpp_utils::StringType::Manual);

        globalData->fallEmergencyTeleport = GameObject::New_ctor();
        globalData->fallEmergencyTeleport->set_name(fallEmergencyTeleportName);
        
        globalData->fallEmergencyTeleport->set_layer(MASKLAYER_HANDTRIGGER);
        BoxCollider* boxCollider = globalData->fallEmergencyTeleport->AddComponent<BoxCollider*>();
        boxCollider->set_isTrigger(true);

        Transform* emergencyTeleportTransform = globalData->fallEmergencyTeleport->get_transform();
        emergencyTeleportTransform->SetParent(mapTransform);
        Vector3 localScale = {2000.0f, 1.0f, 2000.0f};
        Vector3 localPosition = {0.0f, 4700.0f, 0.0f};

        emergencyTeleportTransform->set_localScale(localScale);
        emergencyTeleportTransform->set_localPosition(localPosition);

        Teleporter* emergencyTeleporter = globalData->fallEmergencyTeleport->AddComponent<Teleporter*>();
        static Il2CppString* spawnPointContainerName = il2cpp_utils::createcsstr("SpawnPointContainer", il2cpp_utils::StringType::Manual);
        GameObject* spawnPointContainer = GameObject::Find(spawnPointContainerName);
        Transform* containerTransform = spawnPointContainer->get_transform();

        childCount = containerTransform->get_childCount();
        emergencyTeleporter->teleportPoints->Clear();
        
        for (int i = 0; i < childCount; i++)
        {
            Transform* child = containerTransform->GetChild(i);
            emergencyTeleporter->teleportPoints->Add(child);
        }

        emergencyTeleporter->tagOnTeleport = true;
        emergencyTeleporter->teleporterType = TeleporterType::Map;
    }

    void Loader::ProcessChildren(Transform* parent)
    {
        if (!parent) return;

        int childCount = parent->get_childCount();
        for (int i = 0; i < childCount; i++)
        {
            Transform* child = parent->GetChild(i);
            GameObject* go = child->get_gameObject();
            SetupCollisions(go);

            if (child->get_childCount() > 0)
            {
                ProcessChildren(child);
            }
        }

        // skipping the teleporter point count > 0 here because that's already in MapDescriptor::Initialize()
    }

    void Loader::SetupCollisions(GameObject* child)
    {
        if (!child) return;
        Array<Collider*>* colliders = child->GetComponents<Collider*>();
        if (!colliders) return;
        int length = colliders->Length();
        for (int i = 0; i < length; i++)
        {
            Collider* collider = colliders->values[i];
            if (!collider) return;

            
            if (collider->get_isTrigger())
            {
                child->set_layer(MASKLAYER_GORILLATRIGGER);
                continue;
            }
            else if (child->get_layer() == 0)
            {
                child->set_layer(9);
            }

            if (child->GetComponent<Teleporter*>() || child->GetComponent<TagZone*>() || child->GetComponent<ObjectTrigger*>())
            {
                collider->set_isTrigger(true);
                child->set_layer(MASKLAYER_HANDTRIGGER);
            }
        }
    }

    void Loader::FixLighting()
    {
        Array<MeshRenderer*>* renderers = mapInstance->GetComponentsInChildren<MeshRenderer*>();

        for (int i = 0; i < renderers->Length(); i++)
        {
            Array<Material*>* materials = renderers->values[i]->get_materials();

            for (int j = 0; j < materials->Length(); j++)
            {
                LightingUtils::SetLightingStrength(materials->values[j], 0.25f);
            }
        }
    }
}