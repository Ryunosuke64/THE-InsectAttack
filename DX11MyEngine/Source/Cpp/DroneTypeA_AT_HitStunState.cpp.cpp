#include "pch.h"
#include "Component_EnemyController.h"
#include "Component_Physics.h"
#include "DroneTypeA_StateHeader.h"
#include "GameObject.h"
#include "Component_BoxCollider.h"
#include "Component_DecalRenderer.h"
#include "Component_TimerDestruction.h"
#include "RendererEngine.h"
#include "MeshFactory.h"

using namespace DirectX;
using namespace VECTOR3;
using namespace UtilityData;
using namespace EnemyData;

//*---------------------------------------------------------------------------------------
//* @:DroneTypeA_AT_HitStunState Class 
//*【?】開始
//* 引数：1.EnemyController
//* 返値：void
//*----------------------------------------------------------------------------------------
void DroneTypeA_AT_HitStunState::OnEnter(class EnemyController* pOwner)
{
	auto myTransform = pOwner->get_TransformComponent();
	VEC3 pos = myTransform->get_VEC3ToPos();
	VEC3 rot;
	rot.x = Tool::RandRange(-3.14f, 3.14f);
	rot.y = Tool::RandRange(-3.14f, 3.14f);
	rot.z = Tool::RandRange(-3.14f, 3.14f);

	// 移動ベクトルは0
	pOwner->set_MoveVelocity(VEC3());
	pOwner->set_MoveLogicState(MOVE_BEHAVIOUR_TYPE::NONE);

	// ****************************************************
	//				 被弾音再生
	// ****************************************************
	Master::m_pSoundManager->Play_3D("enemy.ant.hit_01", pos, SOUND_HIT_RADIUS);

	// ****************************************************
	//				 ヒットエフェクト
	// ****************************************************
	SpawnHitEffect("AntHit_01", pos, rot, 1.0f);
}

//*---------------------------------------------------------------------------------------
//* @:DroneTypeA_AT_HitStunState Class 
//*【?】終了
//* 引数：1.EnemyController
//* 返値：void
//*----------------------------------------------------------------------------------------
void DroneTypeA_AT_HitStunState::OnExit(class EnemyController* pOwner)
{
	m_StunTimer = 0.0f;
	pOwner->clear_StateTimer();
}

//*---------------------------------------------------------------------------------------
//* @:DroneTypeA_AT_HitStunState Class 
//*【?】更新
//* 引数：1.EnemyController
//* 返値：void
//*----------------------------------------------------------------------------------------
int DroneTypeA_AT_HitStunState::Update(class EnemyController* pOwner)
{
	// 共通処理
	int commonRes = DroneTypeA_CommonStateProcess::CommonProcess(pOwner);
	if (commonRes == DRONE_TYPE_A_STATE::DRONE_TYPE_A_STATE_ACTIVE_DEAD)
	{
		return commonRes;
	}
	// 再度スタン状態に
	else if (commonRes == DRONE_TYPE_A_STATE::DRONE_TYPE_A_STATE_ACTIVE_HIT_STUN)
	{
		m_StunTimer = 0.0f;

		auto myTransform = pOwner->get_TransformComponent();
		VEC3 pos = myTransform->get_VEC3ToPos();
		VEC3 rot;
		rot.x = Tool::RandRange(-3.14f, 3.14f);
		rot.y = Tool::RandRange(-3.14f, 3.14f);
		rot.z = Tool::RandRange(-3.14f, 3.14f);

		// ****************************************************
		//				 被弾音再生
		// ****************************************************
		Master::m_pSoundManager->Play_3D("enemy.ant.hit_01", pos, SOUND_HIT_RADIUS);

		// ****************************************************
		//				 ヒットエフェクト
		// ****************************************************
		VEC3 decalRot = VEC3(1.57f, Tool::RandRange(0.0f, 6.14f), 0.0f);
		SpawnHitEffect("AntHit_01", pos, rot, 1.0f);
	}


	float deltaTime = Master::m_pTimeManager->get_DeltaTime();
	m_StunTimer += deltaTime;

	if (m_StunTimer > STUN_DURATION)
	{
		return DRONE_TYPE_A_STATE::DRONE_TYPE_A_STATE_ACTIVE_MOVE;
	}

	return DRONE_TYPE_A_STATE::DRONE_TYPE_A_STATE_ACTIVE_HIT_STUN;
}

void DroneTypeA_AT_HitStunState::SpawnHitEffect(const std::string& effectTag, const VECTOR3::VEC3& pos, const VECTOR3::VEC3& rot, const VECTOR3::VEC3& scale)
{
	int hit_handle = Master::m_pEffectManager->PlayEffect(effectTag);
	Master::m_pEffectManager->SetScaleEffect(hit_handle, scale);
	Master::m_pEffectManager->SetPositionEffect(hit_handle, pos.x, pos.y + 2.0f, pos.z);	// 位置が足元になってしまってるので、少し上に補正
	Master::m_pEffectManager->SetRotationEffect(hit_handle, rot.x, rot.y, rot.z);
}

