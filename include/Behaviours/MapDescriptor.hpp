#pragma once

#include "custom-types/shared/macros.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/GameObject.hpp"

DECLARE_CLASS_CODEGEN(MapLoader, MapDescriptor, UnityEngine::MonoBehaviour,
        DECLARE_METHOD(void, Initialize);
        DECLARE_METHOD(static bool, CanBeDescriptor, UnityEngine::GameObject* go);
        DECLARE_INSTANCE_FIELD(float, gravity);
    REGISTER_FUNCTION(
        REGISTER_METHOD(Initialize);
        REGISTER_METHOD(CanBeDescriptor);
        REGISTER_FIELD(gravity);
    )
)