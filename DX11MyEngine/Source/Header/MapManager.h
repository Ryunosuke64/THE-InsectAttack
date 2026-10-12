#pragma once
#include "ConstantMapData.h"

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:MapManager Class --- */
//
//  ★★★シングルトン★★★
//
// 【?】建物の管理
//
// ***************************************************************************************
class MapManager
{
private:
	// マップデータを保持
    std::unordered_map<std::string, Map::MapData> m_MapDataMap;


public:
    MapManager();
    ~MapManager();
    bool LoadMapData(const std::string& _filePath);  // 建物データの読み込み
    bool CreateMap(const std::string& _filePath);
    //bool CleanupMap();

private:
    // コピー禁止
    MapManager(const MapManager&) = delete;
    MapManager& operator=(const MapManager&) = delete;

    bool ExtractionMapData(const nlohmann::json& _json, Map::MapData& _outData);
    
    bool SpawnGround(const std::string& _groundAssetTag);
    bool SpawnMapObject(const Map::MapObjectData& _data);
};

