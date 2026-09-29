#include "pch.h"
#include "Component_EnemyController.h"
#include "Component_MoveLogic.h"
#include "Ant_StateHeader.h"
#include "GameObject.h"
#include "CollisionInfo.h"
#include "Component_Collider.h"

using namespace VECTOR3;
using namespace VECTOR2;
using namespace UtilityData;
using namespace EnemyData;

//*---------------------------------------------------------------------------------------
//* @:Ant_AT_MoveState Class 
//*【?】開始
//* 引数：1.EnemyController
//* 返値：void
//*----------------------------------------------------------------------------------------
void Ant_AT_MoveState::OnEnter(class EnemyController* pOwner)
{
	pOwner->set_IsAnim(true);
	pOwner->set_MoveSpeed(MOVE_SPEED * Master::m_pDataManager->get_EnemyDifficultyFactor()._moveSpeedRate);

	// アニメーション速度
	pOwner->set_AnimSpeed(ANIM_SPEED);

	// 直線移動
	pOwner->set_MoveLogicState(MOVE_BEHAVIOUR_TYPE::LINEAR);

	// 移動時間
	m_MoveDuration = Master::m_pRandomManager->GetFloatRandom(MOVE_TIME_MIN, MOVE_TIME_MAX);

	// 移動方向
	m_MoveDir = Master::m_pRandomManager->GetVEC3Random(DIR_RAND_MIN, DIR_RAND_MAX);
	m_MoveDir.y = 0.0f;

	m_IsDirChange = false;
}

//*---------------------------------------------------------------------------------------
//* @:Ant_AT_MoveState Class 
//*【?】終了
//* 引数：1.EnemyController
//* 返値：void
//*----------------------------------------------------------------------------------------
void Ant_AT_MoveState::OnExit(class EnemyController* pOwner)
{
	pOwner->clear_StateTimer();
}

//*---------------------------------------------------------------------------------------
//* @:Ant_AT_MoveState Class 
//*【?】更新
//* 引数：1.EnemyController
//* 返値：void
//*----------------------------------------------------------------------------------------
int Ant_AT_MoveState::Update(class EnemyController* pOwner)
{
	// 共通処理
	int commonRes = Ant_CommonStateProcess::CommonProcess(pOwner);
	if (commonRes != -1)
	{
		return commonRes;
	}

	//=========================================================================================
	//
	//						移動期間を終えたら、追従ステートへ
	//
	//=========================================================================================
	if (pOwner->get_StateTimer() > m_MoveDuration)
	{
		return ANT_STATE::ANT_STATE_ACTIVE_TRACKING;
	}

	float deltaTime = Master::m_pTimeManager->get_DeltaTime();

	// オーナーオブジェクト
	auto myTransform = pOwner->get_TransformComponent();
	VEC3 myPos = myTransform->get_VEC3ToPos();	// 自分の位置
	VEC3 startPos = pOwner->get_StartPos();

	//// 方向転換
	//if (pOwner->get_StateTimer() > m_MoveDuration * 0.5f && m_IsDirChange == false)
	//{
	//	m_IsDirChange = true;
	//	m_MoveDir = Master::m_pRandomManager->GetVEC3Random(DIR_RAND_MIN, DIR_RAND_MAX);
	//	m_MoveDir.y = 0.0f;
	//}

	/* 親の移動コンポーネントを使い、移動処理を行う */
	// 方向を変えて、移動させる
	MoveParam movePram;
	movePram._moveSpeed = pOwner->get_MoveSpeed();
	movePram._maxSpeed = pOwner->get_MoveSpeed();
	movePram._turnSpeed = 0.05f;	// 急に振り向くと変なので、少し優しめに
	movePram._isPhysicsMove = true;
	movePram._moveDirection = m_MoveDir;
	auto move = pOwner->get_MoveLogicComponent();
	move->set_MoveParam(movePram);	// 移動ロジックにパラメータを渡す


	return ANT_STATE::ANT_STATE_ACTIVE_MOVE;
}
