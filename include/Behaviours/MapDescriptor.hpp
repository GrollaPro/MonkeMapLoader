#pragma once

#include "custom-types/shared/macros.hpp"
#include "UnityEngine/MonoBehaviour.hpp"

DECLARE_CLASS_CODEGEN(MapLoader, MapDescriptor, UnityEngine::MonoBehaviour,
        DECLARE_METHOD(void, Initialize);
        DECLARE_INSTANCE_FIELD(float, gravity);
    REGISTER_FUNCTION(MapDescriptor,
        REGISTER_METHOD(Initialize);
        REGISTER_FIELD(gravity);
    )
)