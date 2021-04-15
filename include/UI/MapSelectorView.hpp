#pragma once

#include "custom-types/shared/macros.hpp"
#include "monkecomputer/shared/ViewLib/View.hpp"
#include "monkecomputer/shared/InputHandlers/UISelectionHandler.hpp"

DECLARE_CLASS_CODEGEN(MapLoader, MapSelectorView, GorillaUI::Components::View, 
    DECLARE_METHOD(void, Awake);
    DECLARE_METHOD(void, ShowMap, int index);
    DECLARE_METHOD(void, DidActivate, bool firstActivation);
    DECLARE_METHOD(void, Redraw);
    DECLARE_METHOD(void, DrawHeader);
    DECLARE_METHOD(void, DrawMaps);
    DECLARE_METHOD(void, OnKeyPressed, int key);
    DECLARE_INSTANCE_FIELD(GorillaUI::Components::View*, mapView);
    DECLARE_INSTANCE_FIELD(int, pageCount);
    DECLARE_INSTANCE_FIELD(int, mapCount);
    DECLARE_INSTANCE_FIELD(void*, selectionHandler);
    DECLARE_INSTANCE_FIELD(void*, pageSelectionHandler);
    
    REGISTER_FUNCTION(
        REGISTER_METHOD(Awake);
        REGISTER_METHOD(ShowMap);
        REGISTER_METHOD(DidActivate);
        REGISTER_METHOD(Redraw);
        REGISTER_METHOD(DrawHeader);
        REGISTER_METHOD(DrawMaps);
        REGISTER_METHOD(OnKeyPressed);
        REGISTER_FIELD(mapView);
        REGISTER_FIELD(pageCount);
        REGISTER_FIELD(mapCount);
        REGISTER_FIELD(selectionHandler);
        REGISTER_FIELD(pageSelectionHandler);
    )
)