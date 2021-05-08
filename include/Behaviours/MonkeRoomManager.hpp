#pragma once

#include "custom-types/shared/macros.hpp"

#include "UnityEngine/MonoBehaviour.hpp"
#include "Photon/Realtime/RoomInfo.hpp"
#include "Models/RoomRegionInfo.hpp"

#include <string>
#include <vector>
#include <map>

struct RoomInfoData {
    std::string name = "";
    std::string gameMode = "";
    std::string region = "";
    int playerCount = 0;

    RoomInfoData() {};
    RoomInfoData(Photon::Realtime::RoomInfo* roomInfo);
    RoomInfoData(Photon::Realtime::RoomInfo* roomInfo, std::string region);
};

using MapToCount = std::map<std::string, int>;
using RegionToMap = std::map<std::string, MapToCount>;

DECLARE_CLASS_CODEGEN(MapLoader, MonkeRoomManager, Il2CppObject,
        DECLARE_CTOR(ctor);
        DECLARE_METHOD(static MonkeRoomManager*, get_instance);
        DECLARE_METHOD(void, OnConnectedToMaster);        
        DECLARE_METHOD(void, OnRoomListUpdate, List<Photon::Realtime::RoomInfo*>*);        
        //DECLARE_INSTANCE_FIELD(List<RoomRegionInfo*>*, roomListCache);

        public:
            static int PlayersOnMap(std::string mapName);
            static void ForceRegionIfExists(std::string mapName);
        private:
            static inline std::string forcedRegion = "";
            static inline std::vector<std::string> checkedRegions = {};
            static inline std::vector<RoomInfoData> roomListCache = {};
            static inline RegionToMap MapsInRegion = {};

    REGISTER_FUNCTION(
        REGISTER_METHOD(ctor);
        REGISTER_METHOD(get_instance);
        REGISTER_METHOD(OnConnectedToMaster);
        REGISTER_METHOD(OnRoomListUpdate);
        //REGISTER_FIELD(roomListCache);
    )
)