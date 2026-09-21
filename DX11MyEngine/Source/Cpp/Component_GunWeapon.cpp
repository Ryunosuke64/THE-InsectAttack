#include "pch.h"
#include "Component_GunWeapon.h"
#include "Component_Transform.h"
#include "Component_NormalBullet.h"
#include "Component_Collider.h"
#include "RendererEngine.h"
#include "GameObjectManager.h"
#include "GameObject.h"
#include "InputFactory.h"
#include "ResourceManager.h"
#include "Component_3DCamera.h"
#include "Component_LineRenderer.h"
#include "Component_PointLight.h"
#include "GameManager.h"
#include "Gun_StateHeader.h"
#include "WeaponStateFactory.h"
#include "Component_SkinnedMeshAnimator.h"
#include "CollisionInfo.h"

using namespace DirectX;
using namespace GIGA_Engine;
using namespace Input;
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
GunWeapon::GunWeapon(std::weak_ptr<GameObject> pOwner, int updateRank)
    : WeaponBase(pOwner, updateRank),
    m_StateMachine(this),
    m_IsNowZoom(false),
    m_IsStopFire(false),
    m_AmmoRemaining(0),
    m_Range(0.0f),
    m_CrntReloadTime(0.0f),
    m_FireInterval(0.0f),
    m_FireTimer(0.0f)
{
    this->set_Tag("GunWeapon");
}



//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
GunWeapon::~GunWeapon()
{

}


//*---------------------------------------------------------------------------------------
//*【?】初期化
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void GunWeapon::Start(RendererEngine& renderer)
{
    // 照準レーザー用
    m_pLineRendererComp = m_pOwner.lock()->get_Component<LineRenderer>();
    m_pFlashPointLight = m_pOwner.lock()->get_Component<PointLight>();


    // ステートの作成
    WeaponStateFactory::CreateGunWeapon(m_StateMachine, 0, renderer);
    m_StateMachine.SetStartState(GUN_STATE::GUN_STATE_IDLE);

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
void GunWeapon::Update(RendererEngine& renderer)
{

}


//*---------------------------------------------------------------------------------------
//*【?】遅延更新
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void GunWeapon::LateUpdate(RendererEngine& renderer)
{
    if (Master::m_pDataManager->get_CameraComponent().expired())
    {
        assert(false);
        return;
    }

    auto camera = Master::m_pDataManager->get_CameraComponent().lock();

    float c_AngleH = camera->get_Angle_H();
    float c_AngleV = camera->get_Angle_V();
    float deltaTime = Master::m_pTimeManager->get_DeltaTime();

    auto transform = m_pOwner.lock()->get_Transform().lock();


    auto player = Master::m_pGameObjectManager->get_ObjectByTag("Player");
    auto skinnedMesh = player->get_Component<SkinnedMeshAnimator>();

    //auto& boneMtx = skinnedMesh->get_BoneLocalWorldMatrix("mixamorig:RightHand");
    
    // 武器の位置を右手の位置に合わせる
    auto& boneMtx = skinnedMesh->get_BoneLocalWorldMatrix("WeaponSocket_Right");
    transform->set_OffsetWorldTransfomationMatrix(boneMtx);

    VEC3 pos = transform->get_WorldVEC3ToPos();

    // 武器を回転させる
    // 水平方向はプレイヤーに合わせているので垂直方向のみ、カメラの回転を使う。
    //transform->set_RotateToRad(VEC3(c_AngleV * -1, 0.0f, 0.0f));

    auto gunParam = get_GunWeaponData();

    // レーザーサイト
    if (gunParam->_isLaserSight && !m_pLineRendererComp.expired())
    {
        // ワールド変換行列から方向をとる
        XMMATRIX worldMtx = transform->get_WorldMtx();
        XMVECTOR forward = worldMtx.r[2];  // Z
        //forward *= -1;  // プレイヤーが-Z前になってしまっているので

		auto laserLineRenderer = m_pLineRendererComp.lock();

        laserLineRenderer->get_OwnerObj().lock()->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);

        // レーザーサイトの始点と方向
        laserLineRenderer->set_Dir(VEC3::FromXMVECTOR(XMVector3Normalize(forward)));
        laserLineRenderer->set_StartPos(pos);

        // レーザーポインタ
        auto laserPoint = Master::m_pGameObjectManager->get_ObjectByTag("LaserPointBillboard");
        laserPoint->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);

        VEC3 laserDir = laserLineRenderer->get_Dir();
		VEC3 laserPointPos = VEC3(10000.0f); // 何処にもあたってない場合、とりあえず遠くに置いておく
        
		// レイの情報を作る
        CollInData_Ray ray;
		ray._point = pos;
		ray._dir = laserDir * m_Range;

        unsigned hitMask = UINT_CAST(COLLISION_CATEGORY::ENEMY) | UINT_CAST(COLLISION_CATEGORY::BUILDING) | UINT_CAST(COLLISION_CATEGORY::DESTRUCTION_BUILDING);
        CollisionInfo hitInfo;
		
        // レイキャストして当たった位置にレーザーポインタを置く
        if (Master::m_pCollisionManager->CheckRaycast(ray, hitMask, &hitInfo))
        {
			laserPointPos = hitInfo.get_HitPoint(); // 少し浮かせる
        }

        laserPoint->get_Transform().lock()->set_Pos(laserPointPos);
        laserPoint->get_Transform().lock()->set_Scale(VEC3(LASER_POINTER_SCALE));

    }

    if (!m_pFlashPointLight.expired()) {
        // 弾を発射してないときはフラッシュライトをオフ
        m_pFlashPointLight.lock()->set_Intensity(0.0f);
    }

    // ズーム倍率があるなら
    if (gunParam->_zoomLength > 1.0f)
    {
        // FOVに倍率
        float defaultFov = Master::m_pDataManager->get_DefaultFov();
        float zoomFov = defaultFov;

        // 右クリック
        if (GetMouseClickDown(MOUSE_BUTTON_STATE::RIGHT)) {
            m_IsNowZoom = m_IsNowZoom ? false : true;   // ズーム切り替え
        }

        // ズーム
        if (m_IsNowZoom) {
            zoomFov = defaultFov / gunParam->_zoomLength; // 倍率が高いほどFovが小さくなる
        }

        // Fovの設定
        camera->set_Fov(zoomFov);
    }

    m_FireTimer += deltaTime;


    /* 武器情報 */
    //const auto& baseBulletData = std::visit([](const auto& arg) -> const BulletData::NormalBulletData& {
    //    return arg;
    //    }, gunParam->_bulletParam);
    //Master::m_pDebugger->BeginDebugWindow(Tool::U8ToChar(u8"武器情報"));
    //Master::m_pDebugger->DG_BulletText(Tool::U8ToChar(u8"名前：%s"), Tool::WStringToString(gunParam->_name).c_str());
    //Master::m_pDebugger->DG_BulletText(Tool::U8ToChar(u8"弾数：%d / %d"), m_AmmoRemaining, gunParam->_bulletMaxNum);
    //Master::m_pDebugger->DG_BulletText(Tool::U8ToChar(u8"ダメージ：%.2f"), baseBulletData._damage);
    //Master::m_pDebugger->DG_BulletText(Tool::U8ToChar(u8"リロード時間：%.2f"), gunParam->_reloadTime);
    //Master::m_pDebugger->DG_BulletText(Tool::U8ToChar(u8"ズーム倍率：%.2f"), gunParam->_zoomLength);
    //Master::m_pDebugger->EndDebugWindow();
    
    // ステート更新
    m_StateMachine.Update();
}


//*---------------------------------------------------------------------------------------
//*【?】描画
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void GunWeapon::Draw(RendererEngine& renderer)
{

}
//*---------------------------------------------------------------------------------------
//*【?】武器のパラメータ取得
//*
//* [引数] なし
//* [返値] 書き換え不可の武器パラメータ
//*----------------------------------------------------------------------------------------
const WeaponData::GunWeaponData* GunWeapon::get_GunWeaponData()const
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
bool GunWeapon::Setup(const WeaponData::BaseWeaponData* _pWeaponData)
{
    m_pWeaponData = static_cast<const GunWeaponData*>(_pWeaponData);
    if (!m_pWeaponData)
    {
        MessageBoxA(NULL, "武器のパラメータが一致しません", "GunWeapon", MB_OK);
        return false;
    }

    auto gunParam = get_GunWeaponData();

    m_AmmoRemaining = gunParam->_bulletMaxNum;

    // 射程距離を取得
    m_Range = 
        gunParam->_bulletData._commonData._lifeTime * gunParam->_bulletData._commonData._speed;


    // 有効状態に
    get_WeaponFlags().EnableFlag(WEAPON_STATUS::ENABLED);


    // 1秒あたりの発射回数から発射間隔を計算
    m_FireInterval = 1.0f / get_GunWeaponData()->_fireRate;

    // 開始時は直ぐ撃てるよう大きな値を入れる
    m_FireTimer = 9999.0f;

    return true;
}


//*---------------------------------------------------------------------------------------
//*【?】武器切り替え時のリセット
//*
//* [引数] 
//* &renderer : 描画エンジンの参照
//* [返値] なし
//*----------------------------------------------------------------------------------------
void GunWeapon::SwicthReset()
{
	// ズーム解除
    Master::m_pDataManager->set_Fov(Master::m_pDataManager->get_DefaultFov());

    auto laserPoint = Master::m_pGameObjectManager->get_ObjectByTag("LaserPointBillboard");
    laserPoint->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);

    m_IsNowZoom = false;

    // レーザーサイトオフ
    if (auto laserLineRenderer = m_pLineRendererComp.lock())
    {
        //laserLineRenderer->get_OwnerObj().lock()->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
    }
}


//*---------------------------------------------------------------------------------------
//*【?】弾の発射処理
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void GunWeapon::Fire(RendererEngine& renderer)
{
    // まだ発射可能時間に達していなければ、返す
    if (m_FireTimer <= m_FireInterval)return;

    // タイマーリセット
    m_FireTimer = 0.0f;

	// 武器使用の有無の設定がオフなら発射しない
    if (!Master::m_pDataManager->get_IsUseWeapon() ||
        !get_WeaponFlags().GetFlag(WEAPON_STATUS::ENABLED))
    {
        return;
    }

    // カメラの取得
    if (Master::m_pDataManager->get_CameraComponent().expired())
    {
        assert(false);
        return;
    }
    auto camera = Master::m_pDataManager->get_CameraComponent().lock();

    float c_AngleH =camera->get_Angle_H();
    float c_AngleV =camera->get_Angle_V();

    auto transform = m_pOwner.lock()->get_Transform().lock();
    VEC3 pos = transform->get_WorldVEC3ToPos();

    // ****************************************************
    //				 発射音再生
    // ****************************************************
    Master::m_pSoundManager->Play_RandPitch(SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::GUN_FIRE02), 300);
        
    auto gunParam = get_GunWeaponData();

    // =====================================================================
    // 武器の向きに合わせて発射するやり方
    // =====================================================================
    {
        //   // ワールド変換行列から方向をとる
        //   XMMATRIX worldMtx = transform->get_WorldMtx();
        //   DirectX::XMVECTOR scale;
        //   DirectX::XMVECTOR rotQuat;
        //   DirectX::XMVECTOR trans;
        //   // 変換行列の分解しクォータニオンを取得する
        //   DirectX::XMMatrixDecompose(&scale, &rotQuat, &trans, worldMtx);


        //   // 基準となる前方ベクトル
        //   DirectX::XMVECTOR baseForward = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
        //   // 武器の回転クオータニオンで、前方ベクトルを回転させる
        //   DirectX::XMVECTOR forwardVec = DirectX::XMVector3Rotate(baseForward, rotQuat);
        //   
        //   VEC3 fw = VEC3::FromXMVECTOR(forwardVec);

        //   // 水平方向の向きを求める (Yaw)
        //   float yaw = atan2(fw.x, fw.z);

        //   // 水平成分の長さ
        //   float xzLen = sqrtf(fw.x * fw.x + fw.z * fw.z);

        //   // 垂直方向の角度を求める (Pitch)
        //   float pitch = atan2(-fw.y, xzLen);

           ///* そのままだとプレイヤーの位置から発射されてしまうので、前方に少しオフセットした位置を発射位置とする */
        //   VEC3 firePos;
        //   firePos.x = pos.x + (fw.x * 1.0f);
        //   firePos.y = pos.y + (fw.y * 1.0f);
        //   firePos.z = pos.z + (fw.z * 1.0f);
    }
    
    // =====================================================================
	// カメラの向きに合わせて発射するやり方
    // =====================================================================
    // カメラの位置と、カメラの注視点を取得
    VEC3 focusPoint = camera->get_FocusPoint();
    VEC3 cameraPos = camera->get_CameraPos();
    VEC3 lookDir = camera->get_LookDir();

    XMVECTOR vCamPos = XMVectorSet(cameraPos.x, cameraPos.y, cameraPos.z, 1.0f);
    XMVECTOR vFocus = XMVectorSet(focusPoint.x, focusPoint.y, focusPoint.z, 1.0f);

    // カメラの前方ベクトルを計算する (注視点 - カメラ位置 を正規化)
    XMVECTOR camForward = XMVector3Normalize(XMVectorSubtract(vFocus, vCamPos));

    // はるか遠くにある仮想のターゲット位置を計算
    float targetDistance = 1000.0f;
    XMVECTOR targetPos = XMVectorMultiplyAdd(lookDir, XMVectorReplicate(targetDistance), vCamPos);

	VEC3 firePos;
	firePos.x = pos.x + (camForward.m128_f32[0]);
    firePos.y = pos.y + (camForward.m128_f32[1]);
    firePos.z = pos.z + (camForward.m128_f32[2]);

    // 発射位置からターゲット位置への「弾の方向ベクトル」を求める
    XMVECTOR vFirePos = XMVectorSet(firePos.x, firePos.y, firePos.z, 1.0f);
    XMVECTOR bulletDirVec = XMVector3Normalize(XMVectorSubtract(targetPos, vFirePos));

    // 基準の前方向（+Z）
    XMVECTOR baseForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

    // 弾の方向ベクトルから Yaw と Pitch を計算（エフェクトや弾のクォータニオン用）
    VEC3 fw = VEC3::FromXMVECTOR(bulletDirVec);
    float yaw = atan2f(fw.x, fw.z);
    float xzLen = sqrtf(fw.x * fw.x + fw.z * fw.z);
    float pitch = atan2f(-fw.y, xzLen);

    // これを弾やマズルフラッシュの基準となる回転クォータニオンにする
    XMVECTOR rotQuat = XMQuaternionRotationRollPitchYaw(pitch, yaw, 0.0f);

    // =====================================================================
    // ホーミング弾の場合は対象の探索
    // =====================================================================
    std::weak_ptr<GameObject> homingTarget;
    if (const auto* homing =
        std::get_if<HomingMovementConfig>(
            &gunParam->_bulletData._moveData))
    {
        homingTarget = SelectHomingTarget(
            cameraPos,
            fw.Normalize(),
            homing->_lockOnRange,
            homing->_lockOnHalfAngleDeg);
    }

    // 同時発射
    for (int i = 0; i < gunParam->_bulletSimultaneousNum; i++)
    {
        // 親の向き等を参照
        //rad.x = (c_AngleV);
        //rad.y = (c_AngleH - 1.57f);
        //rad.z = 0.0f;

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
        m_pFlashPointLight.lock()->set_LightColor(VEC3(1.0f, 0.8f, 0.0f));
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
//*【?】追尾対象を見つける
//*
//* [引数] 
//*_cameraPos&          : カメラ座標 
//*_fw&                 : 前方向
//*_rockOnRange         : ロックオン射程 
//*_rockOnHalfAngleDeg  : ロックオン角度（デグリー）
//* 
//* [返値] 
//* 追尾対象の参照ポインタ
//*----------------------------------------------------------------------------------------
std::weak_ptr <class GameObject>GunWeapon::SelectHomingTarget(const VECTOR3::VEC3& _cameraPos, const VECTOR3::VEC3& _fw, float _rockOnRange, float _rockOnHalfAngleDeg)
{
    const auto candidates = Master::m_pGameObjectManager->get_ObjectListByFactionAlive(FACTION::ENEMY);

    float rangeSq = _rockOnRange * _rockOnRange;
    const float minDot = cosf(Tool::DegToRad(_rockOnHalfAngleDeg));

    std::shared_ptr<GameObject> bestTarget;
    float bestDot = minDot;
    float bestDistanceSq = FLT_MAX;

    for (const auto &candidate : candidates)
    {
        const auto transform = candidate->get_Transform().lock();
        if (!transform) {
            continue;
        }

        VEC3 toTarget = transform->get_VEC3ToPos() - _cameraPos;
        float distanceSq = toTarget.LengthSq();

        // 範囲内チェック
        if (distanceSq <= 0.00001f || distanceSq > rangeSq){
            continue;
        }

        float dot = VEC3::Dot(_fw, toTarget.Normalize());
        
        // ロックオン角度内か
        if (dot < minDot){
            continue;
        }
        bool closerToAim = dot > bestDot;
        bool sameAimAndNearer = fabsf(dot - bestDot) <= 0.0001f && distanceSq < bestDistanceSq;

        if (closerToAim || sameAimAndNearer)
        {
            bestTarget = candidate;
            bestDot = dot;
            bestDistanceSq = distanceSq;
        }
    }

    return bestTarget;
}


//*---------------------------------------------------------------------------------------
//*【?】UI表示に必要な武器データを取得する
//*
//* [引数] なし
//* [返値] 
//* UI表示用武器データ 
//*----------------------------------------------------------------------------------------
WeaponUIData GunWeapon::get_WeaponUIData()const
{
    WeaponUIData uiData;
    uiData._name = get_GunWeaponData()->_name;
    uiData._ammoMaxNum = get_GunWeaponData()->_bulletMaxNum;
    uiData._ammoRemaining = m_AmmoRemaining;
    uiData._reloadTime = get_GunWeaponData()->_reloadTime;
    uiData._crntReloadTime = m_CrntReloadTime;

    return uiData;
}