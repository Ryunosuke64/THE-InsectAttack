#pragma once
#include "MeshFactory.h"
#include "Component_Collider.h"

namespace Map
{
    /* マップオジェクトの種類 */
    enum class MAP_OBJECT_TYPE
    {
        BUILDING,           // 建物
        EXPLOSION_BUILDING, // 爆発する建物
        PROP,               // 岩・車・柵など
        DECORATION,         // 草・花などの装飾物
    };


    // マップオブジェクトの種類を文字列からenumに変換するmap
    static std::map<std::string, MAP_OBJECT_TYPE> g_MapObjectTypeMap =
    {
        {"BUILDING",           MAP_OBJECT_TYPE::BUILDING},
        {"EXPLOSION_BUILDING", MAP_OBJECT_TYPE::EXPLOSION_BUILDING },
        {"PROP",               MAP_OBJECT_TYPE::PROP},
        {"DECORATION",         MAP_OBJECT_TYPE::DECORATION},
    };

    ///* マップの保存する情報*/
    //struct BUILDING_SAVE_DATA
    //{
    //    BUILDING_TYPE_ID _id;	// オブジェクトの種類
    //    VECTOR3::VEC3 _pos;		// 座標
    //    VECTOR3::VEC3 _rotate;	// 回転
    //    VECTOR3::VEC3 _scale;	// スケール
    //};

    /* マップの種類 */
    enum class MAP_TYPE_ID
    {
        TOWN_1,	// 街 1

        NUM,
    };

    /* 建物の状態 */
    enum BUILDING_STATE
    {
        BUILDING_STATE_IDLE,			// 通常
        BUILDING_STATE_CLLAPSE_IN,		// 倒壊始め
        BUILDING_STATE_CLLAPSE_NOW,		// 倒壊中...
        BUILDING_STATE_CLLAPSE_END,		// 倒壊終了
        BUILDING_STATE_FALL,			// 落下
        BUILDING_STATE_END,				// 終了

        BUILDING_STATE_HIT,				// 被弾
    };

    /* 建物のデータ */
    struct BuildingData
    {
		std::string _tag;       // タグ
		float _collapseTimeMin;	// 倒壊にかかる時間の最小値
		float _collapseTimeMax; // 倒壊にかかる時間の最大値
		float _collapseSpeed;	// 倒壊スピード
		float _collapseEndTime;	// 倒壊終了から落下までの時間
		float _fallSpeed;		// 落下スピード
		float _shakeDuration;	// カメラシェイクの持続時間
		float _shakeLength;		// カメラシェイクの強さ
		float _shakeMaxRange;	// カメラシェイクの最大範囲

    };

    //************************
    // [MO] はMapObjectの略
    //************************

    /* コライダーデータ */
    struct MO_ColliderData
    {
        PhysicsData::COLLIDER_TYPE type =                   // コライダーの種類
            PhysicsData::COLLIDER_TYPE::NONE;
        VECTOR3::VEC3 center = VECTOR3::VEC3(0.0f);         // 中心位置
        VECTOR3::VEC3 size = VECTOR3::VEC3(0.0f);           // サイズ
        float radius = 0.0f;                                // 半径
        float height = 0.0f;                                // 高さ
        SURFACE_TYPE surfaceType = SURFACE_TYPE::DEFAULT;   // 材質

    };

    /* リジッドボディデータ */
    struct MO_RigidBodyData
    {
        float mass = 0.0f;              // 質量
        float restitution = 0.0f;       // 反発係数
        float friction = 0.5f;          // 摩擦係数
        float gravityScale = 1.0f;      // 重力倍率
        float linearDamping = 0.0f;     // 移動減衰
        float angularDamping = 0.0f;    // 回転減衰
        VECTOR3::VEC3 linearFactor = VECTOR3::VEC3(1.0f);   // 各軸の移動制限
        VECTOR3::VEC3 angularFactor = VECTOR3::VEC3(1.0f);  // 各軸の回転制限
    };

    /* 物理データ */
    struct MO_PhysicsData
    {
        std::string presetID = "";
        // bool enabled = true;

        MO_RigidBodyData rigidBody;
        std::vector<MO_ColliderData> colliders;

        unsigned int _collisionMask = 0;                    // 衝突判定の対象となるCOLLISION_CATEGORYのビットマスク
        UtilityData::COLLISION_CATEGORY _myCollisionCategory = 
            UtilityData::COLLISION_CATEGORY::NONE;              // 自身の衝突カテゴリ
    };

    /* トランスフォームデータ */
    struct MO_TransformData
    {
        VECTOR3::VEC3 position = VECTOR3::VEC3();
        VECTOR3::VEC3 rotation = VECTOR3::VEC3();
        VECTOR3::VEC3 scale = VECTOR3::VEC3();
    };

    /* マップ内に配置されるオブジェクトのデータ */
    struct MapObjectData
    {
        std::string assetID = "";
        MO_TransformData transform;
    };

    /* マップのデータ */
    struct MapData
    {
        std::string mapName = "";
        std::string groundAssetTag = "";
        std::vector<MapObjectData> objects; // マップ内に定義するオブジェクトの配列
    };


    /* マップの地面アセット */
    struct Map_GroundAssetData
    {
        std::string assetID;

    };

    // マップ内に配置されるオブジェクトの定義
    struct Map_ObjectAssetData
    {
        std::string assetID = "";           // アセットID
        std::string physicsPresetID = "";   // 物理プリセット
        std::string modelPath = "";         // モデルパス

        MAP_OBJECT_TYPE objectType = 
            MAP_OBJECT_TYPE::PROP;

        std::vector<SetupMaterialInfo> materials;

        bool isGrabbable = false;       // 掴むことができるか
        bool isDestruction = true;      // 破壊できるか
        float health = 10.0f;           // 体力
    };


    /* 建物の破片データ */
    struct BuildingFragmentData
    {
        std::string modelPath;              // モデルパス
        std::string physicsPresetID = "";   // 物理プリセット
        MO_TransformData localTransform;    // ローカル座標
    };

    /* 建物の破片データをまとめたもの */
    struct BuildingFragmentDataSet
    {
        std::string assetID = "";                               // アセットID（どの建物モデルに対応した破片データか）
        std::vector<BuildingFragmentData> fragmentDataArray;     // 破片データ
    };
};
