#include "pch.h"
#include "CollisionInfo.h"
#include "GameObject.h"
#include "ConstantGameData.h"
#include "Component_Item.h"
#include "Component_Faction.h"
#include "Component_Health.h"
#include "Component_BoxCollider.h"
#include "Component_Physics.h"
#include "Component_RigidBody.h"
#include "Component_MissionInventory.h"

using namespace UtilityData;
using namespace GameData;
using namespace VECTOR4;
using namespace VECTOR3;

namespace
{
	const float RATE_POINT_EFFECT_SCALE = 0.01f;	// ポイントアイテムのエフェクトの大きさを決めるための係数
}

//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//*----------------------------------------------------------------------------------------
Item::Item(std::weak_ptr<GameObject> pOwner, int updateRank) : 
	IComponent(pOwner,updateRank),
	m_ItemType(ITEM_TYPE::RECOVERY_SMALL),
	m_SuctionElapsedTime(0.0f),
	m_EffectHandle(-1)
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
	m_pRigidBody = m_pOwner.lock()->get_Component<RigidBody>().get();
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
	float deltaTime = Master::m_pTimeManager->get_DeltaTime();

	if (m_pPlayerObj.expired())
	{
		// プレイヤーオブジェクトを取得
		auto playerObj = Master::m_pGameObjectManager->get_ObjectByTag("Player");
		if (playerObj)
		{
			m_pPlayerObj = playerObj;
		}
	}

	VEC3 myCrntPos = m_pTransform->get_VEC3ToPos();

	if (auto playerObj = m_pPlayerObj.lock())
	{
		const auto playerTransform = playerObj->get_TransformConst();
		VEC3 playerPos = playerTransform->get_VEC3ToPos();

		float distSq = VEC3::DistanceSq(myCrntPos, playerPos);
		
		if (distSq <= SUCTION_DISTANCE * SUCTION_DISTANCE)
		{
			m_SuctionElapsedTime += deltaTime;

			float t = m_SuctionElapsedTime / 2.0f;
			t = std::clamp(t, 0.0f, 1.0f);
			VEC3 newPos = VEC3::Lerp(myCrntPos, playerPos, t);
			m_pRigidBody->Teleport(newPos);

			//VEC3 dir = playerPos - myCrntPos;
			//dir = dir.Normalize();
			//m_pRigidBody->SetLinearVelocity(dir * 10.0f);

			VEC3 crntScale = m_pTransform->get_VEC3ToScale();
			VEC3 newScale = VEC3::Lerp(crntScale, VEC3(0.0f), t);
			m_pTransform->set_Scale(newScale);
		}
	}

	// ポイントアイテムのエフェクト再生
	if (m_ItemType == ITEM_TYPE::POINT)
	{
		// 再生されていないなら、エフェクトを再生
		if (!Master::m_pEffectManager->IsPlayingEffect(m_EffectHandle))
		{
			m_EffectHandle = Master::m_pEffectManager->PlayEffect("PointItem", myCrntPos);

			// エフェクトの大きさをポイント値に応じて設定
			Master::m_pEffectManager->SetScaleEffect(m_EffectHandle, VEC3(FLOAT_CAST(m_PointValue * RATE_POINT_EFFECT_SCALE)));
		}
		else
		{
			Master::m_pEffectManager->SetPositionEffect(m_EffectHandle, myCrntPos);
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
			case UtilityData::ITEM_TYPE::RECOVERY_SMALL:ApplyRecovery(hitObj.get(), RATE_MIN_RECOVERY); break;	// 回復 - 小 15%
			case UtilityData::ITEM_TYPE::RECOVERY_LARGE:ApplyRecovery(hitObj.get(), RATE_BIG_RECOVERY); break;	// 回復 - 大 30%
			case UtilityData::ITEM_TYPE::ARMOR:			AddArmor(hitObj.get());		break;						// アーマー
			case UtilityData::ITEM_TYPE::WEAPON:		AddWeapon(hitObj.get());	break;						// 武器箱
			case UtilityData::ITEM_TYPE::POINT:			AddPoint(hitObj.get(), m_PointValue); break;			// ポイント
			default:break;
			}

			VEC3 pos = m_pTransform->get_VEC3ToPos();
			//*****************************************************************************************
			//						アイテム取得音再生
			//*****************************************************************************************
			Master::m_pSoundManager->Play_3D(SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::ITEM_GET), pos, 500.0f);

			// プールへ返す
			m_pOwner.lock()->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);


			// ポイントアイテムの場合のエフェクト再生
			if (m_ItemType == ITEM_TYPE::POINT)
			{
				int pointGetEffectHandle = Master::m_pEffectManager->PlayEffect("PointItemGet", pos);
				float scale = std::clamp(FLOAT_CAST(m_PointValue * RATE_POINT_EFFECT_SCALE), 1.0f, 3.0f);	// 大きさの補正

				// エフェクトの大きさをポイント値に応じて設定
				Master::m_pEffectManager->SetScaleEffect(
					pointGetEffectHandle,
					VEC3(scale));

				// エフェクト停止
				Master::m_pEffectManager->StopEffect(m_EffectHandle);
			}
		}
	}
}


//*---------------------------------------------------------------------------------------
//*【?】パラメータのリセット
//*
//* [引数] なし
//* [返値] なし
//*----------------------------------------------------------------------------------------
void Item::ResetParam()
{
	m_SuctionElapsedTime = 0.0f;
	m_PointValue = 0;
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
	if (auto inventory = _pPlayerObj->get_Component<MissionInventory>())
	{
		inventory->AddWeapon();
	}
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
	if (auto inventory = _pPlayerObj->get_Component<MissionInventory>())
	{
		inventory->AddArmor();
	}
}

//*---------------------------------------------------------------------------------------
//*【?】ポイントの取得処理
//*
//* [引数]
//* *_pObj : プレイヤーオブジェクトのポインタ
//* _value : ポイント値
//* 
//* [返値]
//* void
//*----------------------------------------------------------------------------------------
void Item::AddPoint(class GameObject* _pPlayerObj, int value)
{
	if (auto inventory = _pPlayerObj->get_Component<MissionInventory>())
	{
		inventory->AddPoint(value);
	}
}
