#include "pch.h"
#include "Component_EnemyController.h"
#include "DroneTypeA_StateHeader.h"
#include "Component_RigidBody.h"
#include "Component_MoveLogic.h"
#include "GameObject.h"

using namespace VECTOR3;
using namespace VECTOR2;
using namespace UtilityData;
using namespace EnemyData;

//*---------------------------------------------------------------------------------------
//* @:DroneTypeA_AT_IdleState Class 
//*【?】開始
//* 引数：1.EnemyController
//* 返値：void
//*----------------------------------------------------------------------------------------
void DroneTypeA_AT_IdleState::OnEnter(class EnemyController* pOwner)
{
	// 移動ベクトルは0
	pOwner->set_MoveSpeed(0.0f);
	pOwner->set_IsAnim(false);
	pOwner->set_MoveLogicState(MOVE_BEHAVIOUR_TYPE::NONE);
	m_IdleDuration = Master::m_pRandomManager->GetFloatRandom(IDLE_MIN_TIME, IDLE_MAX_TIME);
}

//*---------------------------------------------------------------------------------------
//* @:Octahedron_AT_IdleState Class 
//*【?】終了
//* 引数：1.EnemyController
//* 返値：void
//*----------------------------------------------------------------------------------------
void DroneTypeA_AT_IdleState::OnExit(class EnemyController* pOwner)
{
	pOwner->clear_StateTimer();
}

//*---------------------------------------------------------------------------------------
//* @:Octahedron_AT_IdleState Class 
//*【?】更新
//* 引数：1.EnemyController
//* 返値：void
//*----------------------------------------------------------------------------------------
int DroneTypeA_AT_IdleState::Update(class EnemyController* pOwner)
{
	auto target = pOwner->get_Target();	// 目標オブジェクト
	if (target == nullptr || pOwner->get_OwnerObj().expired())
	{
		MessageBoxA(NULL, "ターゲットがいません", "DroneTypeA_AT_IdleState", MB_OK);
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
		VEC3 crntPos = myTransform->get_VEC3ToPos();					// 現在の座標
		//
		// ステート名
		//
		auto stateName = magic_enum::enum_name(DRONE_TYPE_A_STATE::DRONE_TYPE_A_STATE_ACTIVE_IDLE);
		Master::m_pDirectWriteManager->DrawString3D(stateName.data(), crntPos, "White_20_STD");

		//=========================================================================================
		//
		//						待機期間を終えたら、移動または、追跡ステートへ
		//
		//=========================================================================================
		if (pOwner->get_StateTimer() > m_IdleDuration)
		{
			// trueなら追跡、falseなら移動
			bool isTracking = Master::m_pRandomManager->GetBoolRandom();

			if (isTracking)
			{
				// 追跡ステートへ
				return DRONE_TYPE_A_STATE::DRONE_TYPE_A_STATE_ACTIVE_TRACKING;
			}
			else
			{
				// 移動ステートへ
				return DRONE_TYPE_A_STATE::DRONE_TYPE_A_STATE_ACTIVE_MOVE;
			}
		}
	}

	return DRONE_TYPE_A_STATE::DRONE_TYPE_A_STATE_ACTIVE_IDLE;
};