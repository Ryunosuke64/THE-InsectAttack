#include "pch.h"
#include "DestructionBehavior_Header.h"
#include "Component_RigidBody.h"
#include "Component_MapObject.h"
#include "MapObjectFactory.h"

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
void ReplaceModelDestructionBehavior::Start(
    GameObject* object,
    const VECTOR3::VEC3& hitPos,
    const VECTOR3::VEC3& hitDir)
{
    auto mapObject =
        object->get_Component<MapObject>();
    //auto rigidBody =
    //    object->get_Component<RigidBody>();

    if (/*rigidBody && */mapObject)
    {
        // アセットデータの取り出し
        std::string assetID = mapObject->get_AssetID();
        const auto assetData = Master::m_pMapManager->FindMapObjectAssetData(assetID);

        // 物理データの取り出し
        const auto physicsData = Master::m_pMapManager->FindPhysicsDataPreset(assetData.physicsPresetID);

        //rigidBody->SetEnabled(true);                        // 物理挙動ON
        //rigidBody->SetMass(physicsData.rigidBody.mass);     // 質量設定

        //// 吹っ飛ばす
        //rigidBody->AddImpulse(
        //    hitDir * IMPULSE_POWER);

        const auto transform = object->get_TransformConst();

        Map::MO_TransformData transformData;
        transformData.position = transform->get_VEC3ToPos();
        transformData.rotation = transform->get_VEC3ToRotateToRad();
        transformData.scale = transform->get_VEC3ToScale();
        if (!MapObjectFactory::CreateBuildingDebris(assetID, transformData))
        {
            assert(false);
        }
    }

    // 破壊開始
    m_IsDestructionStart = true;
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
void ReplaceModelDestructionBehavior::Update(float deltaTime)
{
    if (!m_IsDestructionStart)
    {
        return;
    }

    m_DestroyTimer += deltaTime;

    // デストロイタイムを過ぎたら、終了
    //if (m_DestroyTimer >= DESTROY_TIME)
    {
        m_IsFinished = true;
    }
}

//*---------------------------------------------------------------------------------------
//*【?】破壊が終了したか
//*
//* [引数] 
//* なし 
//* [返値]
//* 終了したか
//*----------------------------------------------------------------------------------------
bool ReplaceModelDestructionBehavior::IsFinished() const
{
    return m_IsFinished;
}
