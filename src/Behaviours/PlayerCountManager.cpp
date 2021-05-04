#include "Behaviours/PlayerCountManager.hpp"

DEFINE_TYPE(MapLoader::PlayerCountManager);

#include "Photon/Pun/PhotonNetwork.hpp"

#include "Photon/Realtime/Player.hpp"
#include "Photon/Realtime/Room.hpp"
#include "Photon/Realtime/InRoomCallbacksContainer.hpp"
#include "Photon/Realtime/MatchMakingCallbacksContainer.hpp"
#include "Photon/Realtime/LoadBalancingClient.hpp"

#include "Photon/Realtime/IInRoomCallbacks.hpp"
#include "Photon/Realtime/IMatchmakingCallbacks.hpp"


#include "GlobalNamespace/GorillaTagManager.hpp"
using namespace GlobalNamespace;
using namespace Photon::Pun;
using namespace Photon::Realtime;

extern Logger& getLogger();

namespace MapLoader
{
    int PlayerCountManager::playerCount;

    void PlayerCountManager::ctor()
    {
        playerCount = 0;

        InRoomCallbacksContainer* inRoomCallbacksContainer = PhotonNetwork::_get_NetworkingClient()->InRoomCallbackTargets;
        if (inRoomCallbacksContainer)
        {
            inRoomCallbacksContainer->Add((IInRoomCallbacks*)this);
        }
        else getLogger().info("inRoom callback container did not exist");

        MatchMakingCallbacksContainer* matchMakingCallbacksContainer = PhotonNetwork::_get_NetworkingClient()->MatchMakingCallbackTargets;
        if (matchMakingCallbacksContainer)
        {
            matchMakingCallbacksContainer->Add((IMatchmakingCallbacks*)this);
        }
        else getLogger().info("matchmaking callback container did not exist");
    }

    void PlayerCountManager::OnEnable()
    {
        getLogger().info("OnEnable playercount");
        PhotonNetwork::AddCallbackTarget(this);
    }

    void PlayerCountManager::OnDisable()
    {
        getLogger().info("OnDisable playercount");
        PhotonNetwork::RemoveCallbackTarget(this);
    }

    bool PlayerCountManager::isMasterClient()
    {
        return PhotonNetwork::get_IsMasterClient();
    }

    void PlayerCountManager::OnJoinedRoom()
    {
        playerCount = PhotonNetwork::get_CurrentRoom()->get_PlayerCount();
        getLogger().info("our Player joined a room with %d players", playerCount);
    }


    void PlayerCountManager::OnPlayerEnteredRoom(Player* newPlayer)
    {
        playerCount++;
        getLogger().info("Player entered room, count is now %d", playerCount);
    }

    void PlayerCountManager::OnPlayerLeftRoom(Player* otherPlayer)
    {
        playerCount--;
        getLogger().info("Player left room, count is now %d", playerCount);
    }
}