#pragma once

#include "custom-types/shared/macros.hpp"
#include "UnityEngine/MonoBehaviour.hpp"

DECLARE_CLASS_CODEGEN(MapLoader, SurfaceClimbSettings, UnityEngine::MonoBehaviour,
    DECLARE_METHOD(void, Start);
    DECLARE_INSTANCE_FIELD(bool, unClimbable);
    DECLARE_INSTANCE_FIELD(float, slipPercentage);

    REGISTER_FUNCTION(SurfaceClimbSettings,
        REGISTER_METHOD(Start);
        REGISTER_FIELD(unClimbable);
        REGISTER_FIELD(slipPercentage);
    )
)