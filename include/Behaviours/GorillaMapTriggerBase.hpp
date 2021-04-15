#pragma once

#include "custom-types/shared/macros.hpp"

namespace MapLoader {
    enum class TouchType {
        Any,
        Head,
        Hands
    };
}

#include "UnityEngine/Collider.hpp"
#include "UnityEngine/MonoBehaviour.hpp"

DECLARE_CLASS_CODEGEN(MapLoader, GorillaMapTriggerBase, UnityEngine::MonoBehaviour, 

        DECLARE_METHOD(void, OnTriggerEnter, UnityEngine::Collider* collider);
        DECLARE_METHOD(void, OnTriggerStay, UnityEngine::Collider* collider);
        DECLARE_METHOD(void, OnTriggerExit, UnityEngine::Collider* collider);
        DECLARE_METHOD(void, RunTrigger, UnityEngine::Collider* collider);
        DECLARE_METHOD(bool, CheckColliderComponents, UnityEngine::Collider* collider);
        DECLARE_INSTANCE_FIELD(float, Delay);
        DECLARE_INSTANCE_FIELD(float, touchedTime);
        DECLARE_INSTANCE_FIELD(bool, isTriggering);
        DECLARE_INSTANCE_FIELD(UnityEngine::Collider*, collider);

    public:
        TouchType touchType = TouchType::Any;

    REGISTER_FUNCTION(
        REGISTER_METHOD(OnTriggerEnter);
        REGISTER_METHOD(OnTriggerStay);
        REGISTER_METHOD(OnTriggerExit);
        REGISTER_METHOD(RunTrigger);
        REGISTER_METHOD(CheckColliderComponents);
        REGISTER_FIELD(Delay);
        REGISTER_FIELD(touchedTime);
        REGISTER_FIELD(isTriggering);
        REGISTER_FIELD(collider);
    )
)