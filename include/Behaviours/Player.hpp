#pragma once

#include "custom-types/shared/macros.hpp"
#include "UnityEngine/Vector3.hpp"
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
    DECLARE_METHOD(static Player*, get_instance);
    DECLARE_METHOD(static void, TeleportPlayer, UnityEngine::Transform* destination);
    DECLARE_METHOD(static void, Offset, UnityEngine::Vector3 offset);
    DECLARE_METHOD(static UnityEngine::Vector3, get_localizedPosition);
    public:
        static inline Player* instance = nullptr;
    REGISTER_FUNCTION(
        REGISTER_METHOD(Start);
        REGISTER_METHOD(get_instance);
        REGISTER_METHOD(TeleportPlayer);
        REGISTER_METHOD(Offset);
        REGISTER_METHOD(get_localizedPosition);
        REGISTER_FIELD(isTeleporting);
        REGISTER_FIELD(destination);
    )
)