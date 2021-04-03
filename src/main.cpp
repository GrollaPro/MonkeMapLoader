#include "modloader/shared/modloader.hpp"
#include "beatsaber-hook/shared/utils/logging.hpp"
#include "beatsaber-hook/shared/utils/utils.h"
#include "quest-cosmetic-loader/shared/CosmeticLoader.hpp"
#include "typedefs.h"
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

#include "UI/MapView.hpp"
#include "UI/MapSelectorView.hpp"
#include "UI/MapSelectorViewManager.hpp"

#include "monkecomputer/shared/Register.hpp"

#include "Models/MapList.hpp"

#include "custom-types/shared/register.hpp"
#include "Utils/FileUtils.hpp"

#include "beatsaber-hook/shared/utils/il2cpp-utils-methods.hpp"

ModInfo modInfo;

Logger& getLogger()
{
    static Logger* logger = new Logger(modInfo, LoggerOptions(false, true));
    return *logger;
}

bool getSceneName(Scene scene, std::string& output);

using namespace CosmeticsLoader;
using namespace MapLoader;

MAKE_HOOK_OFFSETLESS(Player_Awake, void, Il2CppObject* self)
{
    Player_Awake(self);
    Il2CppObject* gameObject = *il2cpp_utils::RunMethod(self, "get_gameObject");
    il2cpp_utils::RunGenericMethod(gameObject, "AddComponent", std::vector<Il2CppClass*>{classof(MapLoader::Player*)});
}

MAKE_HOOK_OFFSETLESS(GorillaComputer_Start, void, Il2CppObject* self)
{
    GorillaComputer_Start(self);
    MapList::Load();

    static std::vector<Il2CppClass*> goKlass = {il2cpp_utils::GetClassFromName("UnityEngine", "GameObject")};

    Il2CppObject* newGo = *il2cpp_utils::New(goKlass[0]);
    Loader* loader = *il2cpp_utils::RunGenericMethod<Loader*>(newGo, "AddComponent", std::vector<Il2CppClass*>{classof(Loader*)});
    loader->Initialize();
}

static bool disabledBecauseUnclimbable = false;
MAKE_HOOK_OFFSETLESS(Player_CollisionsSphereCast, bool, Il2CppObject* self, Vector3 startPosition, float sphereRadius, Vector3 movementVector, float precision, Vector3* finalPosition, RaycastHit* hitInfo)
{

    bool result = Player_CollisionsSphereCast(self, startPosition, sphereRadius, movementVector, precision, finalPosition, hitInfo);
    return result;
    if (result)
    {
        float minimumRaycastDistance = *il2cpp_utils::GetFieldValue<float>(self, "minimumRaycastDistance");
        if (sphereRadius == minimumRaycastDistance)
        {
            Il2CppObject* collider = *il2cpp_utils::RunMethod(*hitInfo, "get_collider");
            Il2CppObject* go = *il2cpp_utils::RunMethod(collider, "get_gameObject");

            static std::vector<Il2CppClass*> surfaceKlass = {classof(SurfaceClimbSettings*)};
            SurfaceClimbSettings* settings = *il2cpp_utils::RunGenericMethod<SurfaceClimbSettings*>(go, "GetComponent", surfaceKlass);

            if (settings && settings->unClimbable)
            {
                bool disableMovement = *il2cpp_utils::GetFieldValue<bool>(self, "disableMovement");

                if (!disableMovement)
                {
                    il2cpp_utils::SetFieldValue(self, "disableMovement", true);
                    disabledBecauseUnclimbable = true;
                }
            }
        }
    }

    return result;
}

MAKE_HOOK_OFFSETLESS(Player_Update, void, Il2CppObject* self)
{
    Player_Update(self);
    return;
    bool disableMovement = *il2cpp_utils::GetFieldValue<bool>(self, "disableMovement");
    if (disableMovement && disabledBecauseUnclimbable)
    {
        il2cpp_utils::SetFieldValue(self, "disableMovement", false);
        disabledBecauseUnclimbable = false;
    }
}

MAKE_HOOK_OFFSETLESS(Player_GetSlidePercentage, float, Il2CppObject* self, RaycastHit raycastHit)
{
    Il2CppObject* collider = *il2cpp_utils::RunMethod(raycastHit, "get_collider");
    Il2CppObject* go = *il2cpp_utils::RunMethod(collider, "get_gameObject");
    static std::vector<Il2CppClass*> settingsKlass = {classof(SurfaceClimbSettings*)};
    SurfaceClimbSettings* settings = *il2cpp_utils::RunGenericMethod<SurfaceClimbSettings*>(go, "GetComponent", settingsKlass);
    
    static auto* get_sharedMesh = il2cpp_utils::FindMethodUnsafe("UnityEngine", "MeshCollider", "get_sharedMesh", 0);
    Il2CppObject* sharedMesh = *il2cpp_utils::RunMethod(collider, get_sharedMesh);

    if (settings)
    {
        static std::vector<Il2CppClass*> surfaceKlass = {il2cpp_utils::GetClassFromName("GorillaLocomotion", "Surface")};
        Il2CppObject* surface = *il2cpp_utils::RunGenericMethod(go, "GetComponent", surfaceKlass);
        float slipPercentage = *il2cpp_utils::GetFieldValue<float>(surface, "slipPercentage");
        return slipPercentage;
    }

    if (!collider || !sharedMesh)
    {
        return *il2cpp_utils::GetFieldValue<float>(self, "defaultSlideFactor");
    }
    
    bool isReadable = *il2cpp_utils::RunMethod<bool>(sharedMesh, "get_isReadable");
    
    if (!isReadable)
    {
        return *il2cpp_utils::GetFieldValue<float>(self, "defaultSlideFactor");
    }
    return Player_GetSlidePercentage(self, raycastHit);
}

static double lastGameEnd = 0.0;
MAKE_HOOK_OFFSETLESS(VRRig_PlayTagSound, void, Il2CppObject* self, int soundIndex)
{
    VRRig_PlayTagSound(self, soundIndex);

    Il2CppObject* photonNetworkController = *il2cpp_utils::GetFieldValue("", "PhotonNetworkController", "instance");
    if (photonNetworkController)
    {
        Il2CppString* currentGameTypeCS = *il2cpp_utils::GetFieldValue<Il2CppString*>(photonNetworkController, "currentGameType");
        std::string currentGameType = currentGameTypeCS ? to_utf8(csstrtostr(currentGameTypeCS)) : "";

        if (Loader::lobbyName != "" && currentGameType.find(Loader::lobbyName) != std::string::npos)
        {
            Il2CppObject* gorillaTagManager = *il2cpp_utils::GetFieldValue("", "GorillaTagManager", "instance");
            bool isCurrentlyTag = *il2cpp_utils::GetFieldValue<bool>(gorillaTagManager, "isCurrentlyTag");
            double timeInfectedGameEnded = *il2cpp_utils::GetFieldValue<double>(gorillaTagManager, "timeInfectedGameEnded");
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

MAKE_HOOK_OFFSETLESS(GorillaTagManager_ReportTag, void, Il2CppObject* self, Il2CppObject* taggedPlayer, Il2CppObject* taggingPlayer)
{
    getLogger().info("Player Tagged!");
    GorillaTagManager_ReportTag(self, taggedPlayer, taggingPlayer);
    Il2CppObject* photonView = *il2cpp_utils::RunMethod(self, "get_photonView");
    bool IsMine = *il2cpp_utils::RunMethod<bool>(photonView, "get_IsMine");

    if (IsMine && taggedPlayer == taggingPlayer)
    {
        static Il2CppClass* raiseEventOptionsKlass = il2cpp_utils::GetClassFromName("Photon.Realtime", "RaiseEventOptions");
        Il2CppObject* raiseEventOptions = *il2cpp_utils::New(raiseEventOptionsKlass);
        static Il2CppClass* webFlagsKlass = il2cpp_utils::GetClassFromName("Photon.Realtime", "WebFlags");
        Il2CppObject* flags = *il2cpp_utils::New(webFlagsKlass, (uint8_t) 1);
        il2cpp_utils::SetFieldValue(raiseEventOptions, "Flags", flags);

        bool isCurrentlyTag = *il2cpp_utils::GetFieldValue<bool>(self, "isCurrentlyTag");

        Il2CppString* taggingPlayerID = *il2cpp_utils::RunMethod<Il2CppString*>(taggingPlayer, "get_UserId");
        Il2CppString* taggedPlayerID = *il2cpp_utils::RunMethod<Il2CppString*>(taggedPlayer, "get_UserId");

        if (isCurrentlyTag)
        {
            Il2CppObject* currentIt = *il2cpp_utils::GetFieldValue(self, "currentIt");
            if (currentIt != taggedPlayer)
            {
                il2cpp_utils::RunMethod(self, "ChangeCurrentIt", taggedPlayer);

                static Il2CppClass* HashtableKlass = il2cpp_utils::GetClassFromName("ExitGames.Client.Photon", "Hashtable");
                
                /*
                Il2CppObject* hashTable = *il2cpp_utils::New(HashtableKlass);
                static Il2CppString* lastTag = il2cpp_utils::createcsstr("lastTag", il2cpp_utils::StringType::Manual);
                double time = *il2cpp_utils::RunMethod<double>("Photon.Pun", "PhotonNetwork", "get_Time");
                il2cpp_utils::RunMethod(hashTable, "Add", lastTag, time);

                Il2CppObject* currentRoom = *il2cpp_utils::GetFieldValue(self, "currentRoom");
                
                Il2CppObject* expectedProperties = *il2cpp_utils::New(HashtableKlass);
                Il2CppObject* otherFlags = *il2cpp_utils::New(webFlagsKlass, (uint8_t)1);
                auto* SetCustomProperties = il2cpp_utils::FindMethodUnsafe(currentRoom, "SetCustomProperties", 3);]
                ::il2cpp_utils::RunMethodThrow<bool, false>(currentRoom, SetCustomProperties, hashTable, nullptr, nullptr);
                */

                double time = (double)*il2cpp_utils::RunMethod<float>("UnityEngine", "Time", "get_time");
                il2cpp_utils::SetFieldValue(self, "lastTag", time);

                Array<Il2CppObject*>* eventContent = reinterpret_cast<Array<Il2CppObject*>*>(il2cpp_functions::array_new(classof(Il2CppObject*), 2));

                eventContent->values[0] = (Il2CppObject*)taggingPlayerID;
                eventContent->values[1] = (Il2CppObject*)taggedPlayerID;
                SendOptions options = *il2cpp_utils::GetFieldValue<SendOptions>("ExitGames.Client.Photon", "SendOptions", "SendReliable");
                il2cpp_utils::RunMethod("Photon.Pun", "PhotonNetwork", "RaiseEvent", (uint8_t)1, (Il2CppObject*)eventContent, raiseEventOptions, options);
            }
        }
        else
        {
            List<Il2CppObject*>* currentInfected = *il2cpp_utils::GetFieldValue<List<Il2CppObject*>*>(self, "currentInfected");
            bool contains = false;
            
            for (int i = 0; i < currentInfected->size; i++)
            {
                if (currentInfected->items->values[i] == taggedPlayer) 
                {
                    contains = true;
                    break;
                }
            }

            if (!contains)
            {
                il2cpp_utils::RunMethod(self, "AddInfectedPlayer", taggedPlayer);

                Array<Il2CppObject*>* eventContent = reinterpret_cast<Array<Il2CppObject*>*>(il2cpp_functions::array_new(classof(Il2CppObject*), 3));
                eventContent->values[0] = (Il2CppObject*)taggingPlayerID;
                eventContent->values[1] = (Il2CppObject*)taggedPlayerID;
                eventContent->values[2] = reinterpret_cast<Il2CppObject*>(*il2cpp_utils::RunMethod<int>(currentInfected, "get_Count"));
                SendOptions options = *il2cpp_utils::GetFieldValue<SendOptions>("ExitGames.Client.Photon", "SendOptions", "SendReliable");
                il2cpp_utils::RunMethod("Photon.Pun", "PhotonNetwork", "RaiseEvent", 2, eventContent, raiseEventOptions, options);
            }
        }
    }
    else
    {
        getLogger().error("IsMine: %d, TaggedPlayer: %p, TaggingPlayer: %p", IsMine, taggedPlayer, taggingPlayer);
    }
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
    Modloader::requireMod("MonkeComputer", "1.0.3");
    std::string mapDir = "/sdcard/ModData/com.AnotherAxiom.GorillaTag/Mods/MonkeMapLoader/CustomMaps/";
    FileUtils::mkdir(mapDir);

    INSTALL_HOOK_OFFSETLESS(getLogger(), GorillaComputer_Start, il2cpp_utils::FindMethodUnsafe("", "GorillaComputer", "Start", 0));
    INSTALL_HOOK_OFFSETLESS(getLogger(), Player_Awake, il2cpp_utils::FindMethodUnsafe("GorillaLocomotion", "Player", "Awake", 0));
    INSTALL_HOOK_OFFSETLESS(getLogger(), Player_CollisionsSphereCast, il2cpp_utils::FindMethodUnsafe("GorillaLocomotion", "Player", "CollisionsSphereCast", 6));
    INSTALL_HOOK_OFFSETLESS(getLogger(), Player_Update, il2cpp_utils::FindMethodUnsafe("GorillaLocomotion", "Player", "Update", 0));
    INSTALL_HOOK_OFFSETLESS(getLogger(), Player_GetSlidePercentage, il2cpp_utils::FindMethodUnsafe("GorillaLocomotion", "Player", "GetSlidePercentage", 1));
    INSTALL_HOOK_OFFSETLESS(getLogger(), VRRig_PlayTagSound, il2cpp_utils::FindMethodUnsafe("", "VRRig", "PlayTagSound", 1));
    INSTALL_HOOK_OFFSETLESS(getLogger(), GorillaTagManager_ReportTag, il2cpp_utils::FindMethodUnsafe("", "GorillaTagManager", "ReportTag", 2));
    
    using namespace MapLoader;
    custom_types::Register::RegisterType<GorillaMapTriggerBase>();
    custom_types::Register::RegisterTypes<Teleporter, TagZone, Player, ObjectTrigger, RotateByHand, PreviewOrb, SurfaceClimbSettings, RoundEndActions, Loader, MapDescriptor>();
    custom_types::Register::RegisterTypes<MapSelectorView, MapView, MapSelectorViewManager>();

    GorillaUI::Register::RegisterViewManager<MapSelectorViewManager*>("Map Loader", VERSION);

    getLogger().info("Mod loaded!");
}

bool getSceneName(Scene scene, std::string& output)
{
    Il2CppString* csString = *il2cpp_utils::RunMethod<Il2CppString*>("UnityEngine.SceneManagement", "Scene", "GetNameInternal", scene.m_Handle);
    if (!csString) return false;
    output = to_utf8(csstrtostr(csString));
    return true; 
}