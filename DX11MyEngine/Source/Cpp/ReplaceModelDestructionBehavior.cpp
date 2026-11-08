#include "pch.h"
#include "DestructionBehavior_Header.h"
#include "Component_RigidBody.h"
#include "Component_MapObject.h"
#include "Component_ImpactDamage.h"
#include "MapObjectFactory.h"

using namespace VECTOR3;

//*---------------------------------------------------------------------------------------
//*【?】開始
//*
//* [引数]
//* *object  : 対象オブジェクト
//* &hitPos  : ヒット位置
//* &hitDir  : ヒット方向
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
    auto rigidBody =
        object->get_Component<RigidBody>();

    const auto transform = object->get_TransformConst();
    VEC3 pos = transform->get_VEC3ToPos();
    VEC3 rot = transform->get_VEC3ToRotateToRad();
    VEC3 scale = transform->get_VEC3ToScale();

    if (rigidBody && mapObject)
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

        Map::MO_TransformData transformData;
        transformData.position = pos;
        transformData.rotation = rot;
        transformData.scale = scale;
        MapObjectFactory::BuildingDebrisGenerateInfo info;
        info.ang_velocity = rigidBody->GetAngularVelocity();    // 現在の速度を引き継ぐ
        info.lin_velocity = rigidBody->GetLinearVelocity();
        auto impactDamage = object->get_Component<ImpactDamage>();
        if (impactDamage)
        {
            info.isImpactDamage = impactDamage->get_IsImpactEnabled();
        }
        if (!MapObjectFactory::CreateBuildingDebris(assetID, transformData, info))
        {
            assert(false);
        }

        // ****************************************************
        //		        破壊サウンド再生
        // ****************************************************
        Master::m_pSoundManager->Play_3D(
            "building.destruction01", pos, 500.0f);

        // ****************************************************
        //		        崩壊サウンドを遅延再生
        // ****************************************************
		const float PLAY_COLLAPSING_SOUND_TIME = 1.7f;  // 崩壊サウンド再生までの時間
        Master::m_pTimeManager->AddTimer(
            PLAY_COLLAPSING_SOUND_TIME,
            [pos]()
            {
                Master::m_pSoundManager->Play_3D(
                    "building.collapsing01", pos, 800.0f);
            });
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

    //
	// 崩壊サウンド再生し、破壊終了
    //
    if(m_ElapsedTime >= PLAY_COLLAPSING_SOUND_TIME)
    {

    }

    // 破壊終了
    m_IsFinished = true;
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
bool ReplaceModelDestructionBehavior::IsFinished() const
{
    return m_IsFinished;
}
