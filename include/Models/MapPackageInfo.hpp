#pragma once

#include "Models/MapConfig.hpp"
#include "Models/Descriptor.hpp"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/document.h"

extern Logger& getLogger();
namespace MapLoader
{
    class MapPackageInfo
    {
        public:
            MapPackageInfo(rapidjson::Document& doc) : 
            descriptor(Descriptor(doc["descriptor"])),
            config(MapConfig(doc["config"]))
            {
                androidFileName = doc["androidFileName"].GetString();
            }

            std::string androidFileName = "";
            Descriptor descriptor;
            MapConfig config;
            Il2CppObject* previewCubeMap = nullptr;
    };
}