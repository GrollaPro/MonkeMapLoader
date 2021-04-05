#include "Models/MapList.hpp"
#include "Utils/FileUtils.hpp"
#include <algorithm>

namespace MapLoader
{
    void MapList::Load()
    {
        maps.clear();
        std::string basePath = "/sdcard/ModData/com.AnotherAxiom.GorillaTag/Mods/MonkeMapLoader/CustomMaps/";
        std::vector<std::string> fileNames;
        FileUtils::GetFilesInFolderPath("gtmap", basePath, fileNames);

        int i = 0;
        for (auto f : fileNames)
        {
            std::string filePath = basePath + f;
            maps.push_back(MapInfo(filePath));
            maps[i].Load();
            i++;
        }

        std::sort(maps.begin(), maps.end());

    }
}