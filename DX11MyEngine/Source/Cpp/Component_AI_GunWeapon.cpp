#include "pch.h"
#include "Component_AI_GunWeapon.h"
#include "Component_Transform.h"
#include "Component_Collider.h"
#include "Component_LineRenderer.h"
#include "Component_PointLight.h"
#include "RendererEngine.h"
#include "GameObjectManager.h"
#include "GameObject.h"
#include "ResourceManager.h"
#include "GameManager.h"
#include "Gun_StateHeader.h"
#include "WeaponStateFactory.h"
#include "CollisionInfo.h"

using namespace DirectX;
using namespace PhysicsData;
using namespace GIGA_Engine;
using namespace VECTOR4;
using namespace VECTOR3;
using namespace VECTOR2;
using namespace BulletData;
using namespace WeaponData;
using namespace UtilityData;

constexpr float LASER_POINTER_SCALE = 0.2f;	// レーザーポインタの大きさ

//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//* [引数]
//* pOwner : オーナーオブジェクト
//* updateRank : 更新レイヤー
//*----------------------------------------------------------------------------------------
AI_GunWeapon::AI_GunWeapon(std::weak_ptr<GameObject> pOwner, int updateRank)
    : WeaponBase(pOwner, updateRank),
    m_IsStopFire(false),
    m_IsBurstFire(false),
    m_AmmoRemaining(0),
    m_Range(0.0f),
    m_FireInterval(0.0f),
    m_FireCooldownTimer(0.0f),
    m_BurstFireTimer(0.0f),
    m_BurstFireRemaining(0),
    m_LocalMuzzleOffset(VEC3()),
    m_TargetPos(VEC3())
{
    this->set_Tag("AI_GunWeapon");
}



//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
AI_GunWeapon::~AI_GunWeapon()
{

}



//*---------------------------------------------------------------------------------------
//*【?】初期化
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void AI_GunWeapon::Start(RendererEngine& renderer)
{
    // 照準レーザー用
    m_pLineRendererComp = m_pOwner.lock()->get_Component<LineRenderer>();
    m_pFlashPointLight = m_pOwner.lock()->get_Component<PointLight>();

    // 全ビットを0に
    m_WeaponFlags.Init();

    // 有効状態に
    get_WeaponFlags().EnableFlag(WEAPON_STATUS::ENABLED);
}

//*---------------------------------------------------------------------------------------
//*【?】更新
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void AI_GunWeapon::Update(RendererEngine& renderer)
{

}



//*---------------------------------------------------------------------------------------
//*【?】遅延更新
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void AI_GunWeapon::LateUpdate(RendererEngine& renderer)
{
    float deltaTime = Master::m_pTimeManager->get_DeltaTime();

    auto transform = m_pOwner.lock()->get_Transform().lock();
    VEC3 pos = transform->get_WorldVEC3ToPos();
    auto gunParam = get_GunWeaponData();

    // レーザーサイト
    if (!m_pLineRendererComp.expired())
    {
        if (gunParam->_isLaserSight)
        {
            // ワールド変換行列から方向をとる
            XMMATRIX worldMtx = transform->get_WorldMtx();
            XMVECTOR forward = worldMtx.r[2];  // Z
            //forward *= -1;  // プレイヤーが-Z前になってしまっているので

            VEC3 muzzlePos = pos + m_LocalMuzzleOffset;

            auto laserLineRenderer = m_pLineRendererComp.lock();

            laserLineRenderer->get_OwnerObj().lock()->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);

            VEC3 targetDir = (m_TargetPos - muzzlePos).Normalize();	// 目標方向


            // レーザーサイトの始点と方向
            laserLineRenderer->set_Color(VEC4(1.0f, 0.0f, 0.0f, 1.0f));
            laserLineRenderer->set_Dir(targetDir);
            laserLineRenderer->set_StartPos(muzzlePos);
            laserLineRenderer->set_Length(500.0f);
            laserLineRenderer->set_Width(0.5f);
        }
        else
        {
            auto laserLineRenderer = m_pLineRendererComp.lock();
            laserLineRenderer->set_Length(0.0f);
            laserLineRenderer->set_Width(0.0f);
        }
    }



    // フラッシュライト
    if (!m_pFlashPointLight.expired())
    {
        // 弾を発射してないときはフラッシュライトをオフ
        m_pFlashPointLight.lock()->set_Intensity(0.0f);
    }

    // バースト射撃中でなければ
    if (m_BurstFireRemaining <= 0)
    {
        m_FireCooldownTimer =
            std::max(0.0f, m_FireCooldownTimer - deltaTime);
    }
    else
    {
        const auto& gunParam = get_GunWeaponData();

        // 次の発射までの時間を減らす
        m_BurstFireTimer -= deltaTime;

        if (m_BurstFireRemaining > 0 &&
            m_BurstFireTimer <= 0.0f)
        {
            OnceFire(renderer);

            // バースト残弾減らす
            m_BurstFireRemaining--;

            if (m_BurstFireRemaining <= 0)
            {
                m_FireCooldownTimer = m_FireInterval;
            }
            else
            {
                m_BurstFireTimer = gunParam->_burstInterval;
            }
        }
    }


}


//*---------------------------------------------------------------------------------------
//*【?】武器のパラメータ取得
//*
//* [引数] なし
//* [返値] 書き換え不可の武器パラメータ
//*----------------------------------------------------------------------------------------
const WeaponData::GunWeaponData* AI_GunWeapon::get_GunWeaponData()const
{
    return static_cast<const WeaponData::GunWeaponData*>(m_pWeaponData);
}

//*---------------------------------------------------------------------------------------
//*【?】セットアップ
//*
//* [引数]
//* & _weaponData : 武器データ
//* [返値]なし
//*----------------------------------------------------------------------------------------
bool AI_GunWeapon::Setup(const WeaponData::BaseWeaponData* _pWeaponData)
{
    m_pWeaponData = static_cast<const GunWeaponData*>(_pWeaponData);
    if (!m_pWeaponData)
    {
        MessageBoxA(NULL, "武器のパラメータが一致しません", "GunWeapon", MB_OK);
        return false;
    }

    const auto& gunParam = get_GunWeaponData();

    m_AmmoRemaining = gunParam->_bulletMaxNum;

    // 発射音
    m_FireSoundID = Master::m_pSoundManager->FindSound(gunParam->_soundID);

    // バースト射撃かどうか
    m_IsBurstFire = gunParam->_burstCount != 0 ? true : false;

    // 射程距離を取得
    m_Range =
        gunParam->_bulletData._commonData._lifeTime * gunParam->_bulletData._commonData._speed;

    // 有効状態に
    get_WeaponFlags().EnableFlag(WEAPON_STATUS::ENABLED);


    // 1秒あたりの発射回数から発射間隔を計算
    m_FireInterval = 1.0f / get_GunWeaponData()->_fireRate;

    // 開始時は直ぐ撃てるようにする
    m_FireCooldownTimer = 0.0f;
    m_BurstFireTimer = 0.0f;

    return true;
}


//*---------------------------------------------------------------------------------------
//*【?】バースト射撃
//*
//* [引数] 
//* &renderer : 描画エンジンの参照
//* [返値] なし
//*----------------------------------------------------------------------------------------
void AI_GunWeapon::BurstFire(RendererEngine& renderer)
{
    // バースト実行中
    if (m_BurstFireRemaining > 0)
    {
        return;
    }

    // クールダウン中
    if (m_FireCooldownTimer > 0.0f)
    {
        return;
    }

    // 射撃
    OnceFire(renderer);


    const auto& gunParam = get_GunWeaponData();

    // 残弾減らす
    m_BurstFireRemaining = gunParam->_burstCount - 1;

    if (m_BurstFireRemaining > 0)
    {
        m_BurstFireTimer = std::max(0.01f, gunParam->_burstInterval);
    }
    // 1発で終了した場合
    else
    {
        m_FireCooldownTimer = m_FireInterval;
    }
}

//*---------------------------------------------------------------------------------------
//*【?】単発射撃
//*
//* [引数] 
//* &renderer : 描画エンジンの参照
//* [返値] なし
//*----------------------------------------------------------------------------------------
void AI_GunWeapon::OnceFire(RendererEngine& renderer)
{
    const auto& gunParam = get_GunWeaponData();

    auto transform = m_pOwner.lock()->get_Transform().lock();
    VEC3 pos = transform->get_WorldVEC3ToPos();
    pos += m_LocalMuzzleOffset;

    VEC3 targetDir = (m_TargetPos - pos).Normalize();	// 目標方向

    // ****************************************************
    //				 発射音再生
    // ****************************************************
    Master::m_pSoundManager->Play_RandPitch(m_FireSoundID, gunParam->_shotSoundRadius);

    // =====================================================================
    // 武器の向きに合わせて発射するやり方
    // =====================================================================
    VEC3 fw = targetDir;

    // 水平方向の向きを求める (Yaw)
    float yaw = atan2(fw.x, fw.z);

    // 水平成分の長さ
    float xzLen = sqrtf(fw.x * fw.x + fw.z * fw.z);

    // 垂直方向の角度を求める (Pitch)
    float pitch = atan2(-fw.y, xzLen);

    /* そのままだとプレイヤーの位置から発射されてしまうので、前方に少しオフセットした位置を発射位置とする */
    VEC3 firePos;
    firePos.x = pos.x + (fw.x);
    firePos.y = pos.y + (fw.y);
    firePos.z = pos.z + (fw.z);
 
    // これを弾やマズルフラッシュの基準となる回転クォータニオンにする
    XMVECTOR rotQuat = XMQuaternionRotationRollPitchYaw(pitch, yaw, 0.0f);

    // 基準の前方向（+Z）
    const XMVECTOR baseForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);


    //// =====================================================================
    //// ホーミング弾の場合は対象の探索
    //// =====================================================================
    std::weak_ptr<GameObject> homingTarget;
    //if (const auto* homing =
    //    std::get_if<HomingMovementConfig>(
    //        &gunParam->_bulletData._moveData))
    //{
    //    homingTarget = SelectHomingTarget(
    //        firePos,
    //        fw.Normalize(),
    //        homing->_lockOnRange,
    //        homing->_lockOnHalfAngleDeg);
    //}

    // 同時発射
    for (int i = 0; i < gunParam->_bulletSimultaneousNum; i++)
    {
        // 弾のバラつき
        float accuracy = gunParam->_accuracy;
        VEC3 accuracyRot;
        accuracyRot.x += Master::m_pRandomManager->GetFloatRandom(-accuracy, accuracy);
        accuracyRot.y += Master::m_pRandomManager->GetFloatRandom(-accuracy, accuracy);
        accuracyRot.z += Master::m_pRandomManager->GetFloatRandom(-accuracy, accuracy);

        // バラつきクォータニオン
        XMVECTOR spreadQuat = XMQuaternionRotationRollPitchYaw(accuracyRot.x, accuracyRot.y, accuracyRot.z);

        // 最終的なクォータニオン作成
        XMVECTOR finalRotQuat = XMQuaternionMultiply(rotQuat, spreadQuat);
        finalRotQuat = XMQuaternionNormalize(finalRotQuat); // 念のため正規化

        // バラつき適用後の前方向
        XMVECTOR finalForwardVec = XMVector3Rotate(baseForward, finalRotQuat);

        // VEC3へ
        VEC3 finalForward = VEC3::FromXMVECTOR(finalForwardVec);

        // 最終Yaw
        float finalYaw = atan2f(
            finalForward.x,
            finalForward.z
        );

        // 最終Pitch
        float xzLen = sqrtf(
            finalForward.x * finalForward.x +
            finalForward.z * finalForward.z
        );

        float finalPitch = atan2f(
            -finalForward.y,
            xzLen
        );

        // トランスフォームパラメータ
        BulletTransformData bulletTransform;
        bulletTransform._pos = firePos;
        bulletTransform._rotQ = finalRotQuat;
        bulletTransform._scale;

        BULLET_TYPE type = gunParam->_bulletType;

        BulletSpawnContext spawnContext;
        spawnContext._transform = bulletTransform;
        spawnContext._target = homingTarget;

        // 弾データを共用体で持っているので、弾タイプにあったパラメータを入れるようにする
        Master::m_pBulletManager->Shot(renderer, spawnContext, gunParam->_bulletData);


        // =====================================================================
        //レーザー弾の場合
        // =====================================================================
        auto& bulletData = gunParam->_bulletData;
        if (bulletData._commonVisualData._visualArchetype == BULLET_VISUAL_ARCHETYPE::EFFECT &&
            bulletData._commonData._bulletType == BULLET_TYPE::LASER)
        {
            VEC3 scale = bulletData._commonVisualData._scale;
            VEC4 effectColor = bulletData._commonVisualData._effectColor;

            // 再生
            auto handle = Master::m_pEffectManager->PlayEffect(
                bulletData._commonVisualData._bulletEffectTag, firePos);

            // スケール・回転
            Master::m_pEffectManager->SetScaleEffect(handle, scale);
            Master::m_pEffectManager->SetRotationEffect(handle, finalPitch, finalYaw, 0.0f);

            // カラー
            Effekseer::Color color = Effekseer::Color(effectColor.x, effectColor.y, effectColor.z, effectColor.w);
            Master::m_pEffectManager->SetAllColorEffect(handle, color);

            // 動的パラメータ
            // レーザーの長さ設定
            Master::m_pEffectManager->SetDynamicParameter(handle, 0, bulletData._commonData._range);
        }
    }

    if (!m_pFlashPointLight.expired()) {
        // フラッシュ
        m_pFlashPointLight.lock()->set_Range(5.0f);
        m_pFlashPointLight.lock()->set_Intensity(3.0f);
        m_pFlashPointLight.lock()->set_LightColor(VEC3(0.2f, 0.8f, 0.2f));
    }

    //*****************************************************************************************
    //						マズルフラッシュエフェクトの再生
    //*****************************************************************************************
    if (gunParam->_muzzleFlashEffectTag.empty() == false)
    {
        VEC3 muzzleScale = gunParam->_muzzleFlashEffectScale;   // エフェクトの大きさ

        auto handle = Master::m_pEffectManager->PlayEffect(gunParam->_muzzleFlashEffectTag, firePos, 1);
        Master::m_pEffectManager->SetScaleEffect(handle, muzzleScale.x, muzzleScale.y, muzzleScale.z);
        Master::m_pEffectManager->SetRotationEffect(handle, pitch, yaw, 0.0f);
    }

    // 弾数減らす
    m_AmmoRemaining--;
}


//*---------------------------------------------------------------------------------------
//*【?】弾の発射処理
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void AI_GunWeapon::Fire(RendererEngine& renderer)
{
    // まだ発射可能時間でない、または残弾が0以下なら、返す
    if (m_FireCooldownTimer > 0.0f || m_AmmoRemaining <= 0)
    {
        return;
    }

    // 武器使用の有無の設定がオフなら発射しない
    if (!Master::m_pDataManager->get_IsUseWeapon() ||
        !get_WeaponFlags().GetFlag(WEAPON_STATUS::ENABLED))
    {
        return;
    }

    const auto& gunParam = get_GunWeaponData();

    if (m_IsBurstFire)
    {
        // バースト射撃
        BurstFire(renderer);
    }
    else
    {
        // 単発射撃
        OnceFire(renderer);

        // 次の発射可能時間までのクールダウンタイマー設定
        m_FireCooldownTimer = m_FireInterval;
    }
}