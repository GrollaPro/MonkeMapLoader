#include "Behaviours/Teleporter.hpp"
#include "Behaviours/TagZone.hpp"
#include "Behaviours/Player.hpp"
#include "Behaviours/MapLoader.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-type-check.hpp"

DEFINE_CLASS(MapLoader::Teleporter);

namespace MapLoader
{
    void Teleporter::Awake()
    {
        static std::vector<const Il2CppClass*> transformKlass = {il2cpp_utils::GetClassFromName("UnityEngine", "Transform")};
        static Il2CppClass* listKlass = il2cpp_utils::GetClassFromName("System.Collections.Generic", "List`1");
        static Il2CppClass* transformListKlass = il2cpp_utils::MakeGeneric(listKlass, transformKlass);
        teleportPoints = *il2cpp_utils::New<List<Il2CppObject*>*>(transformListKlass);
        il2cpp_utils::RunMethod(teleportPoints, "Clear");

        joinGameOnTeleport = false;
        isTeleporting = false;
        tagOnTeleport = false;
    }

    void Teleporter::Trigger(Il2CppObject* collider)
    {
        if (isTeleporting)
            return;

        isTeleporting = true;
        TeleportPlayer();
    }

    void Teleporter::TeleportPlayer()
    {
        if (teleporterType == TeleporterType::Map)
        {
            static Il2CppString* spawnPointContainerName = il2cpp_utils::createcsstr("SpawnPointContainer", il2cpp_utils::StringType::Manual);
            Il2CppObject* spawnPointContainer = *il2cpp_utils::RunMethod("UnityEngine", "GameObject", "Find", spawnPointContainerName);
            Il2CppObject* containerTransform = *il2cpp_utils::RunMethod(spawnPointContainer, "get_transform");

            int childCount = *il2cpp_utils::RunMethod<int>(containerTransform, "get_childCount");
            il2cpp_utils::RunMethod(teleportPoints, "Clear");
            getLogger().info("Found %d children to teleport to!", childCount);
            for (int i = 0; i < childCount; i++)
            {
                Il2CppObject* child = *il2cpp_utils::RunMethod(containerTransform, "GetChild", i);
                il2cpp_utils::RunMethod(teleportPoints, "Add", child);
            }
        }

        if (teleportPoints->size == 0)
        {
            if (teleporterType == TeleporterType::Map)
            {
                getLogger().info("Even after trying to add spawnpoints, the array was empty");
                static Il2CppString* treeHomeTargetObjectName = il2cpp_utils::createcsstr("TreeHomeTargetObject", il2cpp_utils::StringType::Manual);
                Il2CppObject* treeHomeTargetObject = *il2cpp_utils::RunMethod("UnityEngine", "GameObject", "Find", treeHomeTargetObjectName);
                Il2CppObject* treeHomeTransform = *il2cpp_utils::RunMethod(treeHomeTargetObject, "get_transform");
                il2cpp_utils::RunMethod(teleportPoints, "Add", treeHomeTransform);
            }
            else 
            {
                getLogger().error("Teleporter point array was empty!");
                return;
            }
        }

        int index = teleportPoints->size > 1 ? rand() % teleportPoints->size : 0;
        Il2CppObject* dest = teleportPoints->items->values[index];
        
        // insttead of telepporting here

        if (tagOnTeleport) TagZone::TagLocalPlayer();
        if (joinGameOnTeleport) Loader::JoinGame();
        if (teleporterType == TeleporterType::Treehouse) Loader::ResetMapProperties();
        
        // teleport after the entire world has been shifted
        Player::TeleportPlayer(dest);

        isTeleporting = false;
    }
}