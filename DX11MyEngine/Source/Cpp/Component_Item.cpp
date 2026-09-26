#include "pch.h"
#include "CollisionInfo.h"
#include "GameObject.h"
#include "Component_Item.h"
#include "Component_Faction.h"
#include "Component_Health.h"
#include "Component_BoxCollider.h"
#include "Component_Physics.h"

using namespace UtilityData;
using namespace VECTOR4;
using namespace VECTOR3;

//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//*----------------------------------------------------------------------------------------
Item::Item(std::weak_ptr<GameObject> pOwner, int updateRank) : 
	IComponent(pOwner,updateRank),
	m_ItemType(ITEM_TYPE::RECOVERY_SMALL),
	m_Timer(0.0f)
{

}

//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
Item::~Item()
{

}

//*---------------------------------------------------------------------------------------
//*【?】初期化
//*
//* [引数]
//* RendererEngine& : 描画エンジンの参照
//*
//* [返値]
//* void
//*----------------------------------------------------------------------------------------
void Item::Start(RendererEngine& renderer)
{
	m_pTransform = m_pOwner.lock()->get_Transform().lock().get();
	m_pPhysics = m_pOwner.lock()->get_Component<Physics>().get();
	m_pBoxCollider = m_pOwner.lock()->get_Component<BoxCollider>().get();
}

//*---------------------------------------------------------------------------------------
//*【?】更新
//*
//* [引数]
//* RendererEngine& : 描画エンジンの参照
//*
//* [返値]
//* void
//*----------------------------------------------------------------------------------------
void Item::Update(RendererEngine& renderer)
{
	//float deltaTime = Master::m_pTimeManager->get_DeltaTime();
	//m_Timer += deltaTime;
	//VEC3 crntPos = m_pTransform->get_VEC3ToPos();
	//
	//// ふわふわする（0～1）
	////crntPos.y += (sinf(m_Timer) * 0.5f) + 0.5f;
	////crntPos.y += 1.0f; // そのままだと下にめり込んでしまうため

	//// ほぼ停止状態の際は物理コンポーネントを停止し、
	//// コライダーをstaticにする
	//if (m_pPhysics->get_Velocity().LengthSq() < 0.001f)
	//{
	//	//m_pPhysics->set_IsEnable(false);
	//	//m_pBoxCollider->set_IsStatic(true);
	//}

	//m_pTransform->set_Pos(crntPos);
}


//*---------------------------------------------------------------------------------------
//*【?】衝突処理
//*
//* [引数]
//* &other : 衝突相手の情報
//*
//* [返値]
//* void
//*----------------------------------------------------------------------------------------
void Item::OnTriggerEnter(const PhysicsData::CollisionInfo& _other)
{
	auto hitObj = _other.hitObject.lock();
	if (!hitObj){
		return;
	}

	if (auto faction = hitObj->get_Component<Faction>())
	{
		if (faction->get_Faction() == FACTION::PLAYER)
		{
			// アイテムの種別ごとの処理
			switch (m_ItemType)
			{
			case UtilityData::ITEM_TYPE::RECOVERY_SMALL:ApplyRecovery(hitObj.get(),0.15f); break;	// 回復 - 小 15%
			case UtilityData::ITEM_TYPE::RECOVERY_LARGE:ApplyRecovery(hitObj.get(),0.3f); break;	// 回復 - 大 30%
			case UtilityData::ITEM_TYPE::ARMOR:			AddArmor(hitObj.get());		break;	// アーマー
			case UtilityData::ITEM_TYPE::WEAPON:		AddWeapon(hitObj.get());	break;	// 武器箱
			default:break;
			}

			VEC3 pos = m_pTransform->get_VEC3ToPos();
			//*****************************************************************************************
			//						アイテム取得音再生
			//*****************************************************************************************
			Master::m_pSoundManager->Play_3D(SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::ITEM_GET), pos, 500.0f);

			// プールへ返す
			m_pOwner.lock()->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
		}
	}
}


//*---------------------------------------------------------------------------------------
//*【?】衝突処理
//*
//* [引数]
//* &other : 衝突相手の情報
//*
//* [返値]
//* void
//*----------------------------------------------------------------------------------------
void Item::OnCollisionEnter(const PhysicsData::CollisionInfo& _other)
{

}

//*---------------------------------------------------------------------------------------
//*【?】回復処理
//*		最大値 * _rate
//*
//* [引数]
//* *_pObj : プレイヤーオブジェクトのポインタ
//* _rate : 回復率
//* 
//* [返値]
//* void
//*----------------------------------------------------------------------------------------
void Item::ApplyRecovery(class GameObject* _pPlayerObj, float _rate)
{
	if (auto health = _pPlayerObj->get_Component<Health>())
	{
		health->set_RecoveryHP(health->get_MaxHP() * _rate);
	}
}


//*---------------------------------------------------------------------------------------
//*【?】武器取得処理
//*
//* [引数]
//* *_pObj : プレイヤーオブジェクトのポインタ
//* 
//* [返値]
//* void
//*----------------------------------------------------------------------------------------
void Item::AddWeapon(class GameObject* _pPlayerObj)
{

}

//*---------------------------------------------------------------------------------------
//*【?】アーマー取得処理
//*
//* [引数]
//* *_pObj : プレイヤーオブジェクトのポインタ
//* 
//* [返値]
//* void
//*----------------------------------------------------------------------------------------
void Item::AddArmor(class GameObject* _pPlayerObj)
{

}
