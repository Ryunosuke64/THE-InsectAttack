#include "pch.h"
#include "Component_MapObjectDebris.h"
#include "Component_Health.h"
#include "Component_RigidBody.h"
#include "Component_MeshCollider.h"
#include "Component_ModelMeshRenderer.h"
#include "Component_ModelMeshResource.h"
#include "ConstantMapData.h"
#include "BuildingStateFactory.h"

using namespace Map;
using namespace VECTOR3;;

//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//*----------------------------------------------------------------------------------------
MapObjectDebris::MapObjectDebris(std::weak_ptr<GameObject> pOwner, int updateRank) :
	IComponent(pOwner, updateRank),
	m_IsPlaySmokeEffect(false),
	m_CrntState(STATE::ALIVE),
	m_LifeTime(0.0f),
	m_ElapsedTime(0.0f)
{

}

//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
MapObjectDebris::~MapObjectDebris()
{
}

//*---------------------------------------------------------------------------------------
//*【?】初期化
//* 
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void MapObjectDebris::Start(RendererEngine& renderer)
{
	auto owner = m_pOwner.lock();
	auto transform = owner->get_Transform().lock();

	auto modelResource = owner->get_Component<ModelMeshResource>();

	// ここから追加
	auto fragmentModelData = modelResource->get_ModelData().lock();
	if (!fragmentModelData)
	{
		assert(false);
		return;
	}

	// Assimp取り込み後の頂点範囲の中心を、重心の近似として使う
	const auto& boundsCenter =
		fragmentModelData->get_LocalBounds().Center;
	const auto& boundsExtents =
		fragmentModelData->get_LocalBounds().Extents;

	const VEC3 fragmentCenter(
		boundsCenter.x,
		boundsCenter.y,
		boundsCenter.z
	);

	const VEC3 fragmentExtents(
		boundsExtents.x,
		boundsExtents.y,
		boundsExtents.z
	);


	if (transform)
	{
		VEC3 pos = transform->get_VEC3ToPos();
		float effectSize = (fragmentExtents.x + fragmentExtents.y + fragmentExtents.z) / 3.0f;	// 平均値
		effectSize *= 0.5;
		float effectSizes = fragmentExtents.Length();
		int effectHandle = Master::m_pEffectManager->PlayEffect("Smoke_04");
		Master::m_pEffectManager->SetPositionEffect(effectHandle, pos + fragmentCenter);
		Master::m_pEffectManager->SetScaleEffect(effectHandle, effectSize);
	}

	m_pRigidBody = owner->get_Component<RigidBody>();
	m_pCollider = owner->get_Component <MeshCollider>();
}

//*---------------------------------------------------------------------------------------
//*【?】更新
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void MapObjectDebris::Update(RendererEngine& renderer)
{
	float deltaTime = Master::m_pTimeManager->get_DeltaTime();


	switch (m_CrntState)
	{
		//*****************************************************************************************
		//						生存中
		//*****************************************************************************************
	case MapObjectDebris::STATE::ALIVE:
		//
		// 生存時間を越えたら、当たり判定を消して裏世界へ落とす
		//
		if (m_ElapsedTime >= 10.0f)
		{
			auto collider = m_pCollider.lock();
			auto rb = m_pRigidBody.lock();
			if (collider && rb)
			{
				rb->SetEnabled(true);				// 停止していれば、物理シミュレーション再開
				collider->set_CollisionBitMask(0);	// 衝突判定を消し、裏世界へ落とす

				// 裏世界ステート
				m_CrntState = STATE::UNDERWORLD;
			}
		}
		break;
		//*****************************************************************************************
		//						裏世界
		//*****************************************************************************************
	case MapObjectDebris::STATE::UNDERWORLD:
		//
		// 生存時間を越えたら、オブジェクトを削除する
		//
		if (m_ElapsedTime >= 15.0f)
		{
			// 親オブジェクトの削除フラグを立てる
			m_pOwner.lock()->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_DELETE);
		}
		break;
	default:
		break;
	}

	m_ElapsedTime += deltaTime;
}


//*---------------------------------------------------------------------------------------
//*【?】衝突処理
//*
//* [引数]
//* &_other : 衝突相手の情報
//* [返値]なし
//*----------------------------------------------------------------------------------------
void MapObjectDebris::OnCollisionEnter(const PhysicsData::CollisionInfo& _other)
{

}
