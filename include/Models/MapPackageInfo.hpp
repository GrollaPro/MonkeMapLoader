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
                if (doc.HasMember("androidRequiredVersion"))
                {
                    androidRequiredVersion = doc["androidRequiredVersion"].GetString();
                }
            }

            std::string androidRequiredVersion = "1.0.0";
            std::string androidFileName = "";
            Descriptor descriptor;
            MapConfig config;
            Il2CppObject* previewCubeMap = nullptr;
    };
}