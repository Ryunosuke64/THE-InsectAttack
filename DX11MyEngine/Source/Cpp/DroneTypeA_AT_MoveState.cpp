#include "pch.h"
#include "Component_EnemyController.h"
#include "DroneTypeA_StateHeader.h"
#include "GameObject.h"
#include "Component_MoveLogic.h"
#include "CollisionInfo.h"
#include "Component_Collider.h"

using namespace VECTOR3;
using namespace VECTOR2;
using namespace UtilityData;
using namespace EnemyData;
using namespace PhysicsData;

//*---------------------------------------------------------------------------------------
//* @:DroneTypeA_AT_MoveState Class 
//*【?】開始
//* 引数：1.EnemyController
//* 返値：void
//*----------------------------------------------------------------------------------------
void DroneTypeA_AT_MoveState::OnEnter(class EnemyController* pOwner)
{
	pOwner->set_MoveSpeed(
		MOVE_SPEED * Master::m_pDataManager->get_EnemyDifficultyFactor()._moveSpeedRate);

	// 直線移動
	pOwner->set_MoveLogicState(MOVE_BEHAVIOUR_TYPE::LINEAR);

	// 移動時間
	m_MoveDuration = Master::m_pRandomManager->GetFloatRandom(MOVE_TIME_MIN, MOVE_TIME_MAX);

	// イージングをかける
	auto move = pOwner->get_MoveLogicComponent();
	move->set_LinearVelocityControl(true);
	move->StartSpeedEasing(
		0.0f,
		1.0f,
		m_MoveDuration,
		Tool::Easing::EASE_TYPE::IN_SIN);

	// 移動方向
	m_MoveDir = Master::m_pRandomManager->GetVEC3Random(DIR_RAND_MIN, DIR_RAND_MAX);
}

//*---------------------------------------------------------------------------------------
//* @:DroneTypeA_AT_MoveState Class 
//*【?】終了
//* 引数：1.EnemyController
//* 返値：void
//*----------------------------------------------------------------------------------------
void DroneTypeA_AT_MoveState::OnExit(class EnemyController* pOwner)
{
	pOwner->clear_StateTimer();
}

//*---------------------------------------------------------------------------------------
//* @:DroneTypeA_AT_MoveState Class 
//*【?】更新
//* 引数：1.EnemyController
//* 返値：void
//*----------------------------------------------------------------------------------------
int DroneTypeA_AT_MoveState::Update(class EnemyController* pOwner)
{
	// 共通処理
	int commonRes = DroneTypeA_CommonStateProcess::CommonProcess(pOwner);
	if (commonRes != -1)
	{
		return commonRes;
	}
	
	auto myTransform = pOwner->get_TransformComponent();
	VEC3 myPos = myTransform->get_VEC3ToPos();	// 自分の位置

	//
	// ステート名
	//
	auto stateName = magic_enum::enum_name(DRONE_TYPE_A_STATE::DRONE_TYPE_A_STATE_ACTIVE_MOVE);
	Master::m_pDirectWriteManager->DrawString3D(stateName.data(), myPos, "White_20_STD");

	//=========================================================================================
	//
	//						移動期間を終えたら、追跡または待機ステートへ
	//
	//=========================================================================================
	if (pOwner->get_StateTimer() > m_MoveDuration)
	{
		bool isTracking = Master::m_pRandomManager->GetBoolRandom();
		if (isTracking)
		{
			// 追跡ステートへ
			return DRONE_TYPE_A_STATE::DRONE_TYPE_A_STATE_ACTIVE_TRACKING;
		}
		else
		{
			// 待機ステートへ
			return DRONE_TYPE_A_STATE::DRONE_TYPE_A_STATE_ACTIVE_IDLE;
		}
	}

	float deltaTime = Master::m_pTimeManager->get_DeltaTime();

	// オーナーオブジェクト

	VEC3 startPos = pOwner->get_StartPos();

	// 高さが低い場合は、上方向に移動する
	if (myPos.y < 10.0f)
	{
		m_MoveDir.y = 0.2f;
	}


	/* 下方に壁があれば、反射させる */
	// 下方トレース
	VEC3 myDownward = VEC3(0.0f, -1.0f, 0.0f);
	unsigned int collisionMask = UINT_CAST(COLLISION_CATEGORY::BUILDING) | UINT_CAST(COLLISION_CATEGORY::DESTRUCTION_BUILDING);
	CollisionInfo hitInfo;
	CollInData_Ray downTraceRay;
	downTraceRay._point = myPos;
	downTraceRay._dir = myDownward * 5.0f;
	unsigned int group = UINT_CAST(COLLISION_CATEGORY::ENEMY);

	if (Master::m_pPhysicsEngine->Raycast(downTraceRay, group, collisionMask, &hitInfo))
	{
		VEC3 hitNorm = hitInfo.hitNormal;	// 衝突した面の法線

		if (hitNorm.LengthSq() > 0.0001f)
		{
			hitNorm = hitNorm.Normalize();

			// m_MoveDir が壁に向かっている時のみ反射させる
			if (VEC3::Dot(m_MoveDir, hitNorm) < 0.0f) 
			{
				m_MoveDir = VEC3::Reflect(m_MoveDir, hitNorm);	// 反射
				m_MoveDir = m_MoveDir.Normalize();
			}
		}
	}

	/* 親の移動コンポーネントを使い、移動処理を行う */
	// 方向を変えて、移動させる
	MoveParam movePram;
	movePram._moveSpeed = pOwner->get_MoveSpeed();
	movePram._maxSpeed = pOwner->get_MoveSpeed();
	movePram._turnSpeed = 0.1f;	// 急に振り向くと変なので、少し優しめに
	movePram._moveDirection = m_MoveDir;
	movePram._movementMode = MoveParam::MOVEMENT_MODE::FLY;
	movePram._isPhysicsMove = true;
	auto move = pOwner->get_MoveLogicComponent();
	move->set_MoveParam(movePram);	// 移動ロジックにパラメータを渡す


	return DRONE_TYPE_A_STATE::DRONE_TYPE_A_STATE_ACTIVE_MOVE;
};