#pragma once

#include <string>
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/document.h"
#include "modloader/shared/modloader.hpp"

#include "UnityEngine/Color.hpp"

namespace MapLoader
{
    class MapConfig
    {
        public:
            MapConfig(rapidjson::Value& val)
            {
                imagePath = val["imagePath"].GetString();
                cubeMapImagePath = val["cubemapImagePath"].GetString();
                rapidjson::Value& color = val["mapColor"];

                float r = color["r"].GetFloat();
                float g = color["g"].GetFloat();
                float b = color["b"].GetFloat();
                mapColor = UnityEngine::Color(r, g, b);
                gravity = val.HasMember("gravity") ? val["gravity"].GetFloat() : -9.8f;

                guid = val.HasMember("guid") ? val["guid"].GetString() : "";
                version = val.HasMember("version") ? val["version"].GetInt() : 0;

                if (val.HasMember("requiredModIDs"))
                {
                    assert(val["requiredModIDs"].IsArray());

                    for (auto& id : val["requiredModIDs"].GetArray())
                    {
                        requiredModIDs.push_back(id.GetString());
                    }
                }
            }
            
            std::vector<std::string> GetMissingModIDs()
            {
                if (requiredModIDs.size() == 0) return std::vector<std::string>{};

                const std::unordered_map<std::string, const Mod> mods = Modloader::getMods();
                
                std::vector<std::string> missingIDs = {};

                for (auto& id : requiredModIDs)
                {
                    std::unordered_map<std::string, const Mod>::const_iterator it = mods.find(id);
                    if (it == mods.end()) missingIDs.push_back(id); 
                }

                return missingIDs;
            }

            std::string imagePath = "";
            std::string cubeMapImagePath = "";
            std::vector<std::string> requiredModIDs = {};
            UnityEngine::Color mapColor = {1.0f, 1.0f, 1.0f};
            float gravity = -9.8f;
            std::string guid = "";
            int version = 0;
    };
}