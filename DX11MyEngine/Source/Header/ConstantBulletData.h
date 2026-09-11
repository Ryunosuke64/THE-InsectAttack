#pragma once

namespace BulletData
{
    //////////////////////////////////////////////////////////////////////////////////////////
    //
    //						弾の分類・状態を表す列挙型
    //
    //////////////////////////////////////////////////////////////////////////////////////////

    /// <summary>
    /// 弾の振る舞いを表す大分類。
    /// </summary>
    enum class BULLET_BEHAVIOUR_TYPE
    {
        NORMAL,
        EXPLOSION,

        HOMING,

        NUM,
    };

    /// <summary>
    /// 弾の移動方式を表す識別子。
    /// </summary>
    enum class BULLET_MOVE_TYPE
    {
        LINEAR,
        HOMING,

    };

    /// <summary>
	/// 弾の描画に使用する形状を表す。
    /// </summary>
    enum class BULLET_VISUAL_ARCHETYPE
    {
        BILLBOARD,          // ビルボード
        MODEL,              // 3Dモデル
        //LASER,            // レーザー

        NUM,
    };

    /// <summary>
    /// 弾の用途や性質を表す大分類。
    /// </summary>
    enum class BULLET_TYPE
    {
        NORMAL,         // 通常弾
        EXPLOSION,      // 着弾時に爆発する弾
        EXPLOSION_DELAY,// 一定時間後に爆発する弾
        HORMING,        // 目標を追尾する弾
        LASER,          // 直線状のレーザー
        FLAME,          // 火炎弾
        ACID,           // 酸弾

        NUM,
    };

    /// <summary>
    /// 弾1発の実行中の状態。
    /// </summary>
    enum class BULLET_STATE
    {
        FLYING,     // 通常飛行中
        HOMING,     // 目標を追尾中
        ATTACHED    // 衝突対象に吸着中
    };

    /// <summary>
    /// 弾が非アクティブになった理由。
    /// </summary>
    enum class BULLET_END_REASON
    {
        HIT,            // 対象へ命中した
        OUT_OF_RANGE,   // 最大射程を超えた
        CANCELLED       // シーン遷移などで強制終了された
    };

    /// <summary>
    /// 衝突した際にどう反応すさせるか
    /// </summary>
    enum class ENVIRONMENT_RESPONSE
    {
        DEACTIVATE, // 衝突したら弾を消す
        SLIDE,      // 壁面に沿って移動する

        BOUNCE,     // 反射
        ATTACH,     // 壁へ吸着
        PENETRATE   // 壁を貫通
    };

    // 衝突した際の応答を文字列からenumに変換するmap
    static std::map<std::string, ENVIRONMENT_RESPONSE> g_EnvironmentResponenseMap =
    {
        {"DEACTIVATE",  ENVIRONMENT_RESPONSE::DEACTIVATE },
        {"SLIDE",       ENVIRONMENT_RESPONSE::SLIDE },
        {"BOUNCE",      ENVIRONMENT_RESPONSE::BOUNCE },
        {"ATTACH",      ENVIRONMENT_RESPONSE::ATTACH },
        {"PENETRATE",   ENVIRONMENT_RESPONSE::PENETRATE },
    };

    /// <summary>
    /// 発射時に使用する弾の初期Transform情報。
    /// </summary>
    struct BulletTransformData {
        VECTOR3::VEC3 _pos;          // 発射位置
        //VECTOR3::VEC3 _rotRad;     // オイラー角を使用する場合の回転
        DirectX::XMVECTOR _rotQ;     // 発射方向を表す回転クォータニオン
        VECTOR3::VEC3 _scale;        // 発射時の大きさ。現在はVisualConfig::_scaleを使用
    };

    /// <summary>
    /// 弾を1発生成する際にBulletManagerへ渡す情報。
    /// </summary>
    struct BulletSpawnContext
    {
        BulletTransformData _transform;          // 発射位置と向き
        std::weak_ptr<GameObject> _target;        // 追尾などで参照する対象
        std::weak_ptr<GameObject> _shooter;       // この弾を発射したオブジェクト
    };

    //////////////////////////////////////////////////////////////////////////////////////////
    //
    //						JSONから読み込む弾の設定
    //
    //////////////////////////////////////////////////////////////////////////////////////////

    //*****************************************************************************************
    //						共通設定
    //*****************************************************************************************
    /// <summary>
    /// 移動方式や命中方式に関係なく、すべての弾が使用する基本設定。
    /// </summary>
    struct CommonConfig
    {
        int _aliveFrame = 0;                        // 生存フレーム数
        float _speed = 0.0f;                        // 発射時の速度（1フレームに進む距離）
        float _maxSpeed = 0.0f;                     // 最大速度
        float _lifeTime = 0.0f;                     // 生存期間（_aliveFrame / 60）
        float _damage = 0.0f;                       // 命中時に与える基本ダメージ
        float _acceleration = 0.0f;                 // 1秒あたりの速度変化量
        float _gravityScale = 0.0f;                 // 重力の強さ。0.0fなら重力を適用しない
        float _knockbackForce = 1.0f;               // 命中対象へ加えるノックバックの強さ
        int _penetrationsCount = 0;                 // 貫通できる回数
        unsigned int _collisionMask = 0;            // 衝突判定の対象となるCOLLISION_CATEGORYのビットマスク
        float _collisionSize = 0.0f;                // 弾の衝突判定半径
    };


    //*****************************************************************************************
    //						移動設定
    //*****************************************************************************************
    /// <summary>
    /// 一定方向へ直進する弾の移動設定。
    /// 現在は固有パラメータを持たず、CommonConfigの値を使用する。
    /// </summary>
    struct LinearMovementConfig
    {

    };
    /// <summary>
    /// 目標を追尾する弾の移動設定。
    /// </summary>
    struct HomingMovementConfig
    {
        float _turnSpeed = 0.0f;                 // 1秒あたりの旋回速度
        float _targetingDuration = 0.0f;         // 追尾を継続する時間。0.0fの扱いは更新処理側で定義する
        float _accelerationStartDelay = 0.0f;    // 発射してから加速するまでの時間
        float _targetingStartDelay = 0.0f;       // 加速してから追尾を開始するまでの時間

        float _lockOnRange = 0.0f;               // ロックオン有効距離
        float _lockOnHalfAngleDeg = 0.0f;        // Lockオン角度
    };

    //*****************************************************************************************
    //						見た目設定
    //*****************************************************************************************
    /// <summary>
    /// 弾本体、軌跡、飛行中の煙に関する描画設定。
    /// </summary>
    struct CommonVisualConfig
    {
        BULLET_VISUAL_ARCHETYPE _visualArchetype = BULLET_VISUAL_ARCHETYPE::BILLBOARD; // 弾本体の描画方式
        std::string _bulletMaterialTag;                  // 弾本体に使用するマテリアルのタグ
        VECTOR3::VEC3 _scale = VECTOR3::VEC3();          // 弾本体の表示サイズ

        float _trailDrawTime = 0.0;                          // 軌跡の表示時間。0なら軌跡を表示しない
        float _trailWidth = 0;                           // 軌跡の幅
        VECTOR3::VEC3 _trailColor = VECTOR3::VEC3(1.0f); // 軌跡の色
        float _smokeInterval = 0.05f;                    // 飛行中の煙を生成する時間間隔
        float _smokeSize = 0.0f;                         // 煙のサイズ
        std::string _smokeEffectTag;                     // 飛行中に生成する煙エフェクトのタグ
        bool _enableFlightSmoke = false;                 // 飛行中の煙を有効にするか
    };

    /// <summary>
    /// スケールの変更用見た目データ
    /// </summary>
    struct ScaleLerpVisualConfig
    {
        VECTOR3::VEC3 _startScale;  // 開始時の大きさ
        VECTOR3::VEC3 _endScale;    // 最終的な大きさ
        float _duration;            // 時間
    };

    //*****************************************************************************************
    //						命中設定
    //*****************************************************************************************
    /// <summary>
    /// 対象へ直接命中する弾の設定。
    /// ダメージや貫通回数などの共通値はCommonConfigを使用する。
    /// </summary>
    struct DirectHitConfig
    {
        std::string _decalMaterialTag;              // 命中地点へ配置するデカールのマテリアルタグ
        std::string _hitEffectTag;                  // 命中地点で再生するエフェクトのタグ

        ENVIRONMENT_RESPONSE _environmentResponse   // 衝突時の反応
            = ENVIRONMENT_RESPONSE::DEACTIVATE;
    };

    /// <summary>
    /// 命中地点を中心に範囲効果を発生させる爆発弾の設定。
    /// </summary>
    struct ExplosionHitConfig : DirectHitConfig
    {
        float _explosionRadius = 0.0f;              // 爆発の効果半径
        float _explosionEffectAliveTime = 1.0f;     // 爆発エフェクトへ渡す再生時間倍率
        std::string _explosionEffectHandleTag;      // 爆発時に再生するエフェクトのタグ
        bool _isSmoke = true;                       // 爆発時に煙を生成するか

        VECTOR3::VEC3 _expLightColor = 0.0f;        // 爆発時のライトカラー
        float _expLightIntensity = 0.0f;            // 爆発時のライト強度
        float _expLightDuration = 0.0f;             // 爆発時のライトの持続時間

        /// <summary>
        /// 各メンバーを既定値へ戻す。
        /// </summary>
        void Reset()
        {
            *this = ExplosionHitConfig();
        }
    };


    //////////////////////////////////////////////////////////////////////////////////////////
    //
    //						実行時の状態・処理結果
    //
    //////////////////////////////////////////////////////////////////////////////////////////

    /// <summary>
    /// 移動処理が1フレーム分の計算結果として返す値。
    /// </summary>
    struct BulletMoveResult
    {
        VECTOR3::VEC3 _velocity;        // 1秒あたりの移動量
        DirectX::XMVECTOR _rotation;    // 移動後に適用する回転クォータニオン

        float _speed = 0.0f;
    };

    /// <summary>
    /// 見た目変更のための返却パラメータ
    /// </summary>
    struct BulletVisualResult
    {
        VECTOR3::VEC3 _scale;
        VECTOR4::VEC4 _color;
    };
    enum class BULLET_HIT_RESPONSE
    {
        PENETRATE,      // 貫通
        DEACTIVATE,     // 命中後に弾を非アクティブにするか
        PENETRATE_COUNT,// 命中対象を貫通したとして数えるか
        SLIDE,          // 壁沿い移動させる
        BOUNCE          // 反射させる
    };

    /// <summary>
    /// 命中処理後にBulletへ通知する制御結果。
    /// </summary>
    struct BulletHitResult
    {
        BULLET_HIT_RESPONSE _response = BULLET_HIT_RESPONSE::DEACTIVATE;
    };


    /// <summary>
    /// 使用する命中方式の設定を保持するvariant。
    /// </summary>
    using HitConfig = std::variant<
        DirectHitConfig,
        ExplosionHitConfig
    >;
    /// <summary>
    /// 使用する移動方式の設定を保持するvariant。
    /// </summary>
    using MovementConfig = std::variant<
        LinearMovementConfig,
        HomingMovementConfig
    >;
    /// <summary>
    /// 使用する描画方式の設定を保持するvariant。
    /// </summary>
    using CustomVisualConfig = std::variant <
        std::monostate,         // 空の状態を表す
        ScaleLerpVisualConfig
    >;

    /// <summary>
    /// JSONから読み込んだ、弾1種類分の変更されない設定一式。
    /// </summary>
    struct Definition
    {
        CommonConfig _commonData;                           // 全方式で共有する基本設定
        MovementConfig _moveData;                           // 移動方式とその設定
        CommonVisualConfig _commonVisualData;               // 共通描画設定
        CustomVisualConfig _customVisualData;               // 描画に関する設定
        HitConfig _hitData = DirectHitConfig{};             // 命中方式とその設定

        /// <summary>
        /// 設定一式を既定値へ戻す。
        /// </summary>
        void Reset()
        {
            *this = Definition();
        }
    };


    /// <summary>
    /// 弾1発ごとに保持する、更新中に変化する状態。
    /// </summary>
    struct RuntimeState
    {
        BULLET_STATE _state = BULLET_STATE::FLYING;        // 現在の状態

        VECTOR3::VEC3 _moveDirection;                       // 正規化済みの現在の進行方向
        class MyTransform* _transform = nullptr;            // 弾が所有するTransformへの非所有ポインタ
        float _currentSpeed = 0.0f;                         // 現在の移動速度
        float _traveledDistance = 0.0f;                     // 発射後に移動した累計距離
        float _stateTime = 0.0f;                            // 現在の状態へ遷移してからの経過時間
        float _elapsedTime = 0.0f;
        float _aliveTime = 0.0f;
        float _startRotZ = 0.0;
        float _smokeTime = 0.0f; 
        std::weak_ptr<class GameObject> _pTarget;           // 追尾対象
        std::weak_ptr<class GameObject> _pAttachedObject;   // 吸着している対象
        VECTOR3::VEC3 _attachedLocalPosition;               // 吸着対象から見た命中位置

        /// <summary>
        /// 設定一式を既定値へ戻す。
        /// </summary>
        void Reset()
        {
            *this = RuntimeState();
        }
    };
};
