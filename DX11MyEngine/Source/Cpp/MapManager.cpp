#include "pch.h"
#include "MapManager.h"

using namespace Map;

//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//*----------------------------------------------------------------------------------------
MapManager::MapManager()
{
}

//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
MapManager::~MapManager()
{
}

//*---------------------------------------------------------------------------------------
//*【?】マップの作成
//*
//* [引数]
//* _filePath : 作成するマップのファイルパス
//*
//* [返値]
//* true : 成功
//* false : 失敗
//*----------------------------------------------------------------------------------------
bool MapManager::CreateMap(const std::string& _filePath)
{
    const auto mapDataIt = m_MapDataMap.find(_filePath);

    // 登録されていないなら読み込む
    if (mapDataIt == m_MapDataMap.end())
    {
        if (!LoadMapData(_filePath))
        {
            // 読み込み失敗
            Tool::ErrorMessage(L"マップの作成に失敗しました", L"MapManager");
            assert(false);
            return false;
        }
    }

    const MapData& mapData = mapDataIt->second;

    // 地面の生成
    SpawnGround(mapData.groundAssetTag);

    // マップオブジェクトの生成
    for (const auto& mapObj : mapData.objects)
    {
        if (!SpawnMapObject(mapObj))
        {
            // 読み込み失敗
            Tool::ErrorMessage(L"マップオブジェクトの生成に失敗しました", L"MapManager");
            assert(false);
            return false;
        }
    }


    return true;
}


//*---------------------------------------------------------------------------------------
//*【?】マップデータの読み込み
//*
//* [引数]
//* _filePath : マップデータJsonのファイルパス
//*
//* [返値]
//* true : 成功
//* false : 失敗
//*----------------------------------------------------------------------------------------
bool MapManager::LoadMapData(const std::string& _filePath)
{
    using json = nlohmann::json;
    std::ifstream ifs(_filePath);
    if (!ifs.is_open())
    {
        Tool::ErrorMessage(L"マップ定義ファイルを開けませんでした", L"MapManager");
        assert(false);
        return false;
    }

    json j;
    ifs >> j;
    Map::MapData extractionMapData; // 抽出データ

    // 既に作成済みか確かめる
    // mapNameをキーにしているので、それを使う
    //std::string mapName = j.at("mapName").get<std::string>();
    //const auto mapDataIt = m_MapDataMap.find(mapName);
    //if (mapDataIt != m_MapDataMap.end())
    //{
    //    Tool::ErrorMessage(L"既にマップデータが登録されています", L"MapManager");
    //    return true;
    //}

    // マップデータの読み込み
    if (!ExtractionMapData(j, extractionMapData))
    {
        Tool::ErrorMessage(L"マップの読み込みに失敗しました", L"MapManager");
        assert(false);
        return false;
    }

    // ファイルパスをキーにしてマップデータを保存
    m_MapDataMap[_filePath] = extractionMapData;

    return true;
}

//*---------------------------------------------------------------------------------------
//*【?】マップデータの抽出
//*
//* [引数]
//* _json   : 読み込んだjson
//*_outData : 出力データ
//*
//* [返値]
//* true : 成功
//* false : 失敗
//*----------------------------------------------------------------------------------------
bool MapManager::ExtractionMapData(const nlohmann::json& _json, Map::MapData& _outData)
{
    _outData.mapName = _json.at("mapName").get<std::string>();
    _outData.groundAssetTag = _json.at("groundAssetTag").get<std::string>();


    // マップオブジェクトの配列を取得
    for (const auto& obj : _json.at("objects"))
    {
        MapObjectData data;

        data.name = obj.at("name").get<std::string>();
        data.modelPath = obj.at("modelPath").get<std::string>();

        // マップオブジェクトのタイプ
        std::string mapObjectType = _json.at("mapObjectType").get<std::string>();  
        const auto mapObjectTypeIt = g_MapObjectTypeMap.find(mapObjectType);
        // 未定義
        if (mapObjectTypeIt == g_MapObjectTypeMap.end()) {
            return false;
        }
        data.mapObjectType = mapObjectTypeIt->second;


        // 位置/回転/拡縮を取得
        Tool::Json::LoadVEC3Data(obj, "position", data.transform.position);
        Tool::Json::LoadVEC3Data(obj, "rotation", data.transform.rotation);
        Tool::Json::LoadVEC3Data(obj, "scale", data.transform.scale);

        // 掴めるか
        data.isGrabbable = obj.at("isGrabbable").get<bool>();

        // 破壊できるか
        data.isDestruction = obj.at("isDestruction").get<bool>();

        // 体力
        data.health = obj.at("health").get<float>();

        // vectorに追加
        _outData.objects.push_back(data);
    }

    return true;
}


//*---------------------------------------------------------------------------------------
//*【?】地面の生成
//*
//* [引数]
//* _groundAssetTag   : 地面のタグ
//*
//* [返値]
//* true : 成功
//* false : 失敗
//*----------------------------------------------------------------------------------------
bool MapManager::SpawnGround(const std::string& _groundAssetTag)
{

}

//*---------------------------------------------------------------------------------------
//*【?】マップオブジェクトの生成
//*
//* [引数]
//* _assetTag   : タグ（モデルパス）
//* _data       : 生成データ
//*
//* [返値]
//* true : 成功
//* false : 失敗
//*----------------------------------------------------------------------------------------
bool MapManager::SpawnMapObject(const Map::MapObjectData& _data)
{
    _data.health;
    _data.isDestruction;
    _data.isGrabbable;
    _data.mapObjectType;
    _data.modelPath;
    _data.name;
    _data.transform.position;
    _data.transform.rotation;
    _data.transform.scale;
}
