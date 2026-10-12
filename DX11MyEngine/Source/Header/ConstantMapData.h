#pragma once
#include "MeshFactory.h"

namespace Map
{
    /* マップオジェクトの種類 */
    enum class MAP_OBJECT_TYPE
    {
        BUILDING,           // 建物
        EXPLOSION_BUILDING, // 爆発する建物
        PROP,               // 岩・車・柵など
    };


    // マップオブジェクトの種類を文字列からenumに変換するmap
    static std::map<std::string, MAP_OBJECT_TYPE> g_MapObjectTypeMap =
    {
        {"BUILDING",           MAP_OBJECT_TYPE::BUILDING},
        {"EXPLOSION_BUILDING", MAP_OBJECT_TYPE::EXPLOSION_BUILDING },
        {"PROP",               MAP_OBJECT_TYPE::PROP},
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

    struct ColliderData
    {
        COLLIDER_TYPE type;

        VECTOR3::VEC3 center;

        VECTOR3::VEC3 size;
        float radius = 0.0f;
        float height = 0.0f;
    };


    /* トランスフォームデータ */
    struct TransformData
    {
        VECTOR3::VEC3 position = VECTOR3::VEC3();
        VECTOR3::VEC3 rotation = VECTOR3::VEC3();
        VECTOR3::VEC3 scale = VECTOR3::VEC3();
    };

    /* マップ内に配置されるオブジェクトのデータ */
    struct MapObjectData
    {
        std::string name = "";
        std::string modelPath = "";
        TransformData transform;
        
        MAP_OBJECT_TYPE mapObjectType   // マップオブジェクトの種類
            = MAP_OBJECT_TYPE::BUILDING;

        bool isGrabbable = false;       // 掴むことができるか
        bool isDestruction = true;      // 破壊できるか
        float health = 10.0f;           // 体力
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

    };

    // オブジェクト定義
    struct Map_ObjectAssetData
    {
        std::string assetID;

        MAP_OBJECT_TYPE objectType;
        std::string modelAssetID;
        std::vector<SetupMaterialInfo> materials;
        std::vector<> colliders;

        bool isGrabbable;
        bool isDestruction;

        float health;
    };
};
