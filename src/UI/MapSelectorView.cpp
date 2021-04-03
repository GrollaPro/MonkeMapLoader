#include "UI/MapSelectorView.hpp"
#include "UI/MapView.hpp"
#include "monkecomputer/shared/GorillaUI.hpp"
#include "monkecomputer/shared/Helpers/SelectionHelper.hpp"
#include "monkecomputer/shared/Helpers/PageHelper.hpp"
#include "monkecomputer/shared/ViewLib/CustomComputer.hpp"
#include "Models/MapList.hpp"
#include "Behaviours/PreviewOrb.hpp"
#include "monkecomputer/shared/KeyExtension.hpp"

DEFINE_CLASS(MapLoader::MapSelectorView);

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
        text += string_format("<color=#ffff00>== <color=#fdfdfd>Maps</color> ==</color> page [<] %d/%d [>]\n", ((UISelectionHandler*)pageSelectionHandler)->currentSelectionIndex, pageCount - 1);
    }
    
    void MapSelectorView::DrawMaps()
    {
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
        getLogger().info("Key %d input, mapcount: %d", key, mapCount);

        if (mapCount == 0) return;

        getLogger().info("check option 1");
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

        getLogger().info("check first letter");
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

        getLogger().info("check num");
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

        getLogger().info("check paging");
        if (
                ((UISelectionHandler*)selectionHandler)->HandleKey((EKeyboardKey)key) ||
                ((UISelectionHandler*)pageSelectionHandler)->HandleKey((EKeyboardKey)key)
            )
        {
            getLogger().info("Orb needs to be updated");
            int index = ((UISelectionHandler*)selectionHandler)->currentSelectionIndex + (MOD_PAGE_SIZE * ((UISelectionHandler*)pageSelectionHandler)->currentSelectionIndex);
            getLogger().info("Got index");
            if (index > mapCount - 1)
            { 
                getLogger().info("clamped index");
                index = mapCount - 1;
            }
            getLogger().info("Changing orb");
            PreviewOrb::ChangeOrb(MapList::get_map(index));
            getLogger().info("end of if");
            this->Redraw();

        }
        else 
        {
            this->Redraw();
        }

        getLogger().info("Should redraw");
        this->Redraw();
        getLogger().info("Done");
    }
}