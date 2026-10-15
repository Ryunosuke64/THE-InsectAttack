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

    /* 物理プリセット */
    std::unordered_map<std::string, Map::MO_PhysicsData> m_PhysicsDataPresetMap;

    /* 建物の破片データ */
    std::unordered_map<std::string, Map::BuildingFragmentDataSet> m_BuildingFragmentDataMap;

public:
    MapManager();
    ~MapManager();

    bool CreateMap(const std::string& _filePath);               // マップの作成

    bool LoadMapData(const std::string& _filePath);                  // 建物データの読み込み
    bool LoadMapObjectAssetData(const std::string& _filePath);       // マップオブジェクトデータの読み込み - 単体
    bool LoadMapObjectAssetDataAll(const std::string& _filePath);    // マップオブジェクトデータの読み込み - すべてのオブジェクト
    bool LoadPhysicsDataPreset(const std::string& _filePath);        // 物理プリセットの読み込み
    bool LoadBuildingFragmentData(const std::string& _filePath);     // 建物の破片データの読み込み

    Map::Map_ObjectAssetData FindMapObjectAssetData(const std::string& _assetID);
    Map::MO_PhysicsData FindPhysicsDataPreset(const std::string& _presetID);
    Map::BuildingFragmentDataSet FindBuildingFragmentDataSet(const std::string& _presetID);

private:
    // コピー禁止
    MapManager(const MapManager&) = delete;
    MapManager& operator=(const MapManager&) = delete;

    // マップデータ抽出
    bool ExtractionMapData(const nlohmann::json& _json, Map::MapData& _outData);
    
    // マップオブジェクトアセット抽出
    bool ExtractionMapObjectAssetData(const nlohmann::json& _json, Map::Map_ObjectAssetData& _outData);
    
    // 物理プリセットの抽出
    bool ExtractionPhysicsDataPreset(const nlohmann::json& _json, Map::MO_PhysicsData& _outData);
        
    // 建物の破片データの抽出
    bool ExtractionBuildingFragmentData(const nlohmann::json& _json, Map::BuildingFragmentDataSet& _outData);

    // マップオブジェクトアセットが登録されているか確認
    bool IsRegisteredMapObjectAssetData(const std::string& _assetID)const;
    bool IsRegisteredPhysicsDataPreset(const std::string& _presetID)const;
    bool IsRegisteredBuildingFragmentData(const std::string& _presetID)const;
    
    // 地面の生成
    bool SpawnGround(const std::string& _groundAssetID);
    
    // マップオブジェクトの生成
    bool SpawnMapObject(const Map::MapObjectData& _mapObjectData);

    // 建物の破片モデルの生成
    bool SpawnBuildingDebris(const std::string& _assetID);
};

