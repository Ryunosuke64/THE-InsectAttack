#include "pch.h"
#include "Component_EnemyController.h"
#include "Component_BoxCollider.h"
#include "Component_Physics.h"
#include "Component_DecalRenderer.h"
#include "Component_TimerDestruction.h"
#include "Ant_StateHeader.h"
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
//* @:Ant_AT_DeadState Class 
//*【?】開始
//* 引数：1.EnemyController
//* 返値：void
//*----------------------------------------------------------------------------------------
void Ant_AT_DeadState::OnEnter(class EnemyController* pOwner)
{
	pOwner->clear_StateTimer();

	// 移動ベクトルは0
	pOwner->set_MoveVelocity(VEC3());

	// アニメーションの停止
	pOwner->set_IsAnim(false);

	// 死亡エフェクトの作成
	SpawnDeadEffect(pOwner);

	pOwner->set_MoveLogicState(MOVE_BEHAVIOUR_TYPE::NONE);

	auto myTransform = pOwner->get_OwnerObj().lock()->get_Transform().lock();
	m_StartRotQ = myTransform->get_RotationQuaternion();

	float angle = Master::m_pRandomManager->GetFloatRandom(Tool::G_PI_2_F, Tool::G_PI_F);
	angle = Tool::G_PI_F;

	// 前方を軸に180度回転させる
	XMVECTOR axis = myTransform->get_Forward();

	XMVECTOR targetQuat = XMQuaternionRotationAxis(axis, angle);
	m_TargetRotQ = XMQuaternionMultiply(m_StartRotQ, targetQuat);

	VECTOR3::VEC3 knockbackDir;   // 爆心地から外（衝突オブジェクト）へ向かうベクトル

	// 少し上（Y軸）に向かせることで、綺麗な放物線を描いて吹き飛ばせる
	knockbackDir.x = Master::m_pRandomManager->GetFloatRandom(-6.0f, 6.0f);
	knockbackDir.y += 6.0f;
	knockbackDir.z = Master::m_pRandomManager->GetFloatRandom(-6.0f, 6.0f);

	auto physics = pOwner->get_PhysicsComponent();
	physics->AddImpulse(knockbackDir);

	// コライダーの判定をオフに
	//pOwner->get_OwnerObj().lock()->get_Component<BoxCollider>()->set_IsEnable(false);	
	auto collider = pOwner->get_OwnerObj().lock()->get_Component<BoxCollider>();
	collider->set_Center(VEC3(0.0f, -0.5f, 0.0f));
	collider->set_Size(VEC3(1.0f, 1.0f, 1.0f));
}

//*---------------------------------------------------------------------------------------
//* @:Ant_AT_DeadState Class 
//*【?】終了
//* 引数：1.EnemyController
//* 返値：void
//*----------------------------------------------------------------------------------------
void Ant_AT_DeadState::OnExit(class EnemyController* pOwner)
{
	pOwner->clear_StateTimer();
}

//*---------------------------------------------------------------------------------------
//* @:Ant_AT_DeadState Class 
//*【?】更新
//* 引数：1.EnemyController
//* 返値：void
//*----------------------------------------------------------------------------------------
int Ant_AT_DeadState::Update(class EnemyController* pOwner)
{
	if (pOwner->get_OwnerObj().expired())
	{
		MessageBoxA(NULL, "ターゲットがいません", "Ant_PT_MoveState", MB_OK);
		assert(false);
	}
	else
	{
		// 共通処理
		int commonRes = Ant_CommonStateProcess::CommonProcess(pOwner);
		// 同じ場合も返す
		if (commonRes != -1 && 
			commonRes != ANT_STATE::ANT_STATE_ACTIVE_DEAD && 
			commonRes != ANT_STATE::ANT_STATE_ACTIVE_HIT_STUN){
			return commonRes;
		}

		float deltaTime = Master::m_pTimeManager->get_DeltaTime();
		auto myTransform = pOwner->get_OwnerObj().lock()->get_Transform().lock();
		VEC3 crntPos = myTransform->get_VEC3ToPos();					// 現在の座標
		VEC3 crntRot = myTransform->get_VEC3ToLocal_RotateToRad();		// 現在のオイラー
		XMVECTOR crntRotQ = myTransform->get_RotationQuaternion();		// 現在のクオータニオン
		float timer = pOwner->get_StateTimer();


		//*****************************************************************************************
		//						ひっくり返る
		//*****************************************************************************************
		float t = timer / OVERTURN_TIME;
		if (t <= 1.0f) {
			float ease = std::min(Tool::Easing::EaseOutBounce(t), 1.0f);
			crntRotQ = XMQuaternionSlerp(crntRotQ, m_TargetRotQ, ease);	// 球面補間でひっくり返らせる
			myTransform->set_RotationQuaternion(crntRotQ);

			// 跳ねる感じに
			//crntPos.y = ((Tool::Easing::EaseOutBounce(t)*-1.0f) * -3.0f);
			//myTransform->set_Pos(crntPos);
		}


		//*****************************************************************************************
		//						裏世界へ落下する
		//*****************************************************************************************
		if (timer - OVERTURN_TIME > DELETE_TIME)
		{
			auto physics = pOwner->get_PhysicsComponent();

			physics->set_IsEnable(false);
			pOwner->get_OwnerObj().lock()->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_DELETE);
		}
	}

	return ANT_STATE::ANT_STATE_ACTIVE_DEAD;
}


//*---------------------------------------------------------------------------------------
//* @:Ant_AT_DeadState Class 
//*【?】死亡エフェクトの生成
//* 引数：1.EnemyController
//* 返値：void
//*----------------------------------------------------------------------------------------
void Ant_AT_DeadState::SpawnDeadEffect(class EnemyController* pOwner)
{
	auto myTransform = pOwner->get_OwnerObj().lock()->get_Transform().lock();
	VEC3 pos = myTransform->get_VEC3ToPos();

	// ****************************************************
	//				 死亡音再生
	// ****************************************************
	Master::m_pSoundManager->Play_3D(SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::ENEMY_ANT_DEAD), pos, SOUND_DEAD_RADIUS);

	// ****************************************************
	//				アイテム出現
	// ****************************************************
	Master::m_pItemManager->SpawnItemRand(DROP_ITEM_MIN, DROP_ITEM_MAX, pos, 0.0f);

	pOwner->set_IsAnim(false);	// アニメーションを停止

	// 死亡エフェクト
	int handle = Master::m_pEffectManager->PlayEffect("AntDead_01");
	float deadEffectScale = 1.5f;
	Master::m_pEffectManager->SetScaleEffect(handle, deadEffectScale, deadEffectScale, deadEffectScale);
	Master::m_pEffectManager->SetPositionEffect(handle, pos.x, pos.y + 2.0f, pos.z);

	auto matPtr = Master::m_pResourceManager->FindMaterial("Decal_Ant_Splash");
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

	VEC3 scale;
	scale.x = 20.0f;
	scale.y = 20.0f;
	scale.z = 1.0f;

	auto obj = MeshFactory::CreateDecal(decal);
	obj->get_Component<DecalRenderer>()->Start(*m_pRenderer);
	obj->get_Transform().lock()->set_Pos(pos);
	obj->get_Transform().lock()->set_Scale(scale);
	obj->get_Transform().lock()->set_RotateToRad(1.57f, Tool::RandRange(0.0f, 6.14f), 0.0f);
	obj->set_Tag("Ant_Splash");
	auto timer = obj->add_Component<TimerDestruction>();
	timer->set_LifeTime(15.0f);  // 生存時間
}