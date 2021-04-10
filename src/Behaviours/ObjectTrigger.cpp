#include "Behaviours/ObjectTrigger.hpp"

DEFINE_TYPE(MapLoader::ObjectTrigger);

extern Logger& getLogger();
using namespace UnityEngine;

namespace MapLoader
{
    void ObjectTrigger::ctor()
    {
        getLogger().info("trigger ctor");
        objectsToTrigger = *il2cpp_utils::New<List<GameObject*>*>();
    }

    void ObjectTrigger::OnEnable()
    {
        getLogger().info("trigger onEnable");
        for (int i = 0; i < objectsToTrigger->size; i++)
        {
            GameObject* objectToTrigger = objectsToTrigger->get_Item(i);

            objectToTrigger->SetActive(!disableObject);
            objectToTrigger->SetActive(disableObject);
        }

        triggered = false;
    }

    void ObjectTrigger::Trigger(Collider* collider)
    {
        if (triggered && onlyTriggerOnce)
            return;
        
        for (int i = 0; i < objectsToTrigger->size; i++)
        {
            GameObject* objectToTrigger = objectsToTrigger->get_Item(i);
            objectToTrigger->SetActive(disableObject);
            objectToTrigger->SetActive(!disableObject);
        }
        

        triggered = true;
    }
}