#pragma once
#include "ConstantMapData.h"

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:MapManager Class --- */
//
//  ★★★シングルトン★★★
//
// 【?】マップ管理
//      マップの読み込みや作成、マップオブジェクトの管理を行う
// 
// [注意] 
//      ここでいうマップオブジェクトは、GameObjectのことではなく、
//      マップ内に配置されるオブジェクトのことをいう
//
// ***************************************************************************************
class MapManager
{
private:
	// マップデータを保持
    std::unordered_map<std::string, Map::MapData> m_MapDataMap;

    // マップオブジェクトデータの管理
    std::unordered_map<std::string, Map::Map_ObjectAssetData> m_MapObjectAssetDataMap;

    // マップの地面データ管理
    std::unordered_map<std::string, Map::Map_GroundAssetData> m_MapGroundAssetDataMap;

public:
    MapManager();
    ~MapManager();

    bool CreateMap(const std::string& _filePath);               // マップの作成

    bool LoadMapData(const std::string& _filePath);             // 建物データの読み込み
    bool LoadMapObjectAssetData(const std::string& _filePath);       // マップオブジェクトデータの読み込み - 単体
    bool LoadMapObjectAssetDataAll(const std::string& _filePath);    // マップオブジェクトデータの読み込み - すべてのオブジェクト
    Map::Map_ObjectAssetData FindMapObjectAssetData(const std::string& _assetID);

private:
    // コピー禁止
    MapManager(const MapManager&) = delete;
    MapManager& operator=(const MapManager&) = delete;

    bool ExtractionMapData(const nlohmann::json& _json, Map::MapData& _outData);
    bool ExtractionMapObjectAssetData(const nlohmann::json& _json, Map::Map_ObjectAssetData& _outData);

    bool SpawnGround(const std::string& _groundAssetID);
    bool SpawnMapObject(const Map::MapObjectData& _mapObjectData);

    bool IsRegisteredMapObjectAssetData(const std::string& _assetID)const;
};

