#pragma once

#include "monkecomputer/shared/EKeyboardKey.hpp"
#include "custom-types/shared/macros.hpp"
#include "monkecomputer/shared/ViewLib/ViewManager.hpp"
#include "monkecomputer/shared/ViewLib/View.hpp"

DECLARE_CLASS_CODEGEN(MapLoader, MapSelectorViewManager, GorillaUI::Components::ViewManager, 
    DECLARE_METHOD(void, DidActivate, bool firstActivation);
    DECLARE_METHOD(void, NotifyOfKeyPress, int key);
    DECLARE_METHOD(void, BackButtonWasPressed, GorillaUI::Components::View* view);
    
    DECLARE_INSTANCE_FIELD(GorillaUI::Components::View*, listView);
    
    REGISTER_FUNCTION(
        REGISTER_METHOD(DidActivate);
        REGISTER_METHOD(NotifyOfKeyPress);
        REGISTER_METHOD(BackButtonWasPressed);
        REGISTER_FIELD(listView);
    )
)