#include "UI/MapView.hpp"
#include "monkecomputer/shared/ViewLib/CustomComputer.hpp"
#include "Behaviours/MapLoader.hpp"
#include "UI/MapSelectorViewManager.hpp"

DEFINE_TYPE(MapLoader::MapView);

extern Logger& getLogger();

using namespace GorillaUI;

namespace MapLoader
{
    void MapView::Awake()
    {
        loaded = false;
        mapInfo = nullptr;
    }

    void MapView::DidActivate(bool firstActivation)
    {
        loaded = false;
        Redraw();
    }

    void MapView::Load()
    {
        if (!mapInfo)
        {
            return;
        }
        
        static std::vector<Il2CppClass*> loaderKlass = {classof(Loader*)};
        Loader* loader = *il2cpp_utils::RunGenericMethod<Loader*>("UnityEngine", "Object", "FindObjectOfType", loaderKlass);
        loader->LoadMap(*(MapInfo*)mapInfo);
    }

    void MapView::Redraw()
    {
        text = "";

        if (!loaded)
        {
            DrawHeader();
            DrawMap();
        }
        else
        {
            /*
            text += "                                                          <size=25>\n";
            text += "                                                                <color=#2222ff>##########\n";
            text += "                                                          ######<color=#00dd00>####</color>############\n";
            text += "                                                      ########<color=#00dd00>######</color>################\n";
            text += "                                                    <color=#00dd00>####</color>########<color=#00dd00>####</color>################<color=#00dd00>##</color>\n";
            text += "                                                  <color=#00dd00>########</color>####<color=#00dd00>##</color>##<color=#00dd00>##</color>##############<color=#00dd00>######</color>\n";
            text += "                                                  <color=#00dd00>##########</color>##################<color=#00dd00>##########</color>\n";
            text += "                                                <color=#00dd00>####################</color>########<color=#00dd00>##############</color>\n";
            text += "                                                <color=#00dd00>############</color>##<color=#00dd00>######</color>########<color=#00dd00>##############</color>\n";
            text += "                                                <color=#00dd00>##########</color>##<color=#00dd00>########</color>######<color=#00dd00>##########</color>####<color=#00dd00>##</color>\n";
            text += "                                              <color=#00dd00>####################</color>########<color=#00dd00>########</color>########<color=#00dd00>##</color>\n";
            text += "                                              <color=#00dd00>####################</color>##########<color=#00dd00>####</color>####<color=#00dd00>########</color>\n";
            text += "                                              <color=#00dd00>##################</color>################<color=#00dd00>############</color>\n";
            text += "                                              <color=#00dd00>##################</color>##############<color=#00dd00>##############</color>\n";
            text += "                                              ##########<color=#00dd00>######</color>################<color=#00dd00>##############</color>\n";
            text += "                                                ############<color=#00dd00>##</color>##############<color=#00dd00>##############</color>\n";
            text += "                                                ######<color=#00dd00>##</color>####<color=#00dd00>##</color>##############<color=#00dd00>##############</color>\n";
            text += "                                                ####<color=#00dd00>##</color>######################<color=#00dd00>##############</color>\n";
            text += "                                                  ##########################<color=#00dd00>############</color>\n";
            text += "                                                  ############################<color=#00dd00>##########</color>\n";
            text += "                                                    ####<color=#00dd00>##</color>####################<color=#00dd00>########</color>\n";
            text += "                                                      <color=#00dd00>####</color>####################<color=#00dd00>######</color>\n";
            text += "                                                          ##<color=#00dd00>####</color>################\n";
            text += "                                                                ##########</color>\n";
            text += "                                              </size>\n";
            text += "                 MAP LOADED\n";
            */
            text += "\n";
            text += "<size=12><color=#006ca2>\n";
            text += "                                                                                                                              ####################\n";
            text += "                                                                                                                        ################################\n";
            text += "                                                                                                                   ###########################################\n";
            text += "                                                                                                               ##################################################\n";
            text += "                                                                                                            ###########    ######### ###### #########    ############\n";
            text += "                                                                                                          ##########      #######    ######    #######      ##########\n";
            text += "                                                                                                       #########        #######      ######      #######         ########\n";
            text += "                                                                                                     #########         #######       ######       #######         #########\n";
            text += "                                                                                                   ########           ######       ##########      #######           ########\n";
            text += "                                                                                                 #######            ########################################            ########\n";
            text += "                                                                                                ########           ##########################################            #######\n";
            text += "                                                                                              #############       #######                              #######       #############\n";
            text += "                                                                                              ###############  ########                                   #######  ################\n";
            text += "                                                                                            ######    #############                                          #############    ######\n";
            text += "                                                                                           ######        #########                                            #########        ######\n";
            text += "                                                                                          ######           ####                                                  ####           ######\n";
            text += "                                                                                          ######          #####                                                  #####          #######\n";
            text += "                                                                                         ##############  #####                                                    #####  ##############\n";
            text += "                                                                                        #####################                                                      #####################\n";
            text += "                                                                                       ######        #######                                                         ######        ######\n";
            text += "                                                                                       ####                                                                                          ####\n";
            text += "                                                                                      #####                                                                                          #####\n";
            text += "                                                                                      #####                                                                                          #####\n";
            text += "                                                                                      ######                                                                                        ######\n";
            text += "                                                                                      ########                                                                                     #######\n";
            text += "                                                                                      ###########                                                                               ##########\n";
            text += "                                                                                      ###############                                                                       ##############\n";
            text += "                                                                                      ######   ########################                                   #######################   ######\n";
            text += "                                                                                      ######       ######################                              ######################       ######\n";
            text += "                                                                                       ######                ######  ########                      ########   #####                ######\n";
            text += "                                                                                       ######                ######    #######                    #######     #####                ######\n";
            text += "                                                                                        ######                #####       ######                ######       #####                 #####\n";
            text += "                                                                                        ######                #####         #####               ####         #####                ######\n";
            text += "                                                                                         ######                ##################               #################                ######\n";
            text += "                                                                                          ######              ##################                ##################              ######\n";
            text += "                                                                                          #######          #####################                #####################           ######\n";
            text += "                                                                                            ######     ################    ####                  ####     ###############     ######\n";
            text += "                                                                                             ####### ##########  ######   #####                  #####   ######  ##################\n";
            text += "                                                                                              #############       ######  ####                    ####  ######       #############\n";
            text += "                                                                                               #########           ###### ####                    #### #######          #########\n";
            text += "                                                                                                 #######            ##########                    ##########            ########\n";
            text += "                                                                                                  ########           #########                    #########           ########\n";
            text += "                                                                                                     #######           ###################################          ########\n";
            text += "                                                                                                       ########         ################################         ########\n";
            text += "                                                                                                        ###########       #######    #######   ########       ##########\n";
            text += "                                                                                                            ###########    ######### ###### #########    ############\n";
            text += "                                                                                                              ####################################################\n";
            text += "                                                                                                                   ###########################################\n";
            text += "                                                                                                                      ####################################\n";
            text += "                                                                                                                              ####################\n";
            text += "</color></size>\n";
            text += "                 <color=#fdfdfd>MAP LOADED\n";
            text += "       GET MORE MAPS AT</color> <color=#5555ff>MONKEMAPHUB.COM</color>\n";

        }

        CustomComputer::Redraw();
    }
    
    void MapView::DrawHeader()
    {
        text += string_format("<color=#ffff00>== <color=#fdfdfd>Map Details</color> ==</color>\n");
    }
    
    void MapView::DrawMap()
    {
        if (!mapInfo) return;
        text += "\n<size=60>";
        text += string_format("  Author: <color=#fdfdfd>%s</color>\n", ((MapInfo*)mapInfo)->packageInfo->descriptor.author.c_str());
        text += string_format("  Map Name: <color=#fdfdfd>%s</color>\n", ((MapInfo*)mapInfo)->packageInfo->descriptor.mapName.c_str());
        text += string_format("  Description: <color=#fdfdfd>%s</color>\n", ((MapInfo*)mapInfo)->packageInfo->descriptor.description.c_str());
        text += "\n</size>";
    }
    
    void MapView::OnKeyPressed(int key)
    {
        if (loaded) return;
        if (key == (int)EKeyboardKey::Enter)
        {
            loaded = true;
            Load();
        }

        Redraw();
    }
}