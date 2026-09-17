#include "pch.h"
#include "Component_EnemyController.h"
#include "Component_Physics.h"
#include "Ant_StateHeader.h"
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
//* @:Ant_AT_HitStunState Class 
//*【?】開始
//* 引数：1.EnemyController
//* 返値：void
//*----------------------------------------------------------------------------------------
void Ant_AT_HitStunState::OnEnter(class EnemyController* pOwner)
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
	Master::m_pSoundManager->Play_3D(SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::ENEMY_ANT_HIT01), pos, SOUND_HIT_RADIUS);
	
	// ****************************************************
	//				 ヒットエフェクト
	// ****************************************************
	SpawnHitEffect("AntHit_01", pos, rot, 1.0f);

	VEC3 decalRot = VEC3(1.57f, Tool::RandRange(0.0f, 6.14f), 0.0f);
	SpawnHitDecal("Decal_Ant_Splash", pos, decalRot, VEC3(6.0f, 6.0f, 1.0f));


}

//*---------------------------------------------------------------------------------------
//* @:Ant_AT_HitStunState Class 
//*【?】終了
//* 引数：1.EnemyController
//* 返値：void
//*----------------------------------------------------------------------------------------
void Ant_AT_HitStunState::OnExit(class EnemyController* pOwner)
{
	m_StunTimer = 0.0f;
	pOwner->clear_StateTimer();
}

//*---------------------------------------------------------------------------------------
//* @:Ant_AT_HitStunState Class 
//*【?】更新
//* 引数：1.EnemyController
//* 返値：void
//*----------------------------------------------------------------------------------------
int Ant_AT_HitStunState::Update(class EnemyController* pOwner)
{
	// 共通処理
	int commonRes = Ant_CommonStateProcess::CommonProcess(pOwner);
	if (commonRes == ANT_STATE::ANT_STATE_ACTIVE_DEAD)
	{
		return commonRes;
	}
	// 再度スタン状態に
	else if (commonRes == ANT_STATE::ANT_STATE_ACTIVE_HIT_STUN)
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
		Master::m_pSoundManager->Play_3D(SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::ENEMY_ANT_HIT01), pos, SOUND_HIT_RADIUS);

		// ****************************************************
		//				 ヒットエフェクト
		// ****************************************************
		VEC3 decalRot = VEC3(1.57f, Tool::RandRange(0.0f, 6.14f), 0.0f);
		SpawnHitEffect("AntHit_01", pos, rot, 1.0f);
		SpawnHitDecal("Decal_Ant_Splash", pos, decalRot, VEC3(6.0f, 6.0f, 1.0f));
	}


	float deltaTime = Master::m_pTimeManager->get_DeltaTime();
	m_StunTimer += deltaTime;

	if (m_StunTimer > STUN_DURATION)
	{
		return ANT_STATE::ANT_STATE_ACTIVE_MOVE;
	}

	return ANT_STATE::ANT_STATE_ACTIVE_HIT_STUN;
}

void Ant_AT_HitStunState::SpawnHitEffect(const std::string& effectTag, const VECTOR3::VEC3& pos, const VECTOR3::VEC3& rot, const VECTOR3::VEC3& scale)
{
	int hit_handle = Master::m_pEffectManager->PlayEffect(effectTag);
	Master::m_pEffectManager->SetScaleEffect(hit_handle, scale);
	Master::m_pEffectManager->SetPositionEffect(hit_handle, pos.x, pos.y + 2.0f, pos.z);	// 位置が足元になってしまってるので、少し上に補正
	Master::m_pEffectManager->SetRotationEffect(hit_handle, rot.x, rot.y, rot.z);
}

void Ant_AT_HitStunState::SpawnHitDecal(const std::string& materialTag, const VECTOR3::VEC3& pos, const VECTOR3::VEC3& rot, const VECTOR3::VEC3& scale)
{

	auto matPtr = Master::m_pResourceManager->FindMaterial(materialTag);
	SetupMaterialInfo matInfo[1];
	matInfo[0].Index = 0;
	matInfo[0].pMaterialData = matPtr;

	CreateDecalInfo decal;
	decal.pRenderer = m_pRenderer;
	decal.Type = UTILITY_MESH_TYPE::CUBE;
	decal.MatNum = 1;
	decal.MaterialData = matInfo;
	decal.IsActive = false;
	decal.ShaderType = SHADER_TYPE::DEFERRED_STD_DECAL;
	decal.IsNormalMap = false;
	decal.IsDynamic = true;

	auto obj = MeshFactory::CreateDecal(decal);
	obj->get_Component<DecalRenderer>()->Start(*m_pRenderer);
	auto decalTransform = obj->get_Transform().lock();
	decalTransform->set_Pos(pos);
	decalTransform->set_Scale(scale);
	decalTransform->set_RotateToRad(rot);
	obj->set_Tag("Ant_Splash");
	auto timer = obj->add_Component<TimerDestruction>();
	timer->set_LifeTime(8.0f);  // 生存時間
}
