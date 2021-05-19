#pragma once

#include "custom-types/shared/macros.hpp"
#include "monkecomputer/shared/ViewLib/View.hpp"
#include "monkecomputer/shared/InputHandlers/UISelectionHandler.hpp"
#include "Models/MapInfo.hpp"

DECLARE_CLASS_CODEGEN(MapLoader, MapView, GorillaUI::Components::View, 
    DECLARE_METHOD(void, Awake);
    DECLARE_METHOD(void, Load);
    DECLARE_METHOD(void, DidActivate, bool firstActivation);
    DECLARE_METHOD(void, Redraw);
    DECLARE_METHOD(void, DrawHeader);
    DECLARE_METHOD(void, DrawMap);
    DECLARE_METHOD(void, OnKeyPressed, int key);
    DECLARE_INSTANCE_FIELD(bool, loaded);
    DECLARE_INSTANCE_FIELD(void*, mapInfo);
    DECLARE_INSTANCE_FIELD(bool, isUpdated);

    public:
        std::vector<std::string> missingModIDs = {};
    REGISTER_FUNCTION(
        REGISTER_METHOD(Awake);
        REGISTER_METHOD(Load);
        REGISTER_METHOD(DidActivate);
        REGISTER_METHOD(Redraw);
        REGISTER_METHOD(DrawHeader);
        REGISTER_METHOD(DrawMap);
        REGISTER_METHOD(OnKeyPressed);
        REGISTER_FIELD(loaded);
        REGISTER_FIELD(mapInfo);
        REGISTER_FIELD(isUpdated);
    )
)