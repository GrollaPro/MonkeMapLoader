#pragma once

#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/GameObject.hpp"

namespace MapLoader
{
    class GlobalData
    {
        public: 
            UnityEngine::Vector3 get_origin()
            {
                return origin;
            }

            void set_origin(UnityEngine::Vector3 value)
            {
                if (
                    origin.x == value.x &&
                    origin.y == value.y &&
                    origin.z == value.z
                )
                return;

                origin = value;
                customOrigin = value;
                customOrigin.y += 54.0f;
            }

            UnityEngine::Vector3 treeOrigin = {0.0f, 0.0f, 0.0f};

            UnityEngine::GameObject* bigTreeTeleportToMap = nullptr;
            
            UnityEngine::GameObject* bigTreePoint = nullptr;

            UnityEngine::GameObject* fallEmergencyTeleport = nullptr;

            bool isLegacyMap = false;
        private:
            UnityEngine::Vector3 origin = {0.0f, 0.0f, 0.0f};
            UnityEngine::Vector3 customOrigin = {0.0f, 54.0f, 0.0f};
    };
}