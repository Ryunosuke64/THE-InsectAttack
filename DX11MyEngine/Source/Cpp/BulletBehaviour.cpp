#include "pch.h"
#include "BulletBehaviour.h"
#include "Component_Transform.h"
#include "Component_DecalRenderer.h"
#include "Component_TimerDestruction.h"
#include "Component_Health.h"
#include "Component_MoveLogic.h"
#include "RendererEngine.h"
#include "GameObjectManager.h"
#include "GameObject.h"
#include "InputFactory.h"
#include "MeshFactory.h"
#include "CollisionInfo.h"
#include "ResourceManager.h"
#include "Component_Collider.h"
#include "Component_Physics.h"
#include "Component_3DCamera.h"
#include "IMoveBehaviour.h"


using namespace GIGA_Engine;
using namespace UtilityData;
using namespace VECTOR3;
using namespace VECTOR4;
using namespace BulletData;

constexpr float DECAL_SIZE_FACTOR        = 7.0f;   // デカールの大きさの補正値（transformのスケールだと小さすぎるため）
constexpr float DECAL_Z_AXIS_SIZE_FACTOR = 0.0f;   // デカールの奥行に加算する補正値
constexpr float DECAL_LIFE_TIME          = 5.0f;   // デカールの生存時間
constexpr float EFFECTL_SIZE_FACTOR      = 2.0f;   // エフェクトの大きさの補正値（transformのスケールだと小さすぎるため）

constexpr float EXP_SHAKE_MAX_RANGE_EXPLOSION_SCALE_FACTOR  = 15.0f;    // カメラシェイク時、シェイクの最大距離を求める際に掛ける補正値
constexpr float EXP_SHAKE_LENGTH_SCALE_FACTOR               = 0.004f;   // カメラシェイク時、シェイクの大きさを求める際に掛ける補正値
constexpr float EXP_SHAKEDURATION                           = 1.0f;     // カメラシェイクの持続時間
constexpr float EXP_EFFECT_SIZE_FACTOR                      = 0.12f;    // そのままだと、エフェクトが大きすぎるので

constexpr float EXP_LIGHT_RADIUS_FACTOR = 30.0f; // 爆発範囲に掛ける、補正値


namespace BulletBehaviour
{
    //*---------------------------------------------------------------------------------------
    //*【?】移動更新 [直線移動]
    //*
    //* [引数]
    //* &_runtime  : ランタイムデータ
    //* &_common   : 共通データ
    //* &_moveData : 移動データ[直線]
    //* _deltaTime : デルタタイム
    //*
    //* [返値] BulletMoveResult
    //*----------------------------------------------------------------------------------------
    MoveParam UpdateMove(
        BulletData::RuntimeState& _runtime,
        const BulletData::CommonConfig& _common,
        const BulletData::LinearMovementConfig& _moveData,
        float _deltaTime)
    {
        MoveParam result;
        //_runtime._currentSpeed += _common._acceleration * _deltaTime;
        result._moveDirection = _runtime._moveDirection;
        result._moveSpeed = _runtime._currentSpeed;
        result._acceleration = _common._acceleration;
        result._gravity = _common._gravityScale;
        result._maxSpeed = _common._maxSpeed;

        _runtime._state = BULLET_STATE::FLYING;

        return result;
    }

    //*---------------------------------------------------------------------------------------
    //*【?】移動更新 [ホーミング移動]
    //*
    //* [引数]
    //* &_runtime  : ランタイムデータ
    //* &_common   : 共通データ
    //* &_moveData : 移動データ[ホーミング]
    //* _deltaTime : デルタタイム
    //*
    //* [返値] MoveParam
    //*----------------------------------------------------------------------------------------
    MoveParam UpdateMove(
        BulletData::RuntimeState& _runtime,
        const BulletData::CommonConfig& _common,
        const BulletData::HomingMovementConfig& _moveData,
        float _deltaTime)
    {
        MoveParam result;
        result._gravity = _common._gravityScale;
        result._moveSpeed = _runtime._currentSpeed;
        result._moveDirection = _runtime._moveDirection;
        result._maxSpeed = _common._maxSpeed;
        _runtime._state = BULLET_STATE::FLYING;

        // 加速開始時間か
        if (_runtime._aliveTime >= _moveData._accelerationStartDelay)
        {
            result._acceleration = _common._acceleration;
        }

        // ホーミング終了時間
        float targetingEnd =
            _moveData._targetingStartDelay +
            _moveData._targetingDuration;

        // 追尾可能時間か
        bool inTargetingTime =
            _runtime._aliveTime >= _moveData._targetingStartDelay &&
            _runtime._aliveTime < targetingEnd;

        if (!inTargetingTime) {
            return result;
        }

        const auto target = _runtime._pTarget.lock();
        if (!target) {
            return result;
        }
        const auto targetTransform = target->get_TransformConst();
        result._targetPos = targetTransform->get_VEC3ToPos();   // 目標座標
        result._turnSpeed = _moveData._turnSpeed;               // ターン速度

        // 目標へのベクトル
        VEC3 toTarget = result._targetPos - _runtime._transform->get_VEC3ToPos();
        if (toTarget.LengthSq() > 0.0001f)
        {
            // 追尾終了後に使用する方向として保存
            _runtime._moveDirection = toTarget.Normalize();
            result._moveDirection = _runtime._moveDirection;
            _runtime._state = BULLET_STATE::HOMING;
        }

        return result;
    }
    //*---------------------------------------------------------------------------------------
    //*【?】見た目更新 []
    //*
    //* [引数]
    //* &_runtime    : ランタイムデータ
    //* &_common     : 共通データ
    //* &_visualData : 見た目データ[スケール補間]
    //* _deltaTime   : デルタタイム
    //*
    //* [返値] BulletVisualResult
    //*----------------------------------------------------------------------------------------
    BulletData::BulletVisualResult UpdateVisual(
        BulletData::RuntimeState& _runtime,
        const BulletData::CommonConfig& _common,
        const BulletData::ScaleLerpVisualConfig& _visualData,
        float _deltaTime)
    {
        BulletVisualResult result;

        _runtime._elapsedTime += _deltaTime;

        float rate = _runtime._elapsedTime / _visualData._duration;

        rate = std::clamp(rate, 0.0f, 1.0f);

        result._scale = VECTOR3::VEC3::Lerp(
            _visualData._startScale,
            _visualData._endScale,
            rate);

        float rate2 = _runtime._aliveTime / _common._lifeTime;
        result._color = VECTOR4::VEC4::Lerp(
            VEC4(1.0f),
            VEC4(0.0f),
            rate2);

        return result;
    }

    //*---------------------------------------------------------------------------------------
    //*【?】ヒット時の処理 [ダイレクト]
    //*
    //* [引数]
    //* &_runtime   : ランタイムデータ
    //* &_common    : 共通データ
    //* &_moveData  : ヒットデータ[ダイレクト]
    //* &_collision : ヒット時の情報
    //*
    //* [返値]
    //* BulletHitResult
    //*----------------------------------------------------------------------------------------
    BulletData::BulletHitResult OnHit(
        BulletData::RuntimeState& _runtime,
        const BulletData::CommonConfig& _common,
        const BulletData::DirectHitConfig& _hitData,
        const CollisionInfo& _collision,
        class RendererEngine& _renderer)
    {
        BulletHitResult result;

        //*****************************************************************************************
        //						衝突した際の処理
        //*****************************************************************************************
        auto hitObj = _collision.get_HitObject().lock();
        if (!hitObj) return result;
        COLLISION_CATEGORY hitCategory = _collision.get_HitCollider().lock()->get_CollisionCategory();

        // 相手がHealthComponentを持っているか確認（破壊可能な建物は壊せないように）
        auto health = hitObj->get_Component<Health>();
        if (health && hitCategory != COLLISION_CATEGORY::DESTRUCTION_BUILDING)
        {
            // 弾が保持しているダメージ値を渡す
            health->TakeDamage(_common._damage, _collision);

        }

        //*****************************************************************************************
        //						建物などに衝突した場合
        //*****************************************************************************************
        if (hitCategory == COLLISION_CATEGORY::BUILDING || 
            hitCategory == COLLISION_CATEGORY::DESTRUCTION_BUILDING)
        {
            // 衝突時の反応
            switch (_hitData._environmentResponse)
            {
                /* 無効 */
            case ENVIRONMENT_RESPONSE::DEACTIVATE:
                result._response = BULLET_HIT_RESPONSE::DEACTIVATE;
                break;

                /* スライド */
            case ENVIRONMENT_RESPONSE::SLIDE:
                // 壁に沿って移動
                result._response = BULLET_HIT_RESPONSE::SLIDE;
                break;

                /* バウンド */
            case ENVIRONMENT_RESPONSE::BOUNCE:
                result._response = BULLET_HIT_RESPONSE::BOUNCE;
                break;

                /* アタッチ（未使用） */
            case ENVIRONMENT_RESPONSE::ATTACH:
                result._response = BULLET_HIT_RESPONSE::DEACTIVATE;
                break;

                /* 貫通 */
            case ENVIRONMENT_RESPONSE::PENETRATE:
                result._response = BULLET_HIT_RESPONSE::DEACTIVATE;
                break;

                /* その他 */
            default:
                result._response = BULLET_HIT_RESPONSE::DEACTIVATE;
                break;
            }
        }
        //*****************************************************************************************
        //						敵やプレイヤーなどの有機物に衝突した場合
        //*****************************************************************************************
        else
        {
            // 衝突時の反応
            switch (_hitData._environmentResponse)
            {
                /* 無効 */
            case ENVIRONMENT_RESPONSE::DEACTIVATE:
                result._response = BULLET_HIT_RESPONSE::DEACTIVATE;
                break;
            
                /* スライド */
            case ENVIRONMENT_RESPONSE::SLIDE:
                // そのまま通り抜けさせる
                result._response = BULLET_HIT_RESPONSE::PENETRATE;
                break;

                /* バウンド */
            case ENVIRONMENT_RESPONSE::BOUNCE:
                // 壁以外（有機物）に衝突した際は、バウンドさせず、無効にする
                result._response = BULLET_HIT_RESPONSE::DEACTIVATE;
                break;
            
                /* アタッチ（未使用） */
            case ENVIRONMENT_RESPONSE::ATTACH:
                result._response = BULLET_HIT_RESPONSE::DEACTIVATE;
                break;
            
                /* 貫通 */
            case ENVIRONMENT_RESPONSE::PENETRATE:
                // 貫通数を増やす
                result._response = BULLET_HIT_RESPONSE::PENETRATE_COUNT;
                break;
            
                /* その他 */
            default:
                result._response = BULLET_HIT_RESPONSE::DEACTIVATE;
                break;
            }
        }

        auto transform = _runtime._transform;
        VEC3 pos = transform->get_VEC3ToPos();


        VEC3 hitNormal = _collision.get_HitNormal();    // 衝突相手の法線
        VEC3 hitPoint = _collision.get_HitPoint();      // 衝突位置

        // 水平方向の向きを求める
        float yaw = atan2(hitNormal.x, hitNormal.z);
        // 水平成分の長さ
        float xzLen = sqrtf(hitNormal.x * hitNormal.x + hitNormal.z * hitNormal.z);
        // 垂直方向の角度を求める
        // 法線の逆を向かせたいのでマイナスを付ける
        float pitch = atan2(-hitNormal.y, xzLen);

        //*****************************************************************************************
        //						デカールの生成
        //*****************************************************************************************
        if (_hitData._decalMaterialTag.empty() == false)
        {
            auto matPtr = Master::m_pResourceManager->FindMaterial(_hitData._decalMaterialTag);

            SetupMaterialInfo matInfo[1];
            matInfo[0].Index = 0;
            matInfo[0].pMaterialData = matPtr;

            CreateDecalInfo decal;
            decal.pRenderer = &_renderer;
            decal.Type = UTILITY_MESH_TYPE::CUBE;
            decal.MatNum = 1;
            decal.MaterialData = matInfo;
            decal.IsActive = false;
            decal.ShaderType = SHADER_TYPE::DEFERRED_STD_DECAL;
            decal.IsNormalMap = false;
            decal.IsDynamic = true;

            float angleZ = Tool::RandRange(0.0f, 6.14f);

            VEC3 scale = transform->get_VEC3ToScale();
            scale *= DECAL_SIZE_FACTOR;
            scale.z += DECAL_Z_AXIS_SIZE_FACTOR;

            auto obj = MeshFactory::CreateDecal(decal);
            obj->get_Component<DecalRenderer>()->Start(_renderer);
            obj->get_Transform().lock()->set_Pos(hitPoint);
            obj->get_Transform().lock()->set_Scale(scale);
            obj->get_Transform().lock()->set_RotateToRad(pitch, yaw, angleZ);
            obj->set_Tag("BulletHole");
            auto timer = obj->add_Component<TimerDestruction>();
            timer->set_LifeTime(DECAL_LIFE_TIME);  // 生存時間
        }

        //*****************************************************************************************
        //						ヒットエフェクトの再生
        //*****************************************************************************************
        if (_hitData._hitEffectTag.empty() == false)
        {
            VEC3 effectRot = VEC3(pitch, yaw, 0.0f);
            int effectHandle = Master::m_pEffectManager->PlayEffect(_hitData._hitEffectTag);

            VEC3 effectScale = transform->get_VEC3ToScale();
            effectScale *= EFFECTL_SIZE_FACTOR;     // 大きさの補正

            // エフェクトパラメータ
            Master::m_pEffectManager->SetScaleEffect(effectHandle, effectScale.x, effectScale.y, effectScale.z);
            Master::m_pEffectManager->SetPositionEffect(effectHandle, hitPoint.x, hitPoint.y, hitPoint.z);
            Master::m_pEffectManager->SetRotationEffect(effectHandle, effectRot.x, effectRot.y, effectRot.z);
        }

        //*****************************************************************************************
        //						ヒットサウンド再生
        //*****************************************************************************************
        Master::m_pSoundManager->Play_RandPitch_3D(SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::ROCOCHET01), hitPoint, 40, 300);

        return result;
    }

    //*---------------------------------------------------------------------------------------
    //*【?】ヒット時の処理 [爆発]
    //*
    //* [引数]
    //* &_runtime   : ランタイムデータ
    //* &_common    : 共通データ
    //* &_moveData  : ヒットデータ[爆発]
    //* &_collision : ヒット時の情報
    //*
    //* [返値]
    //* BulletHitResult
    //*----------------------------------------------------------------------------------------
    BulletData::BulletHitResult OnHit(
        BulletData::RuntimeState& runtime,
        const BulletData::CommonConfig& _common,
        const BulletData::ExplosionHitConfig& _hitData,
        const CollisionInfo& _collision,
        class RendererEngine& _renderer)
    {
        BulletHitResult result;

        VEC3 crntPos = runtime._transform->get_VEC3ToPos();

        // ****************************************************
        //				 カメラシェイク
        // ****************************************************
        std::shared_ptr<Camera3D> camera;
        if (camera = Master::m_pDataManager->get_CameraComponent().lock())
        {
            float maxRange = _hitData._explosionRadius * EXP_SHAKE_MAX_RANGE_EXPLOSION_SCALE_FACTOR;
            float shaleLength = _hitData._explosionRadius * EXP_SHAKE_LENGTH_SCALE_FACTOR;
            camera->DistanceDecay(EXP_SHAKEDURATION, VEC3(shaleLength), crntPos, maxRange);
        }


        // ****************************************************
        //				 爆発音再生
        // ****************************************************
        Master::m_pSoundManager->Play_3D(SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::EXPLOSION01), crntPos, 1500.0f);


        VEC3 hitNormal = _collision.get_HitNormal();    // 衝突相手の法線
        VEC3 hitPoint = _collision.get_HitPoint();      // 衝突位置

        // 水平方向の向きを求める
        float angleY = atan2(hitNormal.x, hitNormal.z);
        // 水平成分の長さ
        float xzLen = sqrtf(hitNormal.x * hitNormal.x + hitNormal.z * hitNormal.z);
        // 垂直方向の角度を求める
        // 法線の逆を向かせたいのでマイナスを付ける
        float angleX = atan2(-hitNormal.y, xzLen);
        float angleZ = Tool::RandRange(0.0f, 6.14f);

        // デカールの作成
        {
            //auto matPtr = Master::m_pResourceManager->FindMaterial(_hitData._decalMaterialTag);

            //SetupMaterialInfo matInfo[1];
            //matInfo[0].Index = 0;
            //matInfo[0].pMaterialData = matPtr;

            //CreateDecalInfo decal;
            //decal.pRenderer = &_renderer;
            //decal.Type = UTILITY_MESH_TYPE::CUBE;
            //decal.MatNum = 1;
            //decal.MaterialData = matInfo;
            //decal.IsActive = false;
            //decal.ShaderType = SHADER_TYPE::DEFERRED_STD_DECAL;
            //decal.IsNormalMap = false;
            //decal.IsDynamic = true;

            //float expSize = _hitData._explosionRadius * 1.5f;   // 元が半径なので2倍に
            //VEC3 scale;
            //scale.x = expSize;
            //scale.y = expSize;
            //scale.z = expSize;

            //auto obj = MeshFactory::CreateDecal(decal);
            //obj->get_Component<DecalRenderer>()->Start(_renderer);
            //obj->get_Transform().lock()->set_Pos(hitPoint);
            //obj->get_Transform().lock()->set_Scale(scale);
            //obj->get_Transform().lock()->set_RotateToRad(angleX, angleY, angleZ);
            //obj->set_Tag("BulletHole");
            //auto timer = obj->add_Component<TimerDestruction>();
            //timer->set_LifeTime(DECAL_LIFE_TIME);  // 生存時間
        }

        float expSize = _hitData._explosionRadius * 2.0f;   // 元が半径なので2倍に
        VEC3 scale;
        scale.x = expSize;
        scale.y = expSize;
        scale.z = expSize;

        // エフェクト
        VEC3 effectRot = VEC3(abs(angleX - 0.05f), angleY, 0.0f);
        int exp_handle = Master::m_pEffectManager->PlayEffect(_hitData._explosionEffectHandleTag);   // 爆発

        float effectExpSize = expSize * EXP_EFFECT_SIZE_FACTOR;   // 爆発半径（そのままだと大きすぎるので補正）
        VEC3 expRot = Master::m_pRandomManager->GetVEC3Random(0.0f, 3.14f);

        if (_hitData._isSmoke)
        {
            int exp_smoke_handle = Master::m_pEffectManager->PlayEffect("Explosion_Smoke_01");   // 煙

            // 爆発煙
            Master::m_pEffectManager->SetScaleEffect(exp_smoke_handle, effectExpSize, effectExpSize, effectExpSize);
            Master::m_pEffectManager->SetPositionEffect(exp_smoke_handle, hitPoint.x, hitPoint.y, hitPoint.z);
            Master::m_pEffectManager->SetRotationEffect(exp_smoke_handle, 0.0f, 0.0f, 0.0f);

            Master::m_pEffectManager->SetDynamicParameter(exp_smoke_handle, 1, _hitData._explosionEffectAliveTime); // 生存時間を変更
        }

        // 爆発
        Master::m_pEffectManager->SetScaleEffect(exp_handle, effectExpSize, effectExpSize, effectExpSize);
        Master::m_pEffectManager->SetPositionEffect(exp_handle, hitPoint.x, hitPoint.y, hitPoint.z);
        //Master::m_pEffectManager->SetRotationEffect(exp_handle, expRot.x, expRot.y, expRot.z);
        // 動的パラメータの設定
        Master::m_pEffectManager->SetDynamicParameter(exp_handle, 1, _hitData._explosionEffectAliveTime); // 生存時間を変更

        //*****************************************************************************************
        //						爆発時にパッと光らせる
        //*****************************************************************************************
        TransientPointLightDesc desc;
        desc._color = _hitData._expLightColor;
        desc._startRange = _hitData._explosionRadius * EXP_LIGHT_RADIUS_FACTOR;
        desc._endRange = 0.0f;
        desc._startIntensity = _hitData._expLightIntensity;
        desc._endIntensity = 0.0f;
        desc._duration = _hitData._expLightDuration;
        auto handle = Master::m_pLightManager->PlayTransientPointLight(
            hitPoint,
            desc);


        unsigned mask = _common._collisionMask;;
        // 範囲内チェック
        auto targets = Master::m_pCollisionManager->CheckSphere(hitPoint, _hitData._explosionRadius, mask);

        // 範囲内の全員にダメージ
        for (auto& target : targets)
        {
            if (auto obj = target->get_OwnerObj().lock())
            {
                // ダメージ
                if (auto health = obj->get_Component<Health>())
                {
                    health->TakeDamage(_common._damage, _collision);
                }

                // 吹っ飛び
                if (auto physics = obj->get_Component<Physics>())
                {
                    auto targetTransform = obj->get_Transform().lock();
                    VEC3 targetPos = targetTransform->get_VEC3ToPos();

                    VECTOR3::VEC3 knockbackDir = targetPos - hitPoint;   // 爆心地から外（衝突オブジェクト）へ向かうベクトル
                    knockbackDir = knockbackDir.Normalize();

                    // 少し上（Y軸）に向かせることで、放物線を描いて飛ばせる
                    knockbackDir.y += 0.5f;
                    knockbackDir.x += Master::m_pRandomManager->GetFloatRandom(-0.5f, 0.5f);
                    knockbackDir.z += Master::m_pRandomManager->GetFloatRandom(-0.5f, 0.5f);
                    knockbackDir = knockbackDir.Normalize();

                    // 衝撃ベクトルの設定
                    physics->AddImpulse(knockbackDir * _common._knockbackForce);

                    knockbackDir.y = Master::m_pRandomManager->GetFloatRandom(-1, 1);
                    knockbackDir.x = Master::m_pRandomManager->GetFloatRandom(-1, 1);
                    knockbackDir.z = Master::m_pRandomManager->GetFloatRandom(-1, 1);
                    physics->AddAngularImpulse(knockbackDir * _common._knockbackForce);
                }
            }
        }
        return result;
    }
};