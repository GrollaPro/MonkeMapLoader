#pragma once

#include "custom-types/shared/macros.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/MonoBehaviour.hpp"

namespace MapLoader
{
    class Player;
}


DECLARE_CLASS_CODEGEN(MapLoader, Player, UnityEngine::MonoBehaviour,
    DECLARE_INSTANCE_FIELD(UnityEngine::Transform*, destination);
    DECLARE_INSTANCE_FIELD(bool, isTeleporting);

    DECLARE_METHOD(void, Start);
    DECLARE_METHOD(static void, TeleportPlayer, UnityEngine::Transform* destination);
    
    public:
        static inline Player* instance = nullptr;
    REGISTER_FUNCTION(Player,
        REGISTER_METHOD(Start);
        REGISTER_METHOD(TeleportPlayer);
        REGISTER_FIELD(isTeleporting);
        REGISTER_FIELD(destination);
    )
)