#pragma once

#include "custom-types/shared/macros.hpp"

namespace MapLoader
{
    class RoundEndActions;
}

#include "beatsaber-hook/shared/utils/typedefs.h"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/MonoBehaviour.hpp"

DECLARE_CLASS_CODEGEN(MapLoader, RoundEndActions, UnityEngine::MonoBehaviour,
    DECLARE_CTOR(ctor);
    DECLARE_INSTANCE_FIELD(bool, respawnOnRoundEnd);
    DECLARE_METHOD(static void, RoundEnd);
    DECLARE_INSTANCE_FIELD(List<UnityEngine::GameObject*>*, objectsToEnable);
    DECLARE_INSTANCE_FIELD(List<UnityEngine::GameObject*>*, objectsToDisable);

    REGISTER_FUNCTION(RoundEndActions,
        REGISTER_METHOD(ctor);
        REGISTER_METHOD(RoundEnd);
        REGISTER_FIELD(respawnOnRoundEnd);
        REGISTER_FIELD(objectsToEnable);
        REGISTER_FIELD(objectsToDisable);
    )
)