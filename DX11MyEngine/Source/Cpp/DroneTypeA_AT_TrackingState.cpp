#include "pch.h"
#include "Component_EnemyController.h"
#include "DroneTypeA_StateHeader.h"
#include "Component_MoveLogic.h"
#include "GameObject.h"

using namespace VECTOR3;
using namespace VECTOR2;
using namespace UtilityData;
using namespace EnemyData;
using namespace PhysicsData;

//*---------------------------------------------------------------------------------------
//* @:DroneTypeA_AT_TrackingState Class 
//*【?】開始
//* 引数：1.EnemyController
//* 返値：void
//*----------------------------------------------------------------------------------------
void DroneTypeA_AT_TrackingState::OnEnter(class EnemyController* pOwner)
{
	pOwner->set_MoveSpeed(MOVE_SPEED * Master::m_pDataManager->get_EnemyDifficultyFactor()._moveSpeedRate);
	// 追従
	pOwner->set_MoveLogicState(MOVE_BEHAVIOUR_TYPE::HOMING);

	// 追跡時間
	m_TrackingDuration = Master::m_pRandomManager->GetFloatRandom(TRACKING_TIME_MIN, TRACKING_TIME_MAX);

	// 攻撃可能距離
	m_AttackPossibleRange = Master::m_pRandomManager->GetFloatRandom(ATTACK_POSSIBLE_RANGE_MIN, ATTACK_POSSIBLE_RANGE_MAX);


	// イージングをかける
	auto move = pOwner->get_MoveLogicComponent();
	move->set_LinearVelocityControl(true);
	move->StartSpeedEasing(
		0.0f,
		1.0f,
		m_TrackingDuration,
		Tool::Easing::EASE_TYPE::IN_SIN);
}

//*---------------------------------------------------------------------------------------
//* @:DroneTypeA_AT_TrackingState Class 
//*【?】終了
//* 引数：1.EnemyController
//* 返値：void
//*----------------------------------------------------------------------------------------
void DroneTypeA_AT_TrackingState::OnExit(class EnemyController* pOwner)
{
	pOwner->clear_StateTimer();
}

//*---------------------------------------------------------------------------------------
//* @:DroneTypeA_AT_TrackingState Class 
//*【?】更新
//* 引数：1.EnemyController
//* 返値：void
//*----------------------------------------------------------------------------------------
int DroneTypeA_AT_TrackingState::Update(class EnemyController* pOwner)
{
	auto targetObj = pOwner->get_Target();
	auto myObj = pOwner->get_OwnerObj().lock();


	// 目標が居なくなったら待機状態へ戻る
	if (targetObj == nullptr || myObj == nullptr)
	{
		MessageBoxA(NULL, "ターゲットがいません", "Ant_PT_MoveState", MB_OK);
		assert(false);
	}
	else
	{
		// 共通処理
		int commonRes = DroneTypeA_CommonStateProcess::CommonProcess(pOwner);
		if (commonRes != -1)
		{
			return commonRes;
		}
		auto myTransform = pOwner->get_TransformComponent();
		VEC3 myPos = myTransform->get_VEC3ToPos();			// 自分の位置

		////
		//// ステート名
		////
		//auto stateName = magic_enum::enum_name(DRONE_TYPE_A_STATE::DRONE_TYPE_A_STATE_ACTIVE_TRACKING);
		//Master::m_pDirectWriteManager->DrawString3D(stateName.data(), myPos, "White_20_STD");

		//=========================================================================================
		//
		//						追従期間を終えたら、アクティブ時の移動または待機ステートへ
		//
		//=========================================================================================
		if (pOwner->get_StateTimer() > m_TrackingDuration)
		{
			// trueなら移動、falseなら待機
			bool isMoving = Master::m_pRandomManager->GetBoolRandom();

			if (isMoving)
			{
				// 移動ステートへ
				return DRONE_TYPE_A_STATE::DRONE_TYPE_A_STATE_ACTIVE_MOVE;
			}
			else
			{
				// 待機ステートへ
				return DRONE_TYPE_A_STATE::DRONE_TYPE_A_STATE_ACTIVE_IDLE;
			}
		}

		auto targetTransform = targetObj->get_Transform().lock();

		/* 追いかけ移動を行う */
		VEC3 targetPos = targetTransform->get_VEC3ToPos();	// 目標位置
		VEC3 targetDir = (targetPos - myPos).Normalize();	// 目標方向
		targetPos.y += 20.0f;	// 直接ターゲットの位置にすると、突進していってしまうため、少し上に

		// 親の移動コンポーネントを使い、移動処理を行う
		MoveParam movePram;
		movePram._moveSpeed = pOwner->get_MoveSpeed();
		movePram._maxSpeed = pOwner->get_MoveSpeed();
		movePram._turnSpeed = 0.1f;
		movePram._targetPos = targetPos;
		movePram._movementMode = MoveParam::MOVEMENT_MODE::FLY;
		movePram._isPhysicsMove = true;
		auto move = pOwner->get_MoveLogicComponent();
		move->set_MoveParam(movePram);	// 移動ロジックにパラメータを渡す

		//=========================================================================================
		//
		//						攻撃可能距離になったら、レーザー攻撃ステートへ
		//
		//=========================================================================================
		if (VEC3::Distance(myPos, targetPos) < m_AttackPossibleRange)
		{
			return DRONE_TYPE_A_STATE::DRONE_TYPE_A_STATE_ACTIVE_ATTACK;
		}
	}

	return DRONE_TYPE_A_STATE::DRONE_TYPE_A_STATE_ACTIVE_TRACKING;
};