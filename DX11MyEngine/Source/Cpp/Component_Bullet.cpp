#include "pch.h"
#include "Component_Bullet.h"
#include "BulletBehaviour.h"
#include "RendererEngine.h"
#include "Component_Transform.h"
#include "Component_DecalRenderer.h"
#include "Component_TimerDestruction.h"
#include "Component_Health.h"
#include "Component_MoveLogic.h"
#include "GameObjectManager.h"
#include "GameObject.h"
#include "InputFactory.h"
#include "MeshFactory.h"
#include "CollisionInfo.h"
#include "ResourceManager.h"
#include "Component_Collider.h"
#include "Component_Physics.h"
#include "Component_RigidBody.h"
#include "Component_BillboardRenderer.h"

using namespace PhysicsData;
using namespace GIGA_Engine;
using namespace UtilityData;
using namespace Input;
using namespace VECTOR3;
using namespace BulletData;

//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//* [引数]
//* pOwner : オーナーオブジェクト
//* updateRank : 更新レイヤー
//*----------------------------------------------------------------------------------------
Bullet::Bullet(std::weak_ptr<GameObject> pOwner, int updateRank) :
    IComponent(pOwner, updateRank),
    m_StartPos(VEC3()),
    m_PrevPos(VEC3()),
    m_MoveDir(VEC3()),
    m_pDefinition(nullptr),
    //m_pCurrentBehaviour(nullptr),
    m_CrntPenetrationCount(0)
{
    this->set_Tag("Bullet");
}

//*---------------------------------------------------------------------------------------
//*【?】初期化
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void Bullet::Start(RendererEngine& renderer) 
{
}

//*---------------------------------------------------------------------------------------
//*【?】更新
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void Bullet::Update(RendererEngine& renderer)
{
    VEC3 crntPos = m_Runtime._transform->get_VEC3ToPos();
    float deltaTime = Master::m_pTimeManager->get_DeltaTime();
    auto moveComp = m_pOwner.lock()->get_Component<MoveLogic>();

    // 前の位置として保持
    m_PrevPos = crntPos;

    // 移動パラメータ
    MoveParam param;

    // 移動
    std::visit([&](const auto& moveData)
        {
            using MoveType = std::decay_t<decltype(moveData)>;

            /* 直進 */
            if constexpr (std::is_same_v <MoveType, BulletData::LinearMovementConfig>)
            {
                m_Runtime._moveDirection = m_MoveDir;

                param =
                    BulletBehaviour::UpdateMove(
                        m_Runtime,
                        m_pDefinition->_commonData,
                        moveData,
                        deltaTime);
            }
            /* ホーミング */
            else if constexpr (std::is_same_v <MoveType, BulletData::HomingMovementConfig>)
            {
                param =
                    BulletBehaviour::UpdateMove(
                        m_Runtime,
                        m_pDefinition->_commonData,
                        moveData,
                        deltaTime);

                // 追尾終了後の直進方向として保持
                m_MoveDir = param._moveDirection;
            }
        },
        m_pDefinition->_moveData);
    /*
    * 移動ロジックの切り替え
    */
    if (m_Runtime._state == BULLET_STATE::HOMING)
    {
        moveComp->ChangeBehaviour(MOVE_BEHAVIOUR_TYPE::HOMING);
    }
    else
    {
        moveComp->ChangeBehaviour(MOVE_BEHAVIOUR_TYPE::LINEAR);
    }


    // 移動ロジックにパラメータを渡す
    moveComp->set_MoveParam(param);	

    
    // 見た目処理
    if (m_pDefinition->_customVisualData.valueless_by_exception() == false)
    {
        std::visit(
            [&](const auto& visualData)
            {
                using VisualType = std::decay_t<decltype(visualData)>;

                // カスタム見た目設定なし
                if constexpr (std::is_same_v<VisualType, std::monostate>)
                {
                    return;
                }
                else
                {
                    // 見た目に変更がある場合のみ更新
                    const  BulletVisualResult visual_result =
                        BulletBehaviour::UpdateVisual(
                            m_Runtime,
                            m_pDefinition->_commonData,
                            visualData,
                            deltaTime);

                    // スケール変更
                    m_Runtime._transform->set_Scale(
                        visual_result._scale);

                    // ビルボードのカラーを変更
                    if (m_pBillboardRenderer != nullptr) {
                        m_pBillboardRenderer->set_ColorFactor(visual_result._color);
                    }


                    float ease = Tool::Easing::EaseOutQuint(m_Runtime._elapsedTime / m_pDefinition->_commonData._lifeTime);
                    m_Runtime._transform->set_RotateToRad(VEC3(0.0, 0.0, m_Runtime._startRotZ + (ease * 5.0f)));
                }
            },
            m_pDefinition->_customVisualData);
    }

    //m_Runtime._moveDirection = m_MoveDir;

    //// 移動処理
    //BulletMoveResult move_result = std::visit(
    //    [&](const auto& moveData)
    //    {
    //        return BulletBehaviour::UpdateMove(
    //            m_Runtime,
    //            m_pDefinition->_commonData,
    //            moveData,
    //            deltaTime);
    //    },
    //    m_pDefinition->_moveData);

    //// 新しい座標を求める
    //VEC3 newPos = m_PrevPos + move_result._velocity * deltaTime;

    //m_pMyTransform->set_Pos(newPos);
    //m_pMyTransform->set_RotationQuaternion(move_result._rotation);
}

//*---------------------------------------------------------------------------------------
//*【?】遅延更新
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void Bullet::LateUpdate(RendererEngine& renderer)
{
    float deltaTime = Master::m_pTimeManager->get_DeltaTime();

    // 移動後の位置
    VEC3 newPos = m_Runtime._transform->get_VEC3ToPos();

    m_Runtime._aliveTime += deltaTime;
    m_Runtime._smokeTime += deltaTime;

    // 生存期間を過ぎたら無効にする
    if (m_Runtime._aliveTime >=
        m_pDefinition->_commonData._lifeTime)
    {
        Deactivate(BULLET_END_REASON::OUT_OF_RANGE);
        return;
    }

    // 煙
    if (m_pDefinition->_commonVisualData._enableFlightSmoke)
    {
        if (m_Runtime._smokeTime >= m_pDefinition->_commonVisualData._smokeInterval)
        {
            int smoke_handle = Master::m_pEffectManager->PlayEffect(m_pDefinition->_commonVisualData._smokeEffectTag);   // 煙
            float smokeSize = m_pDefinition->_commonVisualData._smokeSize;
            // 爆発煙
            Master::m_pEffectManager->SetScaleEffect(smoke_handle, smokeSize, smokeSize, smokeSize);
            Master::m_pEffectManager->SetPositionEffect(smoke_handle, newPos.x, newPos.y, newPos.z);

            m_Runtime._smokeTime = 0.0f;
        }
    }

    CollisionInfo hitInfo;
    bool isHit = HitCheck(m_pDefinition->_commonData._bulletType, hitInfo);
    if (isHit)
    {
        VEC3 hitPoint = hitInfo.hitPoint;
        VEC3 normal = hitInfo.hitNormal.Normalize();
        
        m_PrevPos = hitPoint;

        // 衝突時の処理
        BulletHitResult result = std::visit(
            [&](const auto& hitData)
            {
                return BulletBehaviour::OnHit(
                    m_Runtime,
                    m_pDefinition->_commonData,
                    hitData,
                    hitInfo,
                    renderer);
            },
            m_pDefinition->_hitData);
            

        switch (result._response)
        {
            /* 貫通 */
        case BULLET_HIT_RESPONSE::PENETRATE:
            break;

            /* 弾を無効にする */
        case BULLET_HIT_RESPONSE::DEACTIVATE:
            Deactivate(BULLET_END_REASON::HIT);
            break;

            /* 貫通数増やす */
        case BULLET_HIT_RESPONSE::PENETRATE_COUNT:
            m_CrntPenetrationCount++;

            // 貫通可能回数を超えた
            if(m_CrntPenetrationCount >= m_pDefinition->_commonData._penetrationsCount)
            {
                Deactivate(BULLET_END_REASON::HIT);
            }
            break;

            /* 壁沿い移動 */
        case BULLET_HIT_RESPONSE::SLIDE:
        {
            // 衝突点に位置を合わせる
            m_Runtime._transform->set_Pos(hitPoint);

            VEC3 slideDir =
                m_MoveDir - normal * VEC3::Dot(m_MoveDir, normal);

            if (slideDir.LengthSq() > 0.0001)
            {
                m_MoveDir = slideDir.Normalize();
            }
            else
            {
                // 壁へほぼ垂直に当たったため、壁面方向を決定できない
                // その場に留まり、生存時間切れを待つ
                m_Runtime._currentSpeed = 0.0f;
            }
        }
        break;

            /* 反射 */
        case BULLET_HIT_RESPONSE::BOUNCE:
        {
            //*****************************************************************************************
            //						めり込み防止の処理を行ってから、反射させる
            //*****************************************************************************************
            constexpr float SKIN_WIDTH = 0.01f;
            
            VEC3 remaining = newPos - hitPoint;
            // 壁へ入り込む成分を除去
            float intoWall = VEC3::Dot(remaining, normal);
            if (intoWall < 0.0f)
            {
                remaining -= normal * intoWall;
            }
            VEC3 resolvedPos = hitPoint + normal * SKIN_WIDTH + remaining;

            m_Runtime._transform->set_Pos(resolvedPos);
            m_PrevPos = resolvedPos;

            // 反射
            m_MoveDir = VEC3::Reflect(m_MoveDir, normal).Normalize();
        }
        break;

        default:
            break;
        }

        auto obj = hitInfo.hitObject.lock();

        if (!obj)
        {
            return;
        }

        // 吹っ飛び
        if (auto rigidBody = obj->get_Component<RigidBody>())
        {
            auto targetTransform = obj->get_Transform().lock();
            VEC3 targetPos = targetTransform->get_VEC3ToPos();
            VECTOR3::VEC3 knockbackDir = m_MoveDir;     // 移動ベクトルをそのまま衝撃のベクトルにする

            // 衝撃ベクトルの設定
            rigidBody->AddImpulse(knockbackDir * m_pDefinition->_commonData._knockbackForce);
        }
    }
}

//*---------------------------------------------------------------------------------------
//*【?】パラメータ等の設定
//*     発射時に呼ぶ 
//* [引数]
//* _pParam : 弾のパラメータ 
//* &_context : 弾の生成に必要な情報
//* [返値]なし
//*----------------------------------------------------------------------------------------
void Bullet::Setup(
    const BulletData::Definition*_pData,
    const BulletData::BulletSpawnContext& _context)
{
    m_pDefinition = _pData;

    _context._shooter;
    _context._target;
    _context._transform;

    // トランスフォームのポインタを取得
    auto transform = m_pOwner.lock()->get_Transform().lock().get();

    m_pBillboardRenderer = nullptr;

    if (auto billboard = m_pOwner.lock()->get_Component<BillboardRenderer>())
    {
        m_pBillboardRenderer = billboard.get();
    }

    // 開始位置
    m_StartPos = transform->get_VEC3ToPos();
    m_PrevPos = m_StartPos;
    
    // 前方向ベクトル
    m_MoveDir = transform->get_WorldForward().Normalize();
    
    m_CrntPenetrationCount = 0;

    m_Runtime._transform = transform;
    m_Runtime._pTarget = _context._target;
    m_Runtime._moveDirection = m_MoveDir;
    m_Runtime._currentSpeed = _pData->_commonData._speed;
    m_Runtime._state = BULLET_STATE::FLYING;


    m_Runtime._startRotZ = Master::m_pRandomManager->GetFloatRandom(-3.0f,3.0f);
}

//*---------------------------------------------------------------------------------------
//*【?】無効にする
//* [引数]
//* _reason  : どんな条件で無効になったか
//* [返値]なし
//*----------------------------------------------------------------------------------------
void Bullet::Deactivate(BulletData::BULLET_END_REASON _reason)
{
    switch (_reason)
    {
    case BulletData::BULLET_END_REASON::HIT:
        break;
    case BulletData::BULLET_END_REASON::OUT_OF_RANGE:
        break;
    case BulletData::BULLET_END_REASON::CANCELLED:
        break;
    default:
        break;
    }


    // 終了処理
    //m_pCurrentBehaviour->OnEnd(*this, _reason);

    m_pOwner.lock()->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);    // ノンアクティブに
}

//*---------------------------------------------------------------------------------------
//*【?】状態のリセット
//* [引数]なし
//* [返値]なし
//*----------------------------------------------------------------------------------------
void Bullet::Reset()
{
    m_CrntPenetrationCount = 0;
    m_Runtime.Reset();
    auto moveLogic = m_pOwner.lock()->get_Component<MoveLogic>();
    moveLogic->ParamReset();

    // ビルボードのカラーを元に戻す
    if (m_pBillboardRenderer != nullptr)
    {
        m_pBillboardRenderer->set_ColorFactor(1.0f);
    }

    m_pBillboardRenderer = nullptr;
}

//*---------------------------------------------------------------------------------------
//*【?】衝突チェック
//* [引数]
//* _type                : どんな条件で無効になったか
//* & _outCollisionInfo  : 衝突情報の出力先
//*
//* [返値]
//* 衝突したかどうか
//*----------------------------------------------------------------------------------------
bool Bullet::HitCheck(BulletData::BULLET_TYPE _type, CollisionInfo& _outCollisionInfo)
{
    bool isHit = false;
    switch (_type)
    {
    case BulletData::BULLET_TYPE::NORMAL:
        isHit = HitCheck_RaySegment(_outCollisionInfo);
        break;
    case BulletData::BULLET_TYPE::EXPLOSION:
        isHit = HitCheck_RaySegment(_outCollisionInfo);
        break;
    case BulletData::BULLET_TYPE::HORMING:
        isHit = HitCheck_RaySegment(_outCollisionInfo);
        break;
    case BulletData::BULLET_TYPE::LASER:
        isHit = HitCheck_Ray(_outCollisionInfo);
        break;
    case BulletData::BULLET_TYPE::FLAME:
        isHit = HitCheck_RaySegment(_outCollisionInfo);
        //isHit = HitCheck_Sphere(_outCollisionInfo);
        break;
    case BulletData::BULLET_TYPE::ACID:
        isHit = HitCheck_RaySegment(_outCollisionInfo);
        //isHit = HitCheck_Sphere(_outCollisionInfo);
        break;
    case BulletData::BULLET_TYPE::NUM:
        break;
    default:
        break;
    }

    return isHit;
}

//*---------------------------------------------------------------------------------------
//*【?】球状判定
//* [引数]
//* & _outCollisionInfo  : 衝突情報の出力先
//* 
//* [返値]
//* 衝突したかどうか
//*----------------------------------------------------------------------------------------
bool Bullet::HitCheck_Sphere(CollisionInfo& _outCollisionInfo)
{
    // 移動後の位置
    VEC3 pos = m_Runtime._transform->get_VEC3ToPos();
    float radius = m_pDefinition->_commonData._collisionSize;
    unsigned mask = m_pDefinition->_commonData._collisionMask;

    return false;
}

//*---------------------------------------------------------------------------------------
//*【?】レイセグメント判定
//*     前回の座標との差分からレイキャストをする 
//* 
//* [引数]
//* & _outCollisionInfo  : 衝突情報の出力先
//* [返値]
//* 衝突したかどうか
//*----------------------------------------------------------------------------------------
bool Bullet::HitCheck_RaySegment(CollisionInfo& _outCollisionInfo)
{
    // 移動後の位置
    VEC3 newPos = m_Runtime._transform->get_VEC3ToPos();

    // レイキャストで衝突判定（コライダーの衝突処理をこっちに移動）
    CollInData_Ray ray;
    ray._point = m_PrevPos;           // 前回の位置からレイを飛ばす
    ray._dir = newPos - m_PrevPos;    // 前回の位置から新しい位置へのベクトル
    if (ray._dir.LengthSq() < 0.001f)
    {
        return false;
    }
    unsigned mask = m_pDefinition->_commonData._collisionMask;
    unsigned group = m_pDefinition->_commonData._myCollisionCategory;
    //return Master::m_pCollisionManager->CheckRaycast(ray, mask, &_outCollisionInfo);

    return Master::m_pPhysicsEngine->Raycast(ray, group, mask, &_outCollisionInfo);
}

//*---------------------------------------------------------------------------------------
//*【?】レイ判定
//* [引数]
//* & _outCollisionInfo  : 衝突情報の出力先
//* 
//* [返値]
//* 衝突したかどうか
//*----------------------------------------------------------------------------------------
bool Bullet::HitCheck_Ray(CollisionInfo& _outCollisionInfo)
{
    // レイキャストで衝突判定（コライダーの衝突処理をこっちに移動）
    CollInData_Ray ray;
    ray._point = m_StartPos;                                    // 前回の位置からレイを飛ばす
    ray._dir = m_MoveDir * m_pDefinition->_commonData._range;
    unsigned mask = m_pDefinition->_commonData._collisionMask;
    unsigned group = m_pDefinition->_commonData._myCollisionCategory;
    //return Master::m_pCollisionManager->CheckRaycast(ray, mask, &_outCollisionInfo);

    return Master::m_pPhysicsEngine->Raycast(ray, group, mask, &_outCollisionInfo);
}