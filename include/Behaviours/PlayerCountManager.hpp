#pragma once

#include "custom-types/shared/macros.hpp"

#include "Photon/Pun/MonoBehaviourPunCallbacks.hpp"
#include "Photon/Realtime/Player.hpp"

DECLARE_CLASS_CODEGEN(MapLoader, PlayerCountManager, Photon::Pun::MonoBehaviourPunCallbacks,
        DECLARE_CTOR(ctor);
        DECLARE_OVERRIDE_METHOD(void, OnEnable, il2cpp_utils::FindMethodUnsafe("Photon.Pun", "MonoBehaviourPunCallbacks", "OnEnable", 0));
        DECLARE_OVERRIDE_METHOD(void, OnDisable, il2cpp_utils::FindMethodUnsafe("Photon.Pun", "MonoBehaviourPunCallbacks", "OnDisable", 0));
        DECLARE_OVERRIDE_METHOD(void, OnPlayerEnteredRoom, il2cpp_utils::FindMethodUnsafe("Photon.Pun", "MonoBehaviourPunCallbacks", "OnPlayerEnteredRoom", 1), Photon::Realtime::Player* newPlayer);
        DECLARE_OVERRIDE_METHOD(void, OnPlayerLeftRoom, il2cpp_utils::FindMethodUnsafe("Photon.Pun", "MonoBehaviourPunCallbacks", "OnPlayerLeftRoom", 1), Photon::Realtime::Player* otherPlayer);
        DECLARE_OVERRIDE_METHOD(void, OnJoinedRoom, il2cpp_utils::FindMethodUnsafe("Photon.Pun", "MonoBehaviourPunCallbacks", "OnJoinedRoom", 0));
        DECLARE_METHOD(static bool, isMasterClient);
        DECLARE_STATIC_FIELD(int, playerCount);
        
    REGISTER_FUNCTION(
        REGISTER_METHOD(ctor);
        REGISTER_METHOD(isMasterClient);
        REGISTER_METHOD(OnPlayerEnteredRoom);
        REGISTER_METHOD(OnPlayerLeftRoom);
        REGISTER_METHOD(OnJoinedRoom);
        REGISTER_METHOD(OnEnable);
        REGISTER_METHOD(OnDisable);
        REGISTER_FIELD(playerCount);
    )
)