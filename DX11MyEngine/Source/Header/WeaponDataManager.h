#pragma once
#include "ConstantWeaponData.h"
#include "ConstantBulletData.h"
#include "ConstantUtilityData.h"



// ---------------------------------------------------------------------------------------
/* --- @:WeaponDataManager Class --- */
//
//  ★★★シングルトン★★★
//
// 【?】武器のマスターパラメータを管理する
//      データの所有権 / 保持
//      読み取り専用データの提供
//
// ***************************************************************************************
class WeaponDataManager
{
private:

    /* 全ての武器データを管理 */
    std::unordered_map<int, std::unique_ptr<WeaponData::BaseWeaponData>> m_AllWeaponsDataMap;
    std::unordered_map<int, std::unique_ptr<WeaponData::BaseWeaponData>> m_EnemyWeaponsDataMap;

    /* レンジャーの武器カテゴリごとの検索用 */
    std::map<WeaponData::Ranger::PRIMARY_WEAPON_TYPE, std::vector<const WeaponData::BaseWeaponData*>> m_RangerCategoryViewMap;



    std::array<
        BulletData::SurfaceHitTable,
        static_cast<size_t>(BulletData::BULLET_TYPE::NUM)> m_SurfaceHitData;


public:
    WeaponDataManager();
    ~WeaponDataManager();

    /// <summary>
    /// 初期化
    /// </summary>
    bool Init();

    /// <summary>
    /// 指定IDの武器データを検索・取得する
    /// </summary>
    /// <param name="_id">武器ID</param>
    /// <returns>読み取り専用武器データ</returns>
    const WeaponData::BaseWeaponData* FindWeaponData(int _id)const;


    /// <summary>
    /// 指定IDの敵の武器データを検索・取得する
    /// </summary>
    /// <param name="_id">武器ID</param>
    /// <returns>読み取り専用武器データ</returns>
    const WeaponData::BaseWeaponData* FindEnemysWeaponData(int _id)const;

    // 武器の読み込み
    bool LoadGunWeaponData(const std::string& filepath, WeaponData::GunWeaponData& outData);


    // 材質ごとのヒットデータ読み込み
    bool LoadSurfaceHitData(const std::string& filepath);

    // ヒットテーブルの検索
    const BulletData::SurfaceHitTable& FindSurfaceHitTable(BulletData::BULLET_TYPE bulletType);

    // エフェクトタグの検索
    const std::string& FindSurfaceHitEffectTag(BulletData::BULLET_TYPE bulletType, SURFACE_TYPE surfaceType);

private:
    // コピー禁止
    WeaponDataManager(const WeaponDataManager&) = delete;
    WeaponDataManager& operator=(const WeaponDataManager&) = delete;
    // ------------------------------------------------------

    /// <summary>
    /// 弾データの読み取り
    /// </summary>
    /// <param name="json"></param>
    /// <param name="outData"></param>
    /// <returns></returns>
    bool LoadBulletData(
        const nlohmann::json& json,
        BulletData::Definition& outData);    
    
    bool LoadMovementData(
        const nlohmann::json& json,
        BulletData::Definition& outData);
        
    bool LoadHitData(
        const nlohmann::json& json,
        BulletData::Definition& outData);

    bool LoadVisualData(
        const nlohmann::json& json,
        BulletData::Definition& outData);

    bool ExtractionSurfacesHitData(const nlohmann::json& _json, BulletData::SurfaceHitTable& _outData);
};

