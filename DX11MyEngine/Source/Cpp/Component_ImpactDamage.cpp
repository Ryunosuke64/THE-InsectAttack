#include "pch.h"
#include "Component_ImpactDamage.h"
#include "Component_Health.h"
#include "Component_3DCamera.h"
#include "GameObject.h"

using namespace GIGA_Engine;
using namespace VECTOR3;

//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//* [引数]
//* pOwner : オーナーオブジェクト
//* updateRank : 更新レイヤー
//*----------------------------------------------------------------------------------------
ImpactDamage::ImpactDamage(std::weak_ptr<GameObject> pOwner, int updateRank)
    :IComponent(pOwner, updateRank),
    m_ElapsedTime(0.0f),
    m_IsImapctEnabled(false)

{
    this->set_Tag("ImpactDamage");
}

//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
ImpactDamage::~ImpactDamage()
{

}

//*---------------------------------------------------------------------------------------
//*【?】初期化
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void ImpactDamage::Start(RendererEngine& renderer)
{
}


//*---------------------------------------------------------------------------------------
//*【?】更新
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void ImpactDamage::Update(RendererEngine& renderer)
{
    if (m_IsImapctEnabled == false)
    {
        return;
    }

    float deltaTime = Master::m_pTimeManager->get_DeltaTime();

    m_ElapsedTime += deltaTime;
}


//*---------------------------------------------------------------------------------------
//*【?】衝突処理
//*
//* [引数]
//* & _other : 衝突相手の情報
//* [返値]なし
//*----------------------------------------------------------------------------------------
void ImpactDamage::OnCollisionEnter(const PhysicsData::CollisionInfo& _other)
{
    if (m_IsImapctEnabled == false)
    {
        return;
    }
    VEC3 hitPos = _other.hitPoint;
    //*******************************************************
    // ダメージ適用可能時間内なら自身と相手にダメージを与える
    //*******************************************************
    if (m_ElapsedTime <= m_Param.damageActiveDuration)
    {
        //********************************************************************************
        // 衝突した相手のオブジェクトに体力コンポーネントがあれば、ダメージを与える
        //********************************************************************************
        auto hitObj = _other.hitObject.lock();
        if (hitObj)
        {
            auto hitObj_Health = hitObj->get_Component<Health>();
            if (hitObj_Health)
            {
                hitObj_Health->TakeDamage(m_Param.damageValue);
            }
        }
        //********************************************************************************
        // 自身のオブジェクトも、体力があればダメージを与える
        //********************************************************************************
        auto ownerObj = m_pOwner.lock();
        if (ownerObj)
        {
            auto myHealth = ownerObj->get_Component<Health>();
            if (myHealth)
            {
                myHealth->TakeDamage(m_Param.damageValue);
            }
        }


        // ****************************************************
        //				 カメラシェイク
        // ****************************************************
        std::shared_ptr<Camera3D> camera;
        if (camera = Master::m_pDataManager->get_CameraComponent().lock())
        {
            camera->DistanceDecay(3.0f, VEC3(0.1f, 0.2f, 0.1f), hitPos, 500.0f);
        }


        //********************************************************************************
        // 衝突エフェクトを再生する
        //********************************************************************************
        if (!m_Param.impactEffectTag.empty())
        {
            VEC3 normal = _other.hitNormal;
            VEC3 effectRotAxis = VEC3(0.0f);
            float effectRotAngle = 0;
            Tool::GetRotationFromNormal(normal, VEC3(0.0f, 0.0f, -1.0f), effectRotAxis, effectRotAngle);
            int effectHandle = Master::m_pEffectManager->PlayEffect(m_Param.impactEffectTag, hitPos);
            Master::m_pEffectManager->SetRotationEffect(effectHandle, effectRotAxis, effectRotAngle);
            Master::m_pEffectManager->SetScaleEffect(effectHandle, m_Param.impactEffectScale);
        }

        //********************************************************************************
        // 衝突サウンドを再生する
        //********************************************************************************
        if (!m_Param.impactSoundTag.empty())
        {
            Master::m_pSoundManager->Play_RandPitch_3D(m_Param.impactSoundTag, hitPos, 500.0f, 100.0f);
        }
    }

}