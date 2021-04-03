#include "Behaviours/PreviewOrb.hpp"

DEFINE_CLASS(MapLoader::PreviewOrb);

namespace MapLoader
{
    void PreviewOrb::Start()
    {
        instance = this;
        static std::vector<Il2CppClass*> rendererKlass = {il2cpp_utils::GetClassFromName("UnityEngine", "Renderer")};
        Il2CppObject* go = *il2cpp_utils::RunMethod(this, "get_gameObject");
        Il2CppObject* renderer = *il2cpp_utils::RunGenericMethod(go, "GetComponent", rendererKlass);
        il2cpp_utils::RunMethod(renderer, "set_enabled", false);
    }

    void PreviewOrb::ChangeOrb(Color color, Il2CppObject* texture)
    {
        static std::vector<Il2CppClass*> rendererKlass = {il2cpp_utils::GetClassFromName("UnityEngine", "Renderer")};
        Il2CppObject* go = *il2cpp_utils::RunMethod(instance, "get_gameObject");
        Il2CppObject* renderer = *il2cpp_utils::RunGenericMethod(go, "GetComponent", rendererKlass);
        il2cpp_utils::RunMethod(renderer, "set_enabled", true);

        Array<Il2CppObject*>* sharedMaterials = *il2cpp_utils::RunMethod<Array<Il2CppObject*>*>(renderer, "get_materials");

        il2cpp_utils::RunMethod(sharedMaterials->values[0], "set_color", color);
        il2cpp_utils::RunMethod(sharedMaterials->values[1], "set_mainTexture", texture);
        getLogger().info("End of Change orb");
    }

    void PreviewOrb::ChangeOrb(MapInfo& info)
    {
        getLogger().info("Changing orb");
        ChangeOrb(info.packageInfo->config.mapColor, info.LoadCubeMap());
    }
}