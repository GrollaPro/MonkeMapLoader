#include "UI/MapSelectorView.hpp"
#include "UI/MapView.hpp"
#include "monkecomputer/shared/GorillaUI.hpp"
#include "monkecomputer/shared/Helpers/SelectionHelper.hpp"
#include "monkecomputer/shared/Helpers/PageHelper.hpp"
#include "monkecomputer/shared/ViewLib/CustomComputer.hpp"
#include "Models/MapList.hpp"
#include "Behaviours/PreviewOrb.hpp"
#include "monkecomputer/shared/KeyExtension.hpp"

DEFINE_TYPE(MapLoader::MapSelectorView);

extern Logger& getLogger();

using namespace GorillaUI;

#define MOD_PAGE_SIZE 10

namespace MapLoader
{
    void MapSelectorView::Awake()
    {
        mapView = nullptr;

        selectionHandler = new UISelectionHandler(EKeyboardKey::Up, EKeyboardKey::Down, EKeyboardKey::Enter, true, true);
        pageSelectionHandler = new UISelectionHandler(EKeyboardKey::Left, EKeyboardKey::Right, EKeyboardKey::Enter, false, true);

        ((UISelectionHandler*)selectionHandler)->max = MOD_PAGE_SIZE;
        std::vector<MapInfo>& maps = MapList::get_maps();
        mapCount = maps.size();
        pageCount = PageHelper::GetPageAmount(maps, MOD_PAGE_SIZE);
        ((UISelectionHandler*)pageSelectionHandler)->max = pageCount;

        if (maps.size() < MOD_PAGE_SIZE)
        {
            ((UISelectionHandler*)selectionHandler)->max = maps.size();
        }
        getLogger().info("Got %d maps, on %d pages", mapCount, pageCount);
    }

    void MapSelectorView::DidActivate(bool firstActivation)
    {
        std::function<void(int)> fun = std::bind(&MapSelectorView::ShowMap, this, std::placeholders::_1);
        ((UISelectionHandler*)selectionHandler)->selectionCallback = fun;
        Redraw();
        if (firstActivation)
        {
            if (mapCount != 0)
            {
                PreviewOrb::ChangeOrb(MapList::get_map(0));
            }
        }
    }

    void MapSelectorView::ShowMap(int index)
    {
        if (!mapView) mapView = GorillaUI::CreateView<MapView*>();
        if (!mapView)
        {
            getLogger().error("Map View Was Not Created!");
            return;
        }

        index += (MOD_PAGE_SIZE * ((UISelectionHandler*)pageSelectionHandler)->currentSelectionIndex);
        getLogger().info("Loading map %d", index);
        ((MapView*)mapView)->mapInfo = &MapList::get_map(index);
        if (computer)
        {
            GorillaUI::Components::ViewManager* activeViewManager = computer->activeViewManager;
            if (activeViewManager)
            {
                activeViewManager->ReplaceTopView(mapView);
            }
            else getLogger().error("active ViewManager was nullptr");
        }
        else getLogger().error("Computer instance was nullptr");
    }

    void MapSelectorView::Redraw()
    {
        text = "";

        DrawHeader();
        DrawMaps();

        getLogger().info("Redrawing on Custom Computer!");
        CustomComputer::Redraw();
    }
    
    void MapSelectorView::DrawHeader()
    {
        text += string_format("<color=#ffff00>== <color=#fdfdfd>Maps</color> ==</color> page [<] %d/%d [>]\n", ((UISelectionHandler*)pageSelectionHandler)->currentSelectionIndex + 1, pageCount);
    }
    
    void MapSelectorView::DrawMaps()
    {
        if (mapCount == 0)
        {
            text += "<size=40>\n\n";
            text += "    You have 0 maps installed! this means that the mod is currently useless...\n";
            text += "    Download maps from <color=#0000ff>monkemaphub.com</color>\n";
            text += "    and drop them into QuestPatcher, or put them in the following filepath on your quest:\n";
            text += "    <color=#fdfdfd>sdcard/ModData/com.AnotherAxiom.GorillaTag/Mods/MonkeMapLoader/CustomMaps</color>\n";
            text += "\n";
            text += "    Once you have put in these maps you either should restart the game, or use <color#fdfdfd>option 1</color> to reload the map list\n";
            text += "</size>";
            return;
        }
        
        std::vector<MapInfo> infos = PageHelper::GetPage(MapList::get_maps(), MOD_PAGE_SIZE, ((UISelectionHandler*)pageSelectionHandler)->currentSelectionIndex);
        ((UISelectionHandler*)selectionHandler)->max = infos.size();
        ((UISelectionHandler*)selectionHandler)->currentSelectionIndex = ((UISelectionHandler*)selectionHandler)->currentSelectionIndex >= infos.size() ? infos.size() - 1 : ((UISelectionHandler*)selectionHandler)->currentSelectionIndex;
        std::vector<std::string> mapNames = {};

        for (auto m : infos)
        {
            mapNames.push_back((m.packageInfo->descriptor.mapName));
        }

        SelectionHelper::DrawSelection(mapNames, ((UISelectionHandler*)selectionHandler)->currentSelectionIndex, text);
    }
    
    void MapSelectorView::OnKeyPressed(int key)
    {
        if (mapCount == 0) return;

        if (key == (int)EKeyboardKey::Option1)
        {
            MapList::Load();
            
            std::vector<MapInfo>& maps = MapList::get_maps();
            mapCount = maps.size();
            pageCount = PageHelper::GetPageAmount(maps, MOD_PAGE_SIZE);
            ((UISelectionHandler*)pageSelectionHandler)->max = pageCount;

            if (mapCount < MOD_PAGE_SIZE)
            {
                ((UISelectionHandler*)selectionHandler)->max = maps.size();
            }
        }

        char letter;
        if (KeyExtension::TryParseLetter((EKeyboardKey)key, letter))
        {
            std::vector<MapInfo>& maps = MapList::get_maps();
            int sel = 0;
            bool found = false;

            for (auto m : maps)
            {
                if (m.packageInfo->descriptor.mapName[0] == letter)
                {
                    found = true;
                    break;
                }
                sel++;
            }

            if (found)
            {
                int page = sel / MOD_PAGE_SIZE;
                sel = sel %= MOD_PAGE_SIZE;

                ((UISelectionHandler*)selectionHandler)->currentSelectionIndex = sel;
                ((UISelectionHandler*)pageSelectionHandler)->currentSelectionIndex = page;

                int index = ((UISelectionHandler*)selectionHandler)->currentSelectionIndex + (MOD_PAGE_SIZE * ((UISelectionHandler*)pageSelectionHandler)->currentSelectionIndex);
                if (index > mapCount - 1) index = mapCount - 1;
                PreviewOrb::ChangeOrb(MapList::get_map(index));
            }
        }

        int num;
        if (KeyExtension::TryParseNumber((EKeyboardKey)key, num))
        {
            std::vector<MapInfo>& maps = MapList::get_maps();
            int sel = 0;
            bool found = false;
            for (auto m : maps)
            {
                if (m.packageInfo->descriptor.mapName[0] == num + '0')
                {
                    found = true;
                    break;
                }
                sel++;
            }
            
            if (found)
            {
                int page = sel / MOD_PAGE_SIZE;
                sel %= MOD_PAGE_SIZE;

                ((UISelectionHandler*)selectionHandler)->currentSelectionIndex = sel;
                ((UISelectionHandler*)pageSelectionHandler)->currentSelectionIndex = page;

                int index = ((UISelectionHandler*)selectionHandler)->currentSelectionIndex + (MOD_PAGE_SIZE * ((UISelectionHandler*)pageSelectionHandler)->currentSelectionIndex);
                if (index > mapCount - 1) index = mapCount - 1;
                PreviewOrb::ChangeOrb(MapList::get_map(index));
            }
        }

        if (
                ((UISelectionHandler*)selectionHandler)->HandleKey((EKeyboardKey)key) ||
                ((UISelectionHandler*)pageSelectionHandler)->HandleKey((EKeyboardKey)key)
            )
        {
            int index = ((UISelectionHandler*)selectionHandler)->currentSelectionIndex + (MOD_PAGE_SIZE * ((UISelectionHandler*)pageSelectionHandler)->currentSelectionIndex);
            if (index > mapCount - 1)
            { 
                index = mapCount - 1;
            }
            PreviewOrb::ChangeOrb(MapList::get_map(index));
            this->Redraw();

        }
        else 
        {
            this->Redraw();
        }

        this->Redraw();
    }
}