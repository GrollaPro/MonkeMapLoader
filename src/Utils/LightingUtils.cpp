#include "Utils/LightingUtils.hpp"

#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "UnityEngine/TextureFormat.hpp"
#include "UnityEngine/Texture.hpp"
#include "UnityEngine/RenderTexture.hpp"
#include "UnityEngine/RenderTextureFormat.hpp"
#include "UnityEngine/RenderTextureReadWrite.hpp"
#include "UnityEngine/Graphics.hpp"
#include "UnityEngine/Rect.hpp"
#include "UnityEngine/RenderTextureDescriptor.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/Shader.hpp"

static Il2CppString* _OcclusionMap = nullptr;
static Il2CppString* _OcclusionStrength = nullptr;
static Il2CppString* _adjustedByLoader = nullptr;

using namespace UnityEngine;

namespace LightingUtils
{
    void SetLightingStrength(Material* material, float strength)
    {
        // strength 1 means unchanged, 
        // 0 means no lighting (probably black lol)

        if (!material) return;
        if (!_OcclusionMap) _OcclusionMap = il2cpp_utils::createcsstr("_OcclusionMap", il2cpp_utils::StringType::Manual);

        if (!material->HasProperty(_OcclusionMap)) return;
        Texture* map = material->GetTexture(_OcclusionMap);
        
        if (!_adjustedByLoader) _adjustedByLoader = il2cpp_utils::createcsstr("_adjustedByLoader", il2cpp_utils::StringType::Manual);

        if (map && map->get_name()->Contains(_adjustedByLoader)) return;
        else if (map && SetExistingLightingStrength(material, strength)) return; 
        else if (map)
        {
            if (!_OcclusionStrength) _OcclusionStrength = il2cpp_utils::createcsstr("_OcclusionStrength", il2cpp_utils::StringType::Manual);
            material->SetFloat(_OcclusionStrength, 1.0f);
        }

        Texture2D* texture = Texture2D::New_ctor(1, 1, TextureFormat::ARGB32, false);

        Il2CppString* matNameCS = material->get_name();
        std::string matName = to_utf8(csstrtostr(matNameCS));
        std::string newMapName = matName + "_OCC_adjustedByLoader";
        texture->set_name(il2cpp_utils::createcsstr(newMapName)); 

        texture->SetPixel(0, 0, Color(strength, strength, strength, 1.0f));
        texture->Apply();

        material->SetTexture(_OcclusionMap, texture);
    }

    bool SetExistingLightingStrength(Material* material, float strength)
    {
        if (!material) return false;
        if (!_OcclusionMap) _OcclusionMap = il2cpp_utils::createcsstr("_OcclusionMap", il2cpp_utils::StringType::Manual);

        if (!material->HasProperty(_OcclusionMap)) return false;
        
        Texture2D* map = (Texture2D*)material->GetTexture(_OcclusionMap);
        if (map && map->get_name()->Contains(_adjustedByLoader)) return false;
        if (!map) return false;

        Texture2D* duplicatedTexture = Texture2D::New_ctor(map->get_width(), map->get_height());;

        Il2CppString* mapNameCS = map->get_name();
        std::string mapName = to_utf8(csstrtostr(mapNameCS));
        std::string newMapName = mapName + "_adjustedByLoader";
        duplicatedTexture->set_name(il2cpp_utils::createcsstr(newMapName));
        
        auto* GetPixels = il2cpp_utils::FindMethodUnsafe(map, "GetPixels", 5);
        Array<Color>* pixels = *il2cpp_utils::RunMethod<Array<Color>*>(map, GetPixels, 0, 0, duplicatedTexture->get_width(), duplicatedTexture->get_height(), 0);
        
        if (pixels)
        {
            for (int i = 0; i < pixels->Length(); i++)
            {
                pixels->values[i] = pixels->values[i].RGBMultiplied(strength);
                
                /*(Color){
                        StrengthFromExistingColorFloat(pixels->values[i].r, strength),
                        StrengthFromExistingColorFloat(pixels->values[i].g, strength),
                        StrengthFromExistingColorFloat(pixels->values[i].b, strength),
                        1.0f
                    };
                    */
            }

            duplicatedTexture->SetPixels(0, 0, map->get_width(), map->get_height(), pixels);
            duplicatedTexture->Apply();

            material->SetTexture(_OcclusionMap, duplicatedTexture);
        }
        else return false;
        return true;
    }

    float StrengthFromExistingColorFloat(float colorValue, float strength)
    {
        return colorValue * strength;
    }
}