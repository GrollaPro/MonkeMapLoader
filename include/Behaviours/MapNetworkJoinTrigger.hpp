#pragma once

#include "custom-types/shared/macros.hpp"
#include "GlobalNamespace/GorillaNetworkJoinTrigger.hpp"
#include "GlobalNamespace/GorillaTriggerBox.hpp"

DECLARE_CLASS_CODEGEN(MapLoader, MapNetworkJoinTrigger, GlobalNamespace::GorillaNetworkJoinTrigger,
    DECLARE_CTOR(ctor);
    DECLARE_METHOD(static MapNetworkJoinTrigger*, get_instance);

    private:
        static inline MapNetworkJoinTrigger* instance = nullptr;
    REGISTER_FUNCTION(
        REGISTER_METHOD(ctor);
        REGISTER_METHOD(get_instance);
    )
)