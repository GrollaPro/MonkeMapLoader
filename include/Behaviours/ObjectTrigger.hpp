#pragma once

#include "custom-types/shared/macros.hpp"
#include "Behaviours/GorillaMapTriggerBase.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Collider.hpp"
#include "beatsaber-hook/shared/utils/typedefs.h"

DECLARE_CLASS_CODEGEN(MapLoader, ObjectTrigger, MapLoader::GorillaMapTriggerBase,
    DECLARE_CTOR(ctor);
    DECLARE_METHOD(void, OnEnable);
    DECLARE_METHOD(void, Trigger, UnityEngine::Collider* collider);
    DECLARE_INSTANCE_FIELD(List<UnityEngine::GameObject*>*, objectsToTrigger);
    DECLARE_INSTANCE_FIELD(bool, disableObject);
    DECLARE_INSTANCE_FIELD(bool, onlyTriggerOnce);
    DECLARE_INSTANCE_FIELD(bool, triggered);

    REGISTER_FUNCTION(
        REGISTER_METHOD(ctor);
        REGISTER_METHOD(OnEnable);
        REGISTER_METHOD(Trigger);
        REGISTER_FIELD(objectsToTrigger);
        REGISTER_FIELD(disableObject);
        REGISTER_FIELD(onlyTriggerOnce);
        REGISTER_FIELD(triggered);
    )
)