#include "pch.h"
#include "MapManager.h"
#include "MapObjectFactory.h"

using namespace Map;
using namespace UtilityData;

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
    // 登録されていないなら読み込む
    if (m_MapDataMap.find(_filePath) == m_MapDataMap.end())
    {
        if (!LoadMapData(_filePath))
        {
            // 読み込み失敗
            Tool::ErrorMessage(L"マップの作成に失敗しました", L"MapManager");
            assert(false);
            return false;
        }
    }
    
    const MapData& mapData = m_MapDataMap[_filePath];

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

        data.assetID = obj.at("assetID").get<std::string>();

        // 位置/回転/拡縮を取得
        Tool::Json::LoadVEC3Data(obj, "position", data.transform.position);
        Tool::Json::LoadVEC3Data(obj, "rotation", data.transform.rotation);
        Tool::Json::LoadVEC3Data(obj, "scale", data.transform.scale);

        // vectorに追加
        _outData.objects.push_back(data);
    }

    return true;
}

//*---------------------------------------------------------------------------------------
//*【?】マップオブジェクトデータの読み込み
//*     マップの作成前にこれを呼ばないと、オブジェクトのデータが参照できない 
//*
//* [引数]
//* _filePath : マップオブジェクトデータJsonのファイルパス
//*
//* [返値]
//* true : 成功
//* false : 失敗
//*----------------------------------------------------------------------------------------
bool MapManager::LoadMapObjectAssetData(const std::string& _filePath)
{
    using json = nlohmann::json;
    std::ifstream ifs(_filePath);
    if (!ifs.is_open())
    {
        Tool::ErrorMessage(L"マップオブジェクト定義ファイルを開けませんでした", L"MapManager");
        assert(false);
        return false;
    }
    
    json j;
    ifs >> j;
    Map::Map_ObjectAssetData extractionMapObjectData; // 抽出データ

    // アセットIDを抽出し、既に登録されているか確認
    extractionMapObjectData.assetID = j.at("assetID").get<std::string>();
    
    // 登録されていれば、再度抽出はせず終了
    if (IsRegisteredMapObjectAssetData(extractionMapObjectData.assetID))
    {
        return true;
    }

    // マップオブジェクトデータの読み込み
    if (!ExtractionMapObjectAssetData(j, extractionMapObjectData))
    {
        Tool::ErrorMessage(L"マップオブジェクトの読み込みに失敗しました", L"MapManager");
        assert(false);
        return false;
    }

    // アセットIDをキーにしてマップデータを保存
    m_MapObjectAssetDataMap[extractionMapObjectData.assetID] = extractionMapObjectData;

    return true;
}


//*---------------------------------------------------------------------------------------
//*【?】マップオブジェクトデータの読み込み
//*     すべてのマップオブジェクトデータを読み込む
//*     指定ファイルにすべてのオブジェクト情報が書かれている必要がある 
//*     ファイル内の形式も単一のものとは異なるので注意 
//*     マップの作成前にこれを呼ばないと、オブジェクトのデータが参照できない 
//*
//* [引数]
//* _filePath : マップオブジェクトデータJsonのファイルパス
//*
//* [返値]
//* true : 成功
//* false : 失敗
//*----------------------------------------------------------------------------------------
bool MapManager::LoadMapObjectAssetDataAll(const std::string& _filePath)
{
    using json = nlohmann::json;
    std::ifstream ifs(_filePath);
    if (!ifs.is_open())
    {
        Tool::ErrorMessage(L"マップオブジェクト定義ファイルを開けませんでした", L"MapManager");
        assert(false);
        return false;
    }

    json j;
    ifs >> j;


    // すべてのオブジェクトデータを取り出す
    for (const auto& obj : j.at("objects"))
    {
        Map::Map_ObjectAssetData extractionMapObjectData; // 抽出データ

        // アセットIDを抽出し、既に登録されているか確認
        extractionMapObjectData.assetID = obj.at("assetID").get<std::string>();
        // 登録されていれば、再度抽出はせず終了
        if (IsRegisteredMapObjectAssetData(extractionMapObjectData.assetID))
        {
            return true;
        }

        // マップオブジェクトデータの読み込み
        if (!ExtractionMapObjectAssetData(obj, extractionMapObjectData))
        {
            Tool::ErrorMessage(L"マップオブジェクトの読み込みに失敗しました", L"MapManager");
            assert(false);
            return false;
        }

        // アセットIDをキーにしてマップデータを保存
        m_MapObjectAssetDataMap[extractionMapObjectData.assetID] = extractionMapObjectData;
    }

    return true;
}

//*---------------------------------------------------------------------------------------
//*【?】物理プリセットの読み込み
//*
//* [引数]
//* _filePath : Jsonのファイルパス
//*
//* [返値]
//* true : 成功
//* false : 失敗
//*----------------------------------------------------------------------------------------
bool MapManager::
LoadPhysicsDataPreset(const std::string& _filePath)
{
    using json = nlohmann::json;
    std::ifstream ifs(_filePath);
    if (!ifs.is_open())
    {
        Tool::ErrorMessage(L"物理プリセット定義ファイルを開けませんでした", L"MapManager");
        assert(false);
        return false;
    }

    json j;
    ifs >> j;

    // すべてのオブジェクトデータを取り出す
    for (const auto& obj : j.at("physics"))
    {
        Map::MO_PhysicsData extractionPhysicsData;

        // プリセットIDを抽出し、既に登録されているか確認
        extractionPhysicsData.presetID = obj.at("presetID").get<std::string>();
        
        // 物理データプリセットの抽出
        if (!ExtractionPhysicsDataPreset(obj, extractionPhysicsData))
        {
            Tool::ErrorMessage(L"物理プリセットの読み込みに失敗しました", L"MapManager");
            assert(false);
            return false;
        }

        // プリセットIDをキーにしてデータを保存
        m_PhysicsDataPresetMap[extractionPhysicsData.presetID] = extractionPhysicsData;
    }

    return true;
}



//*---------------------------------------------------------------------------------------
//*【?】マップオブジェクトデータの抽出
//*
//* [引数]
//* _json   : 読み込んだjson
//*_outData : 出力データ
//*
//* [返値]
//* true : 成功
//* false : 失敗
//*----------------------------------------------------------------------------------------
bool MapManager::ExtractionMapObjectAssetData(const nlohmann::json& _json, Map::Map_ObjectAssetData& _outData)
{
    //
    // assetIDは呼び出し元ですでに抽出済みなのでここではしない
    //

    // モデルパス
    _outData.modelPath = _json.at("modelPath").get<std::string>();

    // 物理プリセットID
    _outData.physicsPresetID = _json.at("physicsPresetID").get<std::string>();

    // マップオブジェクトのタイプ
    std::string mapObjectType = _json.at("mapObjectType").get<std::string>();
    const auto mapObjectTypeIt = g_MapObjectTypeMap.find(mapObjectType);
    // 未定義
    if (mapObjectTypeIt == g_MapObjectTypeMap.end()) {
        return false;
    }
    _outData.objectType = mapObjectTypeIt->second;

    // マテリアルデータを取得
    for (const auto& mat : _json.at("materials"))
    {
        SetupMaterialInfo setupMaterial;

        // マテリアルスロット
        setupMaterial.Index = mat.at("slot").get<int>();
        // マテリアルのタグ
        std::string materialTag = mat.at("tag").get<std::string>();

        // マテリアルのタグから実際のマテリアルデータを取得
        setupMaterial.pMaterialData = Master::m_pResourceManager->FindMaterial(materialTag);

        // vectorに追加
        _outData.materials.push_back(setupMaterial);
    }

    // 掴めるか
    _outData.isGrabbable = _json.at("isGrabbable").get<bool>();

    // 破壊できるか
    _outData.isDestruction = _json.at("isDestruction").get<bool>();

    // 体力
    _outData.health = _json.at("health").get<float>();



    return true;
}

//*---------------------------------------------------------------------------------------
//*【?】物理プリセットの抽出
//*
//* [引数]
//* _json   : 読み込んだjson
//*_outData : 出力データ
//*
//* [返値]
//* true : 成功
//* false : 失敗
//*----------------------------------------------------------------------------------------
bool MapManager::
ExtractionPhysicsDataPreset(const nlohmann::json& _json, Map::MO_PhysicsData& _outData)
{
    // *******************************************
    // リジッドボディ関連
    // *******************************************
    {
        const nlohmann::json rbJson = _json["rigidBody"];
        MO_RigidBodyData rigidBodyData;
        rigidBodyData.mass              = rbJson.value("mass", 0.0f);
        rigidBodyData.restitution       = rbJson.value("restitution", 0.0f);
        rigidBodyData.friction          = rbJson.value("friction", 0.0f);
        rigidBodyData.gravityScale      = rbJson.value("gravityScale", 0.0f);
        rigidBodyData.linearDamping     = rbJson.value("linearDamping", 0.0f);
        rigidBodyData.angularDamping    = rbJson.value("angularDamping", 0.0f);
        Tool::Json::LoadVEC3Data(rbJson, "linearFactor", rigidBodyData.linearFactor);
        Tool::Json::LoadVEC3Data(rbJson, "angularFactor", rigidBodyData.angularFactor);
        _outData.rigidBody = rigidBodyData;
    }

    // *******************************************
    // コリジョンマスク関連
    // *******************************************
    {
        // 自身の衝突カテゴリ
        //
        std::string myCollisionCategoryStr = _json.value("collisionCategory", "");
        const auto myCollisionCategoryIt = g_CollisionCategoryMap.find(myCollisionCategoryStr);;
        // 未定義衝突カテゴリ
        if (myCollisionCategoryIt == g_CollisionCategoryMap.end())
        {
            return false;
        }
        _outData._myCollisionCategory = (myCollisionCategoryIt->second);

        //
        // 衝突マスク
        //
        _outData._collisionMask = 0;
        if (_json.contains("collisionMask") &&
            _json["collisionMask"].is_array())
        {
            // 配列を回してOR演算
            for (const auto& maskJson : _json["collisionMask"])
            {
                const std::string maskName = maskJson.get<std::string>();
                const auto categoryIt = g_CollisionCategoryMap.find(maskName);

                // 未定義衝突カテゴリ
                if (categoryIt == g_CollisionCategoryMap.end())
                {
                    return false;
                }

                _outData._collisionMask |= UINT_CAST(categoryIt->second);
            }
        }
    }

    // *******************************************
    // コライダー関連
    // *******************************************
    {
        // コライダーデータを取得
        for (const auto& coll : _json.at("colliders"))
        {
            MO_ColliderData collider;

            // コライダーのタイプ
            std::string colliderTypeStr = coll.at("type").get<std::string>();
            const auto colliderTypeIt = PhysicsData::g_ColliderTypeMap.find(colliderTypeStr);
            // 未定義
            if (colliderTypeIt == PhysicsData::g_ColliderTypeMap.end()) {
                return false;
            }
            collider.type = colliderTypeIt->second;

            // パラメータ
            collider.height = coll.value("height", 0.0f);
            collider.radius = coll.value("radius", 0.0f);
            Tool::Json::LoadVEC3Data(coll, "center", collider.center);
            Tool::Json::LoadVEC3Data(coll, "size", collider.size);

            // 材質
            std::string surfaceStr = coll.value("surfaceType", "DEFAULT");
            collider.surfaceType = g_SurfaceTypeMap[surfaceStr];

            // vectorに追加
            _outData.colliders.push_back(collider);
        }
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
    return true;
}

//*---------------------------------------------------------------------------------------
//*【?】マップオブジェクトの生成
//*
//* [引数]
//* _mapObjectData   : マップオブジェクトデータ
//*
//* [返値]
//* true : 成功
//* false : 失敗
//*----------------------------------------------------------------------------------------
bool MapManager::SpawnMapObject(const Map::MapObjectData& _mapObjectData)
{
    return MapObjectFactory::CreateMapObject(_mapObjectData);
}

//*---------------------------------------------------------------------------------------
//*【?】マップオブジェクトがあるかどうか探す
//*
//* [引数]
//* _assetID   : マップオブジェクトに紐付けされているキー
//*
//* [返値]
//* true : 成功
//* false : 失敗
//*----------------------------------------------------------------------------------------
bool MapManager::IsRegisteredMapObjectAssetData(const std::string& _assetID)const
{
    const auto mapObjectDataIt = m_MapObjectAssetDataMap.find(_assetID);

    // 登録されている
    if (mapObjectDataIt != m_MapObjectAssetDataMap.end())
    {
        return true;
    }

    // されていない
    return false;
}

//*---------------------------------------------------------------------------------------
//*【?】物理プリセットがあるかどうか探す
//*
//* [引数]
//* _assetID   : 物理プリセットで紐付けされているキー
//*
//* [返値]
//* true : 成功
//* false : 失敗
//*----------------------------------------------------------------------------------------
bool MapManager::IsRegisteredPhysicsDataPreset(const std::string& _presetID)const
{
    const auto physicsDataPresetIt = m_PhysicsDataPresetMap.find(_presetID);

    // 登録されている
    if (physicsDataPresetIt != m_PhysicsDataPresetMap.end())
    {
        return true;
    }

    // されていない
    return false;
}

//*---------------------------------------------------------------------------------------
//*【?】アセットIDからマップオブジェクトの定義データを探す
//*
//* [引数]
//* _assetID   : マップオブジェクトに紐付けされているキー
//*
//* [返値]
//* true : 成功
//* false : 失敗
//*----------------------------------------------------------------------------------------
Map::Map_ObjectAssetData MapManager::
FindMapObjectAssetData(const std::string& _assetID)
{
    if (!IsRegisteredMapObjectAssetData(_assetID))
    {
        std::wstring errorMessage = Tool::StringToWstring(_assetID) + L" アセットが登録されていません";
        Tool::ErrorMessage(errorMessage, L"MapManager");
        assert(false);
        return {};
    }

    return m_MapObjectAssetDataMap[_assetID];
}

//*---------------------------------------------------------------------------------------
//*【?】アセットIDからマップオブジェクトの定義データを探す
//*
//* [引数]
//* _assetID   : マップオブジェクトに紐付けされているキー
//*
//* [返値]
//* true : 成功
//* false : 失敗
//*----------------------------------------------------------------------------------------
Map::MO_PhysicsData MapManager::
FindPhysicsDataPreset(const std::string& _presetID)
{
    // 物理プリセットが登録されているか確かめる
    if (!IsRegisteredPhysicsDataPreset(_presetID))
    {
        std::wstring errorMessage = Tool::StringToWstring(_presetID) + L" 物理プリセットが登録されていません";
        Tool::ErrorMessage(errorMessage, L"MapManager");
        assert(false);
        return {};
    }

    return m_PhysicsDataPresetMap[_presetID];
}
