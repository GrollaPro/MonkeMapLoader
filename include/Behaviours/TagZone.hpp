#pragma once

#include "custom-types/shared/macros.hpp"
#include "Behaviours/GorillaMapTriggerBase.hpp"

DECLARE_CLASS_CODEGEN(MapLoader, TagZone, MapLoader::GorillaMapTriggerBase,
    DECLARE_METHOD(void, Awake);
    DECLARE_METHOD(void, Trigger, Il2CppObject* collider);
    DECLARE_METHOD(static void, TagLocalPlayer);
    DECLARE_INSTANCE_FIELD(bool, canBeTagged);

    REGISTER_FUNCTION(
        REGISTER_METHOD(Awake);
        REGISTER_METHOD(Trigger);
        REGISTER_METHOD(TagLocalPlayer);
        REGISTER_FIELD(canBeTagged);
    )
)