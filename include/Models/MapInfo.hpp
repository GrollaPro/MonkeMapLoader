#pragma once

#include <string>
#include "Models/MapPackageInfo.hpp"
namespace MapLoader
{
    class MapInfo
    {
        public:
            bool operator<(const MapInfo& second) const;
            MapInfo(std::string filePath) : filePath(filePath) {};
            void Load();
            Il2CppObject* LoadCubeMap();
            std::string filePath = "";
            MapPackageInfo* packageInfo;
    };
}