#pragma once

#include "custom-types/shared/macros.hpp"
#include "Behaviours/GorillaMapTriggerBase.hpp"
#include "typedefs.h"

DECLARE_CLASS_CODEGEN(MapLoader, ObjectTrigger, MapLoader::GorillaMapTriggerBase,
    DECLARE_METHOD(void, Awake);
    DECLARE_METHOD(void, OnEnable);
    DECLARE_METHOD(void, Trigger, Il2CppObject* collider);
    DECLARE_INSTANCE_FIELD(List<Il2CppObject*>*, objectsToTrigger);
    DECLARE_INSTANCE_FIELD(bool, disableObject);
    DECLARE_INSTANCE_FIELD(bool, onlyTriggerOnce);
    DECLARE_INSTANCE_FIELD(bool, triggered);
    REGISTER_FUNCTION(ObjectTrigger,
        REGISTER_METHOD(Awake);
        REGISTER_METHOD(OnEnable);
        REGISTER_METHOD(Trigger);
        REGISTER_FIELD(objectsToTrigger);
        REGISTER_FIELD(disableObject);
        REGISTER_FIELD(onlyTriggerOnce);
        REGISTER_FIELD(triggered);
    )
)