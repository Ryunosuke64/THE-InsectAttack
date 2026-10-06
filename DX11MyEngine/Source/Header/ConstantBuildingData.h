#pragma once

namespace BuildingData
{
    /* 建物の種類 */
    enum class BUILDING_TYPE_ID
    {
        BUILDING_TYPE_ID_GROUND_ROAD_1,	// 道路	
        BUILDING_TYPE_ID_BUILDING_1,

        BUILDING_TYPE_ID_MAX,
    };

    /* マップの保存する情報*/
    struct BUILDING_SAVE_DATA
    {
        BUILDING_TYPE_ID _id;	// オブジェクトの種類
        VECTOR3::VEC3 _pos;		// 座標
        VECTOR3::VEC3 _rotate;	// 回転
        VECTOR3::VEC3 _scale;	// スケール
    };

    /* マップの種類 */
    enum class MAP_TYPE_ID
    {
        TOWN_1,	// 街 1

        NUM,
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

    struct TransformData
    {
        VECTOR3::VEC3 position = VECTOR3::VEC3();
        VECTOR3::VEC3 rotation = VECTOR3::VEC3();
        VECTOR3::VEC3 scale = VECTOR3::VEC3();
    };

    struct MapObjectData
    {
        std::string name = "";
        std::string modelPath = "";
        TransformData transform;


    };
};
