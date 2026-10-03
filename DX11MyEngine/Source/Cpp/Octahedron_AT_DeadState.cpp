#include "pch.h"
#include "Component_EnemyController.h"
#include "Component_BoxCollider.h"
#include "Component_MeshCollider.h"
#include "Component_Physics.h"
#include "Component_DecalRenderer.h"
#include "Component_RigidBody.h"
#include "Component_TimerDestruction.h"
#include "Octahedron_StateHeader.h"
#include "GameObject.h"
#include "RendererEngine.h"
#include "MeshFactory.h"
#include "ResourceManager.h"

using namespace DirectX;
using namespace VECTOR3;
using namespace VECTOR2;
using namespace UtilityData;
using namespace EnemyData;
using namespace Tool;

//*---------------------------------------------------------------------------------------
//* @:Octahedron_AT_DeadState Class 
//*【?】開始
//* 引数：1.EnemyController
//* 返値：void
//*----------------------------------------------------------------------------------------
void Octahedron_AT_DeadState::OnEnter(class EnemyController* pOwner)
{
	// 移動ベクトルは0
	pOwner->set_MoveVelocity(VEC3());

	auto myTransform = pOwner->get_OwnerObj().lock()->get_Transform().lock();
	VEC3 pos = myTransform->get_VEC3ToPos();


	// ****************************************************
	//				アイテム出現
	// ****************************************************
	Master::m_pItemManager->SpawnItemRand(DROP_ITEM_MIN, DROP_ITEM_MAX, pos, 10.0f);
	Master::m_pItemManager->SpawnPointItem(pos, 1000);	// ポイントアイテムをスポーン

	// ****************************************************
	//				 死亡音再生
	// ****************************************************
	Master::m_pSoundManager->Play_3D(SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::ENEMY_OCTAHEDRON_DEAD), pos, SOUND_DEAD_RADIUS);

	// 死亡エフェクトの作成
	SpawnDeadEffect(pOwner, 15.0f);

	pOwner->set_MoveLogicState(MOVE_BEHAVIOUR_TYPE::NONE);

	VECTOR3::VEC3 knockbackDir;   // 爆心地から外（衝突オブジェクト）へ向かうベクトル
	// 少し上（Y軸）に向かせることで、綺麗な放物線を描いて吹き飛ばせる
	knockbackDir.x = Master::m_pRandomManager->GetFloatRandom(-10.0f, 10.0f);
	knockbackDir.y += 10.0f;
	knockbackDir.z = Master::m_pRandomManager->GetFloatRandom(-10.0f, 10.0f);

	// 物理コンポーネントの設定
	auto rigidBody = pOwner->get_RigidBodyComponent();
	rigidBody->SetMass(1.0f);
	rigidBody->SetGravity(VEC3(0.0f, -9.8f, 0.0f));
	rigidBody->SetRestitution(0.5f);
	rigidBody->AddImpulse(knockbackDir);
}

//*---------------------------------------------------------------------------------------
//* @:Octahedron_AT_DeadState Class 
//*【?】終了
//* 引数：1.EnemyController
//* 返値：void
//*----------------------------------------------------------------------------------------
void Octahedron_AT_DeadState::OnExit(class EnemyController* pOwner)
{
	pOwner->clear_StateTimer();
}

//*---------------------------------------------------------------------------------------
//*【?】更新
//*
//* [引数]
//* *pOwner : 親となるエネミーコントローラー
//* [返値]なし
//*----------------------------------------------------------------------------------------
int Octahedron_AT_DeadState::Update(class EnemyController* pOwner)
{
	if (pOwner->get_OwnerObj().expired())
	{
		MessageBoxA(NULL, "ターゲットがいません", "Octahedron_AT_DeadState", MB_OK);
		assert(false);
	}
	else
	{
		// 共通処理
		int commonRes = Octahedron_CommonStateProcess::CommonProcess(pOwner);
		// 同じ場合も返す
		if (commonRes != -1 &&
			commonRes != OCTAHEDRON_STATE::OCTAHEDRON_STATE_ACTIVE_DEAD &&
			commonRes != OCTAHEDRON_STATE::OCTAHEDRON_STATE_ACTIVE_HIT_STUN) {
			return commonRes;
		}

		float deltaTime = Master::m_pTimeManager->get_DeltaTime();
		auto myTransform = pOwner->get_TransformComponent();
		VEC3 crntPos = myTransform->get_VEC3ToPos();					// 現在の座標
		VEC3 crntRot = myTransform->get_VEC3ToLocal_RotateToRad();		// 現在のオイラー
		XMVECTOR crntRotQ = myTransform->get_RotationQuaternion();		// 現在のクオータニオン
		float timer = pOwner->get_StateTimer();


		//*****************************************************************************************
		//						エフェクト再生
		//				崩れていく際に、炎が出ている感じにする
		//*****************************************************************************************
		m_FrameCounter++;

		if (m_FrameCounter % 30 == 0)
		{
			VEC3 explosionRot;
			VEC3 explosionScale = 4.0f;
			explosionRot.x = Master::m_pRandomManager->GetFloatRandom(-G_PI_F, G_PI_F);
			explosionRot.y = Master::m_pRandomManager->GetFloatRandom(-G_PI_F, G_PI_F);
			explosionRot.z = Master::m_pRandomManager->GetFloatRandom(-G_PI_F, G_PI_F);
			int handle = Master::m_pEffectManager->PlayEffect("Explosion_02");
			Master::m_pEffectManager->SetPositionEffect(handle, crntPos.x, crntPos.y, crntPos.z);
			Master::m_pEffectManager->SetRotationEffect(handle, explosionRot.x, explosionRot.y, explosionRot.z);
			Master::m_pEffectManager->SetScaleEffect(handle, explosionScale.x, explosionScale.y, explosionScale.z);
		}


		//*****************************************************************************************
		//						デストロイ
		//*****************************************************************************************
		if (timer > DELETE_TIME)
		{
			auto rigidBody = pOwner->get_RigidBodyComponent();
			//rigidBody->set_IsEnable(false);
			pOwner->get_OwnerObj().lock()->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_DELETE);


			// ****************************************************
			//				 爆発音再生
			// ****************************************************
			Master::m_pSoundManager->Play_3D(SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::EXPLOSION01), crntPos, SOUND_DEAD_RADIUS);
			//auto myTransform = pOwner->get_OwnerObj().lock()->get_Transform().lock();
			//VEC3 pos = myTransform->get_VEC3ToPos();

			// 大爆発エフェクト
			SpawnDeadEffect(pOwner, 40.0f);
			SpawnDeadFragmentEffect(pOwner, 15.0f);
		}
	}

	return OCTAHEDRON_STATE::OCTAHEDRON_STATE_ACTIVE_DEAD;
}

//*---------------------------------------------------------------------------------------
//*【?】死亡エフェクトの生成
//*
//* [引数]
//* *pOwner : 親となるエネミーコントローラー
//* _effectSize : エフェクトのサイズ
//* [返値]なし
//*----------------------------------------------------------------------------------------
void Octahedron_AT_DeadState::SpawnDeadEffect(class EnemyController* pOwner, float _effectSize)
{
	auto myTransform = pOwner->get_TransformComponent();
	VEC3 pos = myTransform->get_VEC3ToPos();

	// 死亡エフェクト
	int handle = Master::m_pEffectManager->PlayEffect("Explosion_Acid_01");
	float deadEffectScale = _effectSize;
	Master::m_pEffectManager->SetScaleEffect(handle, deadEffectScale, deadEffectScale, deadEffectScale);
	Master::m_pEffectManager->SetPositionEffect(handle, pos.x, pos.y, pos.z);
}

//*---------------------------------------------------------------------------------------
//*【?】爆散破片エフェクトの生成
//*
//* [引数]
//* *pOwner : 親となるエネミーコントローラー
//* _effectSize : エフェクトのサイズ
//* [返値]なし
//*----------------------------------------------------------------------------------------
void Octahedron_AT_DeadState::SpawnDeadFragmentEffect(class EnemyController* pOwner, float _effectSize)
{
	auto myTransform = pOwner->get_TransformComponent();
	VEC3 pos = myTransform->get_VEC3ToPos();

	// 死亡エフェクト
	int handle = Master::m_pEffectManager->PlayEffect("Fragment_Octahedron");
	float deadEffectScale = _effectSize;
	Master::m_pEffectManager->SetScaleEffect(handle, deadEffectScale, deadEffectScale, deadEffectScale);
	Master::m_pEffectManager->SetPositionEffect(handle, pos.x, pos.y, pos.z);
}