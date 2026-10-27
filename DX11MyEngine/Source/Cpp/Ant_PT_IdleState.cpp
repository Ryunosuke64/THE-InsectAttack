#include "pch.h"
#include "Component_EnemyController.h"
#include "Ant_StateHeader.h"
#include "GameObject.h"

using namespace VECTOR3;
using namespace VECTOR2;
using namespace UtilityData;
using namespace EnemyData;

//*---------------------------------------------------------------------------------------
//* @:Ant_PT_IdleState Class 
//*【?】開始
//* 引数：1.EnemyController
//* 返値：void
//*----------------------------------------------------------------------------------------
void Ant_PT_IdleState::OnEnter(class EnemyController* pOwner)
{
	// 移動ベクトルは0
	pOwner->set_MoveVelocity(VEC3());

	pOwner->set_IsAnim(true);
	pOwner->ChangeAnimation(INT_CAST(ANT01_ANIM_ID::IDLE));
	// アニメーション速度
	pOwner->set_AnimSpeed(ANIM_SPEED);


	pOwner->set_MoveLogicState(MOVE_BEHAVIOUR_TYPE::NONE);
	m_IdleDuration = Master::m_pRandomManager->GetFloatRandom(IDLE_MIN_TIME, IDLE_MAX_TIME);
}

//*---------------------------------------------------------------------------------------
//* @:Ant_PT_IdleState Class 
//*【?】終了
//* 引数：1.EnemyController
//* 返値：void
//*----------------------------------------------------------------------------------------
void Ant_PT_IdleState::OnExit(class EnemyController* pOwner)
{
	pOwner->clear_StateTimer();
}

//*---------------------------------------------------------------------------------------
//* @:Ant_PT_IdleState Class 
//*【?】更新
//* 引数：1.EnemyController
//* 返値：void
//*----------------------------------------------------------------------------------------
int Ant_PT_IdleState::Update(class EnemyController* pOwner)
{
	auto target = pOwner->get_Target();	// 目標オブジェクト
	if (target == nullptr || pOwner->get_OwnerObj().expired())
	{
		MessageBoxA(NULL, "ターゲットがいません", "Ant_PT_MoveState", MB_OK);
		assert(false);
	}
	else
	{
		// 共通処理
		int commonRes = Ant_CommonStateProcess::CommonProcess(pOwner);
		if (commonRes != -1)
		{
			return commonRes;
		}
		
		//=========================================================================================
		//
		//						待機期間を終えたら、移動ステートへ
		//
		//=========================================================================================
		if (pOwner->get_StateTimer() > m_IdleDuration)
		{
			return ANT_STATE::ANT_STATE_PATROL_MOVE;
		}

		auto targetTransform = target->get_Transform().lock();
		VEC3 targetPos = targetTransform->get_VEC3ToPos();	// 目標位置

		auto myTransform = pOwner->get_TransformComponent();

		VEC3 myPos = myTransform->get_VEC3ToPos();	// 自分の位置

		//=========================================================================================
		//
		//						視界内に入ったら追跡ステートへ
		//
		//=========================================================================================
		if (VEC3::TargetInTheSight(myPos, targetPos, myTransform->get_Forward(), SEARCH_FIELD_OF_VIEW_DEG, SEARCH_RANGE))
		{
			return ANT_STATE::ANT_STATE_ACTIVE_TRACKING;
		}
	}

	return ANT_STATE::ANT_STATE_PATROL_IDLE;
}
