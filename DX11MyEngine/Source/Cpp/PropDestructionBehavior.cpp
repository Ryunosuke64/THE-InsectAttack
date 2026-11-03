#include "pch.h"
#include "DestructionBehavior_Header.h"
#include "Component_RigidBody.h"
#include "Component_MapObject.h"
#include "Component_Collider.h"

using namespace VECTOR3;

//*---------------------------------------------------------------------------------------
//*【?】開始
//*
//* [引数]
//* *object  :
//* &hitPos  :
//* &hitDir  :
//*
//* [返値]
//* なし
//*----------------------------------------------------------------------------------------
void PropDestructionBehavior::Start(
    GameObject* object,
    const VECTOR3::VEC3& hitPos,
    const VECTOR3::VEC3& hitDir)
{
    auto mapObject = 
        object->get_Component<MapObject>();
    auto rigidBody =
        object->get_Component<RigidBody>();


    if (rigidBody && mapObject)
    {
        // アセットデータの取り出し
        std::string assetID = mapObject->get_AssetID();
        const auto assetData = Master::m_pMapManager->FindMapObjectAssetData(assetID);
        
        // 物理データの取り出し
        const auto physicsData = Master::m_pMapManager->FindPhysicsDataPreset(assetData.physicsPresetID);

        rigidBody->SetEnabled(true);                        // 物理挙動ON
        rigidBody->SetMass(physicsData.rigidBody.mass);     // 質量設定

        // 吹っ飛ばす
        rigidBody->AddImpulse(          
            hitDir * IMPULSE_POWER); 
    }

    int smoke_effectHandle = Master::m_pEffectManager->PlayEffect("Smoke_03");
    Master::m_pEffectManager->SetPositionEffect(smoke_effectHandle,hitPos);
    Master::m_pEffectManager->SetScaleEffect(smoke_effectHandle, VEC3(2.0f));


    // 破壊開始
    m_CrntState = STATE::DESTROYED;

    m_pRigidBody = object->get_Component<RigidBody>();
}


//*---------------------------------------------------------------------------------------
//*【?】更新
//*
//* [引数]
//* deltaTime : デルタタイム
//*
//* [返値]
//* なし
//*----------------------------------------------------------------------------------------
void PropDestructionBehavior::Update(float deltaTime)
{

    switch (m_CrntState)
    {
        //*****************************************************************************************
        //						生存中
        //*****************************************************************************************
    case PropDestructionBehavior::STATE::ALIVE:
        // 生存中は何もしない
        return;

        //*****************************************************************************************
        //						崩壊中
        //*****************************************************************************************
    case PropDestructionBehavior::STATE::DESTROYED:
        //
        // 生存時間を越えたら、当たり判定を消して裏世界へ落とす
        //
        if (m_ElapsedTime >= TO_UNDERWORLD_TIME)
        {
            auto rb = m_pRigidBody.lock();
            if (rb)
            {
                // 停止していれば、物理シミュレーション再開
                rb->SetEnabled(true);

                // 衝突判定を消し、裏世界へ落とす
                rb->SetCollisionBitMask_And_MyCategory_FromRigidBody(
                    0, UtilityData::COLLISION_CATEGORY::PROP
                );
			
                // 裏世界ステート
                m_CrntState = STATE::UNDERWORLD;
            }
        }
        break;
        //*****************************************************************************************
        //						裏世界へ
        //*****************************************************************************************
    case PropDestructionBehavior::STATE::UNDERWORLD:
        //
        // デストロイタイムを過ぎたら、終了
        //
        if (m_ElapsedTime >= DESTROY_TIME)
        {
            m_IsFinished = true;
        }
        break;
    default:
        break;
    }

    m_ElapsedTime += deltaTime;
}

//*---------------------------------------------------------------------------------------
//*【?】破壊が終了したか
//*
//* [引数] 
//* なし 
//* [返値]
//* 終了したか
//*----------------------------------------------------------------------------------------
bool PropDestructionBehavior::IsFinished() const
{
    return m_IsFinished;
}
