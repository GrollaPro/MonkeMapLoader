#include "Utils/RoomUtils.hpp"
#include "beatsaber-hook/shared/utils/utils.h"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "Photon/Pun/PhotonNetwork.hpp"
#include "Photon/Pun/ServerSettings.hpp"
#include "Photon/Realtime/LoadBalancingClient.hpp"
#include "Photon/Realtime/Room.hpp"

#include "GlobalNamespace/PhotonNetworkController.hpp"
#include "GlobalNamespace/GorillaComputer.hpp"

#include "ExitGames/Client/Photon/Hashtable.hpp"

#include "UnityEngine/PlayerPrefs.hpp"
#include "UnityEngine/SkinnedMeshRenderer.hpp"

extern Logger& getLogger();

using namespace Photon::Pun;
using namespace Photon::Realtime;
using namespace UnityEngine;

namespace MapLoader::RoomUtils
{
    void JoinModdedLobby(std::string map)
    {
        std::string gameModeName = "infection_MOD_" + map;
        getLogger().info("Attempting to join a room with gamemode %s", gameModeName.c_str());
        GlobalNamespace::PhotonNetworkController* photonNetworkController = GlobalNamespace::PhotonNetworkController::_get_instance();

        bool InRoom = PhotonNetwork::get_InRoom();

        Room* currentRoom = PhotonNetwork::get_CurrentRoom();
        std::string gameMode = "";
        if (currentRoom)
        {
            ExitGames::Client::Photon::Hashtable* customProperties = currentRoom->get_CustomProperties();
            static Il2CppString* gameModeLabel = il2cpp_utils::createcsstr("gameMode", il2cpp_utils::StringType::Manual);
            Il2CppString* gameModeCS = (Il2CppString*)customProperties->get_Item((Il2CppObject*)gameModeLabel);
            gameMode = gameModeCS ? to_utf8(csstrtostr(gameModeCS)) : "";
        }

        if (InRoom && gameMode.find("privatetag") != std::string::npos) return;

        Il2CppString* gameType = il2cpp_utils::createcsstr(gameModeName);

        static Il2CppString* queueCS = il2cpp_utils::createcsstr("DEFAULT", il2cpp_utils::StringType::Manual);
        GlobalNamespace::GorillaComputer::_get_instance()->currentQueue = queueCS;

        static Il2CppString* currentQueue = il2cpp_utils::createcsstr("currentQueue", il2cpp_utils::StringType::Manual);
        PlayerPrefs::SetString(currentQueue, queueCS);
        PlayerPrefs::Save();

        if (InRoom && gameMode != gameModeName)
        {
            photonNetworkController->currentGameType = gameType;
            photonNetworkController->attemptingToConnect = true;

            Array<SkinnedMeshRenderer*>* offlineVRRig = photonNetworkController->offlineVRRig;

            for (int i = 0; i < offlineVRRig->Length(); i++)
            {
                offlineVRRig->values[i]->set_enabled(true);
            }

            PhotonNetwork::Disconnect();
            getLogger().info("Disconnected from room");
            return;
        }

        bool attemptingToConnect = photonNetworkController->attemptingToConnect;
        if (!InRoom && !attemptingToConnect)
        {
            photonNetworkController->currentGameType = gameType;
            photonNetworkController->attemptingToConnect = true;
            photonNetworkController->AttemptToConnectToRoom();
            getLogger().info("Attempting room connect");
        }
    }
}