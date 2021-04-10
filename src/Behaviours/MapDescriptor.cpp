#include "Behaviours/MapDescriptor.hpp"
#include "beatsaber-hook/shared/rapidjson/include/rapidjson/document.h"
#include <unordered_map>
#include <map>
#include "Behaviours/Teleporter.hpp"
#include "Behaviours/TagZone.hpp"
#include "Behaviours/ObjectTrigger.hpp"
#include "Behaviours/SurfaceClimbSettings.hpp"
#include "Behaviours/RoundEndActions.hpp"

#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/UI/Text.hpp"

DEFINE_TYPE(MapLoader::MapDescriptor);

extern Logger& getLogger();

using namespace UnityEngine;

#define toMapIfContains(text, contains, map) \
if (text.find(contains) != std::string::npos) map.push_back({gameObject, text})

namespace MapLoader
{
    using MapBehaviourRepresentation = struct MapBehaviourRepresentation{
            GameObject* first;
            std::string second;
        };

    using SerializeMap = std::vector<MapBehaviourRepresentation>; 
    using TeleporterMap = std::map<std::string, Teleporter*>;
    using TriggerMap = std::map<std::string, ObjectTrigger*>;

    void MapDescriptor::Initialize()
    {
        Array<UI::Text*>* textComponents = GetComponentsInChildren<UI::Text*>(true);
        
        SerializeMap teleporters = {};
        SerializeMap teleportPoints = {};
        SerializeMap objectTriggers = {};
        SerializeMap triggerPoints = {};
        SerializeMap tagZones = {};
        SerializeMap surfaces = {};
        SerializeMap roundEndActions = {};
        SerializeMap actionObjects = {};


        getLogger().info("Handling all Text Components");
        for (int i = 0; i < textComponents->Length(); i++)
        {
            UI::Text* component = textComponents->values[i];
            Il2CppString* serializedCS = component->get_text();
            std::string serialized = to_utf8(csstrtostr(serializedCS));

            if (serialized.find("{") == std::string::npos || serialized.find("}") == std::string::npos) continue;
            GameObject* gameObject = component->get_gameObject();

            while (serialized.find("}, {") != std::string::npos)
            {
                getLogger().error("serialized was: \n%s", serialized.c_str());
                int seperationpos = serialized.find("}, {");
                std::string subObj = serialized.substr(0, seperationpos + 1);
                serialized = serialized.erase(0, seperationpos + 3);
                getLogger().error("serialized is now: \n%s", serialized.c_str());
                getLogger().error("subObj: \n%s", subObj.c_str());

                toMapIfContains(subObj, "TeleporterName", teleporters);
                else toMapIfContains(subObj, "TeleportPoint", teleportPoints);
                else toMapIfContains(subObj, "ObjectTriggerName", objectTriggers);
                else toMapIfContains(subObj, "TriggeredBy", triggerPoints);
                else toMapIfContains(subObj, "TagZone", tagZones);
                else toMapIfContains(subObj, "SurfaceClimbSettings", surfaces);
                else toMapIfContains(subObj, "RoundEndActions", roundEndActions);
                else toMapIfContains(subObj, "RoundEndAction", actionObjects);
                else getLogger().error("Could not find object type for object with textcomponent:\n%s", subObj.c_str());
            }
            
            toMapIfContains(serialized, "TeleporterName", teleporters);
            else toMapIfContains(serialized, "TeleportPoint", teleportPoints);
            else toMapIfContains(serialized, "ObjectTriggerName", objectTriggers);
            else toMapIfContains(serialized, "TriggeredBy", triggerPoints);
            else toMapIfContains(serialized, "TagZone", tagZones);
            else toMapIfContains(serialized, "SurfaceClimbSettings", surfaces);
            else toMapIfContains(serialized, "RoundEndActions", roundEndActions);
            else toMapIfContains(serialized, "RoundEndAction", actionObjects);
            else getLogger().error("Could not find object type for object with textcomponent:\n%s", serialized.c_str());
        }

        // initialize teleporters
        TeleporterMap teleporterMap = {};
        static std::vector<Il2CppClass*> teleporterKlass = {classof(Teleporter*)};
        getLogger().info("Handling teleporters");
        for (auto t : teleporters)
        {
            Teleporter* tele = t.first->AddComponent<Teleporter*>();
            rapidjson::Document doc;
            doc.Parse(t.second.c_str());

            TouchType touchType = (TouchType)doc["TouchType"].GetInt();
            float delay = doc["Delay"].GetFloat();
            std::string teleporterName = doc["TeleporterName"].GetString();
            bool tagOnTeleport = doc["TagOnTeleport"].GetBool();

            teleporterMap[teleporterName] = tele;

            tele->touchType = touchType;
            tele->Delay = delay;
            tele->tagOnTeleport = tagOnTeleport;
            getLogger().info("\t%s", teleporterName.c_str());
        }    

        // initialize teleport points
        getLogger().info("Handling teleportPoints");
        for (auto t : teleportPoints)
        {
            Transform* transform = t.first->get_transform();
            rapidjson::Document doc;
            doc.Parse(t.second.c_str());

            std::string teleporter = doc["TeleportPoint"].GetString();

            teleporterMap[teleporter]->teleportPoints->Add(transform);
        }

        static Il2CppString* treeHomeTargetObjectName = il2cpp_utils::createcsstr("TreeHomeTargetObject", il2cpp_utils::StringType::Manual);
        GameObject* treeHomeTargetObject = GameObject::Find(treeHomeTargetObjectName);
        Transform* treeHomeTransform = treeHomeTargetObject->get_transform();

        // if there are 0 points, set type to treehouse
        for (auto t : teleporterMap)
        {
            int pointCount = t.second->teleportPoints->size;
            if (pointCount == 0) 
            {
                t.second->teleporterType = TeleporterType::Treehouse;
                t.second->teleportPoints->Add(treeHomeTransform);
            }
        }
        
        // initialize object triggers
        TriggerMap triggerMap = {};
        getLogger().info("Handling objectTriggers");
        for (auto t : objectTriggers)
        {
            ObjectTrigger* trigger = t.first->AddComponent<ObjectTrigger*>();
            rapidjson::Document doc;
            doc.Parse(t.second.c_str());

            std::string objectTriggerName = doc["ObjectTriggerName"].GetString();
            bool onlyTriggerOnce = doc["OnlyTriggerOnce"].GetBool();
            bool disableObject = doc["DisableObject"].GetBool();
            
            trigger->onlyTriggerOnce = onlyTriggerOnce;
            trigger->disableObject = disableObject;

            triggerMap[objectTriggerName] = trigger;

            getLogger().info("\t%s", objectTriggerName.c_str());
        }

        // initialize trigger points
        getLogger().info("Handling triggered");
        for (auto t : triggerPoints)
        {
            rapidjson::Document doc;
            doc.Parse(t.second.c_str());
            std::string trigger = doc["TriggeredBy"].GetString();
            
            triggerMap[trigger]->objectsToTrigger->Add(t.first);
            getLogger().info("\t%s", trigger.c_str());
        }

        // initialize tagZones
        getLogger().info("Handling TagZones");
        for (auto t : tagZones)
        {
            TagZone* zone = t.first->AddComponent<TagZone*>();
        }

        // initialize surfaces
        getLogger().info("Handling Surfaces");
        for (auto s : surfaces)
        {
            SurfaceClimbSettings* surface = s.first->AddComponent<SurfaceClimbSettings*>();
            rapidjson::Document doc;
            doc.Parse(s.second.c_str());
            
            bool unclimbable = doc["Unclimbable"].GetBool();
            float slipPercentage = doc["slipPercentage"].GetFloat();

            surface->unClimbable = unclimbable;
            surface->slipPercentage = slipPercentage;
            surface->Start();
        }
        
        getLogger().info("Handling Round end");
        std::vector<RoundEndActions*> endActions = {};
        for (auto r : roundEndActions)
        {
            RoundEndActions* roundEndAction = r.first->AddComponent<RoundEndActions*>();
            rapidjson::Document doc;
            doc.Parse(r.second.c_str());

            bool respawnOnRoundEnd = doc["RespawnOnRoundEnd"].GetBool();

            roundEndAction->respawnOnRoundEnd = respawnOnRoundEnd;
            endActions.push_back(roundEndAction);
        }

        for (auto a : actionObjects)
        {
            rapidjson::Document doc;
            doc.Parse(a.second.c_str());
            std::string action = doc["RoundEndAction"].GetString();
            for (auto endAction : endActions)
            {
                if (action == "Disable")
                {
                    endAction->objectsToDisable->Add(a.first);
                }
                else if (action == "Enable")
                {
                    endAction->objectsToEnable->Add(a.first);
                }
            }
        }
    }
}