#include "pch.h"
#include "Ant_StateHeader.h"
#include "Component_EnemyController.h"
#include "Component_MoveLogic.h"
#include "GameObject.h"

using namespace VECTOR3;
using namespace VECTOR2;
using namespace EnemyData;
using namespace UtilityData;


//*---------------------------------------------------------------------------------------
//* @:Ant_AT_TrackingState Class 
//*【?】開始
//* 引数：1.EnemyController
//* 返値：void
//*----------------------------------------------------------------------------------------
void Ant_AT_TrackingState::OnEnter(class EnemyController *pOwner)
{
	pOwner->set_IsAnim(true);
	pOwner->set_MoveSpeed(MOVE_SPEED * Master::m_pDataManager->get_EnemyDifficultyFactor()._moveSpeedRate);
	// アニメーション速度
	pOwner->set_AnimSpeed(ANIM_SPEED);
	// 追従
	pOwner->set_MoveLogicState(MOVE_BEHAVIOUR_TYPE::HOMING);

	// 追跡時間
	m_TrackingDuration = Master::m_pRandomManager->GetFloatRandom(TRACKING_TIME_MIN, TRACKING_TIME_MAX);

	// 攻撃可能距離
	m_AttackPossibleRange = Master::m_pRandomManager->GetFloatRandom(ATTACK_POSSIBLE_RANGE_MIN, ATTACK_POSSIBLE_RANGE_MAX);
}

//*---------------------------------------------------------------------------------------
//* @:Ant_AT_TrackingState Class 
//*【?】終了
//* 引数：1.EnemyController
//* 返値：void
//*----------------------------------------------------------------------------------------
void Ant_AT_TrackingState::OnExit(class EnemyController *pOwner)
{
	pOwner->clear_StateTimer();
}

//*---------------------------------------------------------------------------------------
//* @:Ant_AT_TrackingState Class 
//*【?】更新
//* 引数：1.EnemyController
//* 返値：void
//*----------------------------------------------------------------------------------------
int Ant_AT_TrackingState::Update(class EnemyController *pOwner)
{
	auto targetObj = pOwner->get_Target();
	auto myObj = pOwner->get_OwnerObj().lock();

	//if (pOwner->get_IsGrounded() == false)
	//{
	//	return ANT_STATE::ANT_STATE_PATROL_IDLE;
	//}

	// 目標が居なくなったら待機状態へ戻る
	if (targetObj == nullptr || myObj == nullptr)
	{
		pOwner->ChangeState(ANT_STATE::ANT_STATE_PATROL_IDLE);
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
		//						追従期間を終えたら、アクティブ時の移動ステートへ
		//
		//=========================================================================================
		if (pOwner->get_StateTimer() > m_TrackingDuration)
		{
			return ANT_STATE::ANT_STATE_ACTIVE_MOVE;
		}

		auto targetTransform = targetObj->get_Transform().lock();
		auto myTransform = pOwner->get_TransformComponent();

		/* 追いかけ移動を行う */
		VEC3 targetPos = targetTransform->get_VEC3ToPos();	// 目標位置
		VEC3 myPos = myTransform->get_VEC3ToPos();			// 自分の位置
		VEC3 targetDir = (targetPos - myPos).Normalize();	// 目標方向
		targetDir.y = 0.0f;

		// 親の移動コンポーネントを使い、移動処理を行う
		MoveParam movePram;
		movePram._moveSpeed = pOwner->get_MoveSpeed();
		movePram._maxSpeed = pOwner->get_MoveSpeed();
		movePram._turnSpeed = 0.1f;
		movePram._targetPos = targetPos;
		movePram._isPhysicsMove = true;
		auto move = pOwner->get_MoveLogicComponent();
		move->set_MoveParam(movePram);	// 移動ロジックにパラメータを渡す

		
		//=========================================================================================
		//
		//						攻撃可能距離になったら、酸攻撃ステートへ
		//
		//=========================================================================================
		if (VEC3::Distance(myPos, targetPos) < m_AttackPossibleRange)
		{
			return ANT_STATE::ANT_STATE_ACTIVE_ATTACK_ACID;
		}

		// 移動ベクトルを渡す
		//pOwner->set_MoveVelocity(targetDir * pOwner->get_MoveSpeed());
	}

	return ANT_STATE::ANT_STATE_ACTIVE_TRACKING;
}

