#include "modloader/shared/modloader.hpp"
#include "beatsaber-hook/shared/utils/logging.hpp"
#include "beatsaber-hook/shared/utils/utils.h"
#include "quest-cosmetic-loader/shared/CosmeticLoader.hpp"
#include <thread>

#include "Behaviours/GorillaMapTriggerBase.hpp"
#include "Behaviours/ObjectTrigger.hpp"
#include "Behaviours/PreviewOrb.hpp"
#include "Behaviours/RotateByHand.hpp"
#include "Behaviours/RoundEndActions.hpp"
#include "Behaviours/SurfaceClimbSettings.hpp"
#include "Behaviours/TagZone.hpp"
#include "Behaviours/Teleporter.hpp"
#include "Behaviours/Player.hpp"
#include "Behaviours/MapDescriptor.hpp"
#include "Behaviours/MapLoader.hpp"
#include "Behaviours/RoomList.hpp"
#include "Behaviours/MonkeRoomManager.hpp"
#include "Behaviours/MapNetworkJoinTrigger.hpp"
#include "Behaviours/MovingPlatform.hpp"

#include "UI/MapView.hpp"
#include "UI/MapSelectorView.hpp"
#include "UI/MapSelectorViewManager.hpp"

#include "monkecomputer/shared/Register.hpp"
#include "monkecomputer/shared/GorillaUI.hpp"

#include "Models/MapList.hpp"

#include "custom-types/shared/register.hpp"
#include "Utils/FileUtils.hpp"

#include "beatsaber-hook/shared/utils/il2cpp-utils-methods.hpp"

#include "GorillaLocomotion/Player.hpp"
#include "GorillaLocomotion/Surface.hpp"

#include "GlobalNamespace/VRRig.hpp"
#include "GlobalNamespace/GorillaComputer.hpp"
#include "GlobalNamespace/GorillaTagManager.hpp"
#include "GlobalNamespace/PhotonNetworkController.hpp"
#include "GlobalNamespace/GorillaQuitBox.hpp"

#include "UnityEngine/Animator.hpp"
#include "UnityEngine/Animations/AnimationPlayableOutput.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Quaternion.hpp"
#include "UnityEngine/RaycastHit.hpp"
#include "UnityEngine/Collider.hpp"
#include "UnityEngine/MeshCollider.hpp"
#include "UnityEngine/Mesh.hpp"
#include "UnityEngine/Time.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/SceneManagement/Scene.hpp"

#include "Photon/Pun/PhotonNetwork.hpp"
#include "Photon/Pun/PhotonView.hpp"
#include "Photon/Pun/PhotonAnimatorView.hpp"

#include "Photon/Realtime/LoadBalancingClient.hpp"
#include "Photon/Realtime/Player.hpp"
#include "Photon/Realtime/Room.hpp"
#include "Photon/Realtime/RaiseEventOptions.hpp"
#include "Photon/Realtime/WebFlags.hpp"
#include "Photon/Realtime/ConnectionCallbacksContainer.hpp"

#include "ExitGames/Client/Photon/SendOptions.hpp"
#include "ExitGames/Client/Photon/Hashtable.hpp"

ModInfo modInfo;

Logger& getLogger()
{
    static Logger* logger = new Logger(modInfo, LoggerOptions(false, true));
    return *logger;
}

bool getSceneName(UnityEngine::SceneManagement::Scene scene, std::string& output);

using namespace CosmeticsLoader;
using namespace MapLoader;
using namespace UnityEngine;

MAKE_HOOK_OFFSETLESS(Player_Awake, void, GorillaLocomotion::Player* self)
{
    Player_Awake(self);
    self->get_gameObject()->AddComponent<MapLoader::Player*>();
}

MAKE_HOOK_OFFSETLESS(GorillaComputer_Start, void, GlobalNamespace::GorillaComputer* self)
{
    if (!MonkeRoomManager::get_instance()) il2cpp_utils::New<MonkeRoomManager*>();

    GorillaComputer_Start(self);
    MapList::Load();

    GameObject* loaderGO = *il2cpp_utils::New<GameObject*>();
    loaderGO->AddComponent<Loader*>()->Initialize();

    GameObject* joinGO = *il2cpp_utils::New<GameObject*>();
    MapNetworkJoinTrigger* roomList = joinGO->AddComponent<MapNetworkJoinTrigger*>();
    Object::DontDestroyOnLoad(joinGO);

    Array<GlobalNamespace::GorillaQuitBox*>* quitBoxes = Resources::FindObjectsOfTypeAll<GlobalNamespace::GorillaQuitBox*>();

    for (int i = 0; i < quitBoxes->Length(); i++)
    {
        quitBoxes->values[i]->set_enabled(false);
    }
    
    /*
    GameObject* listGO = *il2cpp_utils::New<GameObject*>();
    RoomList* roomList = listGO->AddComponent<RoomList*>();

    Object::DontDestroyOnLoad(listGO);

    GameObject* countGO = *il2cpp_utils::New<GameObject*>();
    PlayerCountManager* countManager = countGO->AddComponent<PlayerCountManager*>();

    Object::DontDestroyOnLoad(countGO);
    */
}

MAKE_HOOK_OFFSETLESS(Player_GetSlidePercentage, float, GorillaLocomotion::Player* self, RaycastHit raycastHit)
{
    Collider* collider = raycastHit.get_collider();
    GameObject* go = collider->get_gameObject();
    
    SurfaceClimbSettings* settings = go->GetComponent<SurfaceClimbSettings*>();
    
    auto* get_sharedMesh = il2cpp_utils::FindMethodUnsafe("UnityEngine", "MeshCollider", "get_sharedMesh", 0);
    Mesh* sharedMesh = *il2cpp_utils::RunMethod<Mesh*>(collider, get_sharedMesh);

    if (settings)
    {
        GorillaLocomotion::Surface* surface = go->GetComponent<GorillaLocomotion::Surface*>();
        return surface->slipPercentage;
    }

    if (!collider || !sharedMesh)
    {
        return self->defaultSlideFactor;
    }
    
    if (!sharedMesh->get_isReadable())
    {
        return self->defaultSlideFactor;
    }
    return Player_GetSlidePercentage(self, raycastHit);
}

static double lastGameEnd = 0.0;
MAKE_HOOK_OFFSETLESS(VRRig_PlayTagSound, void, GlobalNamespace::VRRig* self, int soundIndex)
{
    using namespace GlobalNamespace;
    VRRig_PlayTagSound(self, soundIndex);

    PhotonNetworkController* photonNetworkController = PhotonNetworkController::_get_instance();
    if (photonNetworkController)
    {
        Il2CppString* currentGameTypeCS = photonNetworkController->currentGameType;
        std::string currentGameType = currentGameTypeCS ? to_utf8(csstrtostr(currentGameTypeCS)) : "";

        if (Loader::lobbyName != "" && currentGameType.find(Loader::lobbyName) != std::string::npos)
        {
            GorillaTagManager* gorillaTagManager = GorillaTagManager::_get_instance();
            
            static Il2CppString* isCurrentlyTagString = il2cpp_utils::createcsstr("isCurrentlyTag", il2cpp_utils::StringType::Manual);
            Il2CppObject* tagptr = *il2cpp_utils::New<Il2CppObject*>();
            bool isCurrentlyTag = false;
            if (gorillaTagManager->currentRoom->get_CustomProperties()->TryGetValue(isCurrentlyTagString, tagptr))
            {
                isCurrentlyTag = *(bool*)&tagptr;
            }
            else 
            {
                getLogger().info("Could not get currently tag bool, assuming false");
            }

            static Il2CppString* timeInfectedGameEndedString = il2cpp_utils::createcsstr("timeInfectedGameEnded", il2cpp_utils::StringType::Manual);
            
            Il2CppObject* timeptr = *il2cpp_utils::New<Il2CppObject*>();
            double timeInfectedGameEnded = 0.0;
            if (gorillaTagManager->currentRoom->get_CustomProperties()->TryGetValue(timeInfectedGameEndedString, timeptr))
            {
                timeInfectedGameEnded = *(double*)&timeptr;
            }
            else
            {
                getLogger().info("Could not get timeInfectedGameEnded double, assuming 0.0");
            }

            if (timeInfectedGameEnded > lastGameEnd)
            {
                lastGameEnd = timeInfectedGameEnded;
                RoundEndActions::RoundEnd();
            }
            else if (isCurrentlyTag) RoundEndActions::RoundEnd();
            else 
            {
                getLogger().info("Not calling game round end because time was not greater and isCurrentlyTag was false");
            }
        }
        else 
        {
            getLogger().error("Not running game end because lobby name was wrong,\nLooked for %s, had %s", Loader::lobbyName.c_str(), currentGameType.c_str());
        }
    }
    else
    {
        getLogger().error("PhotonNetworkController was nullptr");
    }
}

MAKE_HOOK_OFFSETLESS(GorillaTagManager_ReportTag, void, GlobalNamespace::GorillaTagManager* self, Photon::Realtime::Player* taggedPlayer, Photon::Realtime::Player* taggingPlayer)
{
    using namespace Photon::Pun;
    using namespace Photon::Realtime;
    GorillaTagManager_ReportTag(self, taggedPlayer, taggingPlayer);
    
    PhotonView* photonView = self->get_photonView();
    
    bool IsMine = photonView->get_IsMine();
    bool equals = taggedPlayer ? taggedPlayer->Equals(taggingPlayer) : false;
    if (IsMine && equals)
    {
        RaiseEventOptions* raiseEventOptions = RaiseEventOptions::New_ctor();
        WebFlags* flags = WebFlags::New_ctor((uint8_t)1);
        raiseEventOptions->Flags = flags;

        bool isCurrentlyTag = self->isCurrentlyTag;

        Il2CppString* taggingPlayerID = taggingPlayer->get_UserId();
        Il2CppString* taggedPlayerID = taggedPlayer->get_UserId();

        if (isCurrentlyTag)
        {
            Photon::Realtime::Player* currentIt = self->currentIt;
            if (currentIt && !currentIt->Equals(taggedPlayer))
            {
                self->ChangeCurrentIt(taggedPlayer);
                self->lastTag = (double)UnityEngine::Time::get_time();
                
                /*
                Array<Il2CppObject*>* eventContent = reinterpret_cast<Array<Il2CppObject*>*>(il2cpp_functions::array_new(classof(Il2CppObject*), 2));

                eventContent->values[0] = (Il2CppObject*)taggingPlayerID;
                eventContent->values[1] = (Il2CppObject*)taggedPlayerID;
                ExitGames::Client::Photon::SendOptions options = ExitGames::Client::Photon::SendOptions::_get_SendReliable();
                PhotonNetwork::RaiseEvent(1, (Il2CppObject*)eventContent, raiseEventOptions, options);
                */
            }
            else getLogger().info("Player Was already it!");
        }
        else if (Time::get_time() > (self->lastTag + (double)self->tagCoolDown))
        {
            bool contains = self->currentInfected->Contains(taggedPlayer);
            
            if (!contains)
            {
                self->AddInfectedPlayer(taggedPlayer);
    	        /*
                Array<Il2CppObject*>* eventContent = reinterpret_cast<Array<Il2CppObject*>*>(il2cpp_functions::array_new(classof(Il2CppObject*), 3));
                eventContent->values[0] = (Il2CppObject*)taggingPlayerID;
                eventContent->values[1] = (Il2CppObject*)taggedPlayerID;

                long count = self->currentInfected->get_Count();
                eventContent->values[2] = (Il2CppObject*)count;

                ExitGames::Client::Photon::SendOptions options = ExitGames::Client::Photon::SendOptions::_get_SendReliable();
                PhotonNetwork::RaiseEvent(2, (Il2CppObject*)eventContent, raiseEventOptions, options);
                */
            }
            else getLogger().info("Player Was already infected!");
        }
    }
    else
    {
        getLogger().error("IsMine: %d, TaggedPlayer: %p, TaggingPlayer: %p", IsMine, taggedPlayer, taggingPlayer);
    }
}

MAKE_HOOK_OFFSETLESS(MatchMakingCallbacksContainer_OnJoinedRoom, void, Photon::Realtime::MatchMakingCallbacksContainer* self)
{
    MatchMakingCallbacksContainer_OnJoinedRoom(self);
    getLogger().info("OnJoinedRoom Callback");
}

// forced region patch
std::string patchForcedRegion = "";
MAKE_HOOK_OFFSETLESS(PhotonNetworkController_GetRegionWithLowestPing, Il2CppString*, GlobalNamespace::PhotonNetworkController* self)
{
    Il2CppString* resultCS = PhotonNetworkController_GetRegionWithLowestPing(self);

    std::string result = resultCS ? to_utf8(csstrtostr(resultCS)) : "";

    // if we have a region we want to force and the region is not different, then return that
    if (patchForcedRegion != "" && result != patchForcedRegion)
    {
        result = patchForcedRegion;
        patchForcedRegion = "";
    }

    return il2cpp_utils::createcsstr(result);
}

MAKE_HOOK_OFFSETLESS(ConnectionCallbacksContainer_OnConnectedToMaster, void, Photon::Realtime::ConnectionCallbacksContainer* self)
{
    ConnectionCallbacksContainer_OnConnectedToMaster(self);

    if (!MonkeRoomManager::get_instance()) il2cpp_utils::New<MonkeRoomManager*>();

    // we do some onconnectedtomaster
    MonkeRoomManager::get_instance()->OnConnectedToMaster();

}

MAKE_HOOK_OFFSETLESS(LobbyCallbacksContainer_OnRoomListUpdate, void, Photon::Realtime::LobbyCallbacksContainer* self, List<Photon::Realtime::RoomInfo*>* roomList)
{
    LobbyCallbacksContainer_OnRoomListUpdate(self, roomList);

    if (!MonkeRoomManager::get_instance()) il2cpp_utils::New<MonkeRoomManager*>();

    MonkeRoomManager::get_instance()->OnRoomListUpdate(roomList);
    // we do some OnRoomListUpdate
}

extern "C" void setup(ModInfo& info)
{
    info.id = ID;
    info.version = VERSION;

    modInfo = info;
}

extern "C" void load()
{
    getLogger().info("Loading mod...");
    GorillaUI::Init();
    
    std::string mapDir = "/sdcard/ModData/com.AnotherAxiom.GorillaTag/Mods/MonkeMapLoader/CustomMaps/";
    FileUtils::mkdir(mapDir);

    INSTALL_HOOK_OFFSETLESS(getLogger(), GorillaComputer_Start, il2cpp_utils::FindMethodUnsafe("", "GorillaComputer", "Start", 0));
    INSTALL_HOOK_OFFSETLESS(getLogger(), Player_Awake, il2cpp_utils::FindMethodUnsafe("GorillaLocomotion", "Player", "Awake", 0));
    INSTALL_HOOK_OFFSETLESS(getLogger(), Player_GetSlidePercentage, il2cpp_utils::FindMethodUnsafe("GorillaLocomotion", "Player", "GetSlidePercentage", 1));
    INSTALL_HOOK_OFFSETLESS(getLogger(), VRRig_PlayTagSound, il2cpp_utils::FindMethodUnsafe("", "VRRig", "PlayTagSound", 1));
    INSTALL_HOOK_OFFSETLESS(getLogger(), GorillaTagManager_ReportTag, il2cpp_utils::FindMethodUnsafe("", "GorillaTagManager", "ReportTag", 2));
    INSTALL_HOOK_OFFSETLESS(getLogger(), MatchMakingCallbacksContainer_OnJoinedRoom, il2cpp_utils::FindMethodUnsafe("Photon.Realtime", "MatchMakingCallbacksContainer", "OnJoinedRoom", 0));
    INSTALL_HOOK_OFFSETLESS(getLogger(), PhotonNetworkController_GetRegionWithLowestPing, il2cpp_utils::FindMethodUnsafe("", "PhotonNetworkController", "GetRegionWithLowestPing", 0));
    INSTALL_HOOK_OFFSETLESS(getLogger(), ConnectionCallbacksContainer_OnConnectedToMaster, il2cpp_utils::FindMethodUnsafe("Photon.Realtime", "ConnectionCallbacksContainer", "OnConnectedToMaster", 0));
    INSTALL_HOOK_OFFSETLESS(getLogger(), LobbyCallbacksContainer_OnRoomListUpdate, il2cpp_utils::FindMethodUnsafe("Photon.Realtime", "LobbyCallbacksContainer", "OnRoomListUpdate", 1));

    using namespace MapLoader;

    custom_types::Register::RegisterType<GorillaMapTriggerBase>();
    custom_types::Register::RegisterTypes<Teleporter, TagZone, Player, ObjectTrigger, RotateByHand, PreviewOrb, SurfaceClimbSettings, RoundEndActions, Loader, MapDescriptor, RoomList, MonkeRoomManager, MovingPlatform>();
    custom_types::Register::RegisterTypes<MapSelectorView, MapView, MapSelectorViewManager, MapNetworkJoinTrigger>();

    GorillaUI::Register::RegisterViewManager<MapSelectorViewManager*>("Map Loader", VERSION);

    getLogger().info("Mod loaded!");
}

bool getSceneName(UnityEngine::SceneManagement::Scene scene, std::string& output)
{
    Il2CppString* csString = scene.GetNameInternal(scene.m_Handle);
    if (!csString) return false;
    output = to_utf8(csstrtostr(csString));
    return true; 
}