#pragma once
#include "ConstantBulletData.h"

/// <summary>
/// 武器データをまとめた名前空間
/// </summary>
namespace WeaponData
{

    /// <summary>
    /// UI表示用の武器データ
    /// </summary>
    struct WeaponUIData
    {
        std::wstring _name;             // 武器名
        int _ammoMaxNum = 0;            // 弾数( 0なら弾を持たない武器 )
        int _ammoRemaining = 0;         // 現在の弾数
        float _reloadTime;              // リロード時間
        float _crntReloadTime;          // 現在のリロード時間
    };


    /// <summary>
    /// ベースとなる武器データ
    /// </summary>
    struct BaseWeaponData
    {
        virtual ~BaseWeaponData() = default;

        int _level = -1;                // 武器レベル
        std::wstring _name;             // 武器名

        /// <summary>
        /// リセット
        /// </summary>
        void Reset()
        {
            *this = BaseWeaponData();
        }
    };

    /// <summary>
    /// 銃系武器のデータ
    /// </summary>
    struct GunWeaponData : BaseWeaponData
    {
        int _bulletMaxNum = 0;                  // 弾数
        int _bulletSimultaneousNum = 1;         // 弾の同時発射数
        float _fireRate = 0.0f;                 // 連射速度
        int _burstCount = 0;                    // 1バーストで発射する回数
        float _burstInterval = 0.0f;            // バースト内の射撃間隔
        float _reloadTime = 0.0f;               // リロード時間
        float _accuracy = 0.0f;                 // 精度（ラジアン）
        float _zoomLength = 0.0f;               // ズーム倍率（0.0以外の時に作動）
        bool _isLaserSight = true;              // レーザーサイトはあるか
        float _shotSoundRadius = 0.0f;          // 発射音が聞こえる範囲
        std::string _soundID = "";              // 発射音のID
		std::string _muzzleFlashEffectTag;      // マズルフラッシュエフェクトのタグ
		VECTOR3::VEC3 _muzzleFlashEffectScale = VECTOR3::VEC3(1.0f); // マズルフラッシュエフェクトの大きさ
		VECTOR3::VEC3 _muzzleFlashEffectColor = VECTOR3::VEC3(1.0f); // マズルフラッシュエフェクトの色
		VECTOR3::VEC3 _muzzleFlashEffectColor_N = VECTOR3::VEC3(1.0f); // マズルフラッシュエフェクトの色 (0.0～1.0にしたもの jsonからは読み込まない)

        BulletData::BULLET_TYPE _bulletType = BulletData::BULLET_TYPE::NORMAL;  // 弾の種類

        // variantは共用体（union）を使いやすくしてくれるやつ
        // 必ず「_bulletType」と一致したデータを入れる
        //using BulletParamVariant = std::variant<BulletData::Definition, BulletData::ExplosionBulletData>;
        //BulletParamVariant _bulletParam;    // 弾のパラメータ

        BulletData::Definition _bulletData;    // 弾のデータ

        /// <summary>
        /// リセット
        /// </summary>
        void Reset()
        {
            *this = GunWeaponData();
        }
    };

    /// <summary>
    /// 爆発武器のデータ
    /// </summary>
    struct ExplosionWeaponData : GunWeaponData
    {
        /// <summary>
        /// リセット
        /// </summary>
        void Reset()
        {
            *this = ExplosionWeaponData();
        }
    };

    /// <summary>
    /// レンジャーの武器データ
    /// </summary>
    namespace Ranger
    {
        constexpr int RANGER_WEAPON_NUM = 2;    // 二つまで装備

        /// <summary>
        /// 装備する武器のカテゴリ
        /// </summary>
        enum class WEAPON_CATEGORY
        {
            PRIMARY01,  // メイン武器1
            PRIMARY02,  // メイン武器2
            SUB,        // サブ武器
            SPECIAL,    // 特殊（補助装備）
        };

        /// <summary>
        /// メイン武器の種類
        /// </summary>
        enum class PRIMARY_WEAPON_TYPE
        {
            ASSULT_RIFLE,       // アサルトライフル
            SHOTGUN,            // ショットガン
            SNIPER_RIFLE,       // スナイパーライフル
            ROCKET_LAUNCHER,    // ロケットランチャー
            MISSILE_LAUNCHER,   // ミサイルランチャー
            GRANADE_LAUNCHER,   // グレネードランチャー
            SUPER_WEAPON,       // スーパーウェポン
            NUM,
        };

        /// <summary>
        /// アサルトライフルの種類
        /// </summary>
        enum class ASSULT_RIFLE_ID
        {
            Hector22,
            Hector24PT,
            Hector24,

            RasterGCR1,
            RasterGCR2,
            RasterGCR3,
            RasterGCR4,
            RasterGCR5,

            NUM,
        };
    };
};