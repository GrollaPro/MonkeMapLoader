#pragma once

#include "UnityEngine/Material.hpp"
#include "UnityEngine/Texture2D.hpp"

namespace LightingUtils
{
    void SetLightingStrength(UnityEngine::Material* material, float strength);
    bool SetExistingLightingStrength(UnityEngine::Material* material, float strength);
    float StrengthFromExistingColorFloat(float colorValue, float strength);
}