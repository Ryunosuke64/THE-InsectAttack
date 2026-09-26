#include "pch.h"
#include "Component_EnemyController.h"
#include "Component_MoveLogic.h"
#include "Ant_StateHeader.h"
#include "GameObject.h"
#include "CollisionInfo.h"
#include "Component_Collider.h"

using namespace PhysicsData;
using namespace VECTOR3;
using namespace VECTOR2;
using namespace UtilityData;
using namespace DirectX;
using namespace EnemyData;

//*---------------------------------------------------------------------------------------
//* @:Ant_PT_MoveState Class 
//*【?】開始
//* 引数：1.EnemyController
//* 返値：void
//*----------------------------------------------------------------------------------------
void Ant_PT_MoveState::OnEnter(class EnemyController* pOwner)
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
//* @:Ant_PT_MoveState Class 
//*【?】終了
//* 引数：1.EnemyController
//* 返値：void
//*----------------------------------------------------------------------------------------
void Ant_PT_MoveState::OnExit(class EnemyController* pOwner)
{
	pOwner->clear_StateTimer();
}

//*---------------------------------------------------------------------------------------
//* @:Ant_PT_MoveState Class 
//*【?】更新
//* 引数：1.EnemyController
//* 返値：void
//*----------------------------------------------------------------------------------------
int Ant_PT_MoveState::Update(class EnemyController* pOwner)
{
	auto target = pOwner->get_Target();	// 目標オブジェクト
	if (target == nullptr || pOwner->get_OwnerObj().expired())
	{
		MessageBoxA(NULL,"ターゲットがいません","Ant_PT_MoveState",MB_OK);
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
		//						移動期間を終えたら、待機ステートへ
		//
		//=========================================================================================
		if (pOwner->get_StateTimer() > m_MoveDuration)
		{
			return ANT_STATE::ANT_STATE_PATROL_IDLE;
		}

		float deltaTime = Master::m_pTimeManager->get_DeltaTime();

		auto targetTransform = target->get_Transform().lock();
		VEC3 targetPos = targetTransform->get_VEC3ToPos();	// 目標位置

		// オーナーオブジェクト
		auto myTransform = pOwner->get_TransformComponent();
		VEC3 myPos = myTransform->get_VEC3ToPos();	// 自分の位置
		VEC3 startPos = pOwner->get_StartPos();

		// 方向転換（半分の時間で）
		if (pOwner->get_StateTimer() > m_MoveDuration * 0.5f && m_IsDirChange == false)
		{
			m_IsDirChange = true;
			m_MoveDir = Master::m_pRandomManager->GetVEC3Random(DIR_RAND_MIN, DIR_RAND_MAX);
			m_MoveDir.y = 0.0f;
		}

		/* 移動範囲外に出ていたら、反射させる */
		if (!VEC3::TargetInTheRange(myPos, startPos, MOVE_RANGE)) {
			VEC3 boundaryNorm = startPos - myPos;
			boundaryNorm.y = 0.0f; // Y成分を無視して平面で考える

			// ゼロ除算回避
			if (boundaryNorm.LengthSq() > 0.0001f) {
				boundaryNorm = boundaryNorm.Normalize();

				// m_MoveDir が外側を向いている（中心方向と逆）時のみ反射させる
				if (VEC3::Dot(m_MoveDir, boundaryNorm) < 0.0f) {
					m_MoveDir = VEC3::Reflect(m_MoveDir, boundaryNorm);
					m_MoveDir.y = 0.0f;
					m_MoveDir = m_MoveDir.Normalize();
				}
			}
		}

		/* 前方に壁があれば、反射させる */
		// 前方トレース
		VEC3 myForward = myTransform->get_Forward();
		int collisionMask = UINT_CAST(COLLISION_CATEGORY::DESTRUCTION_BUILDING);
		CollisionInfo hitInfo;
		CollInData_Ray frontTraceRay;
		frontTraceRay._point = myPos;
		frontTraceRay._dir = myForward * 5.0f;

		if (Master::m_pCollisionManager->CheckRaycast(frontTraceRay, collisionMask, &hitInfo)) {
			VEC3 hitNorm = hitInfo.hitNormal;	// 衝突した面の法線
			hitNorm.y = 0.0f; // 建物の壁が傾いていてもY方向の力は無視する

			if (hitNorm.LengthSq() > 0.0001f) {
				hitNorm = hitNorm.Normalize();

				// m_MoveDir が壁に向かっている時のみ反射させる
				if (VEC3::Dot(m_MoveDir, hitNorm) < 0.0f) {
					m_MoveDir = VEC3::Reflect(m_MoveDir, hitNorm);	// 反射
					m_MoveDir.y = 0.0f;
					m_MoveDir = m_MoveDir.Normalize();
				}
			}
		}


		/* 親の移動コンポーネントを使い、移動処理を行う */
		// 方向を変えて、移動させる
		MoveParam movePram;
		movePram._moveSpeed = pOwner->get_MoveSpeed();
		movePram._maxSpeed = pOwner->get_MoveSpeed();
		movePram._turnSpeed = 0.05f;	// 急に振り向くと変なので、少し優しめに
		movePram._moveDirection = m_MoveDir;
		auto move = pOwner->get_MoveLogicComponent();
		move->set_MoveParam(movePram);	// 移動ロジックにパラメータを渡す


		// 壁のぼり
		//VEC3 myForward = myTransform->get_Forward();

		//// 前方トレース
		//int collisionMask = UINT_CAST(COLLISION_CATEGORY::DESTRUCTION_BUILDING);
		//CollisionInfo hitInfo;
		//CollInData_Ray frontTraceRay;
		//frontTraceRay._point = myPos;
		//frontTraceRay._dir = myForward * 10.0f;
		//// 地面トレース
		//CollInData_Ray groundTraceRay;
		//groundTraceRay._point = myPos;
		//groundTraceRay._dir = (myTransform->get_Up() * -1.0f) * 5.0f;	// 上方向を反転
		//if (Master::m_pCollisionManager->CheckRaycast(frontTraceRay, collisionMask, &hitInfo))
		//{
		//	VEC3 hitNorm = hitInfo.get_HitNormal();	// 衝突した面の法線を上ベクトルにする
		//	VEC3 hitPoint = hitInfo.get_HitPoint();
		//	VEC3 forward = myForward;
		//	float dot = VEC3::Dot(forward, hitNorm);
		//	forward = forward - (hitNorm * dot);
		//	forward = forward.Normalize();

		//	VEC3 right = VEC3::Cross(hitNorm, forward).Normalize();
		//	forward = VEC3::Cross(right, hitNorm).Normalize();
		//	XMMATRIX mat = XMMatrixIdentity();
		//	mat.r[0] = right;    // X軸
		//	mat.r[1] = hitNorm;  // Y軸
		//	mat.r[2] = forward;  // Z軸 

		//	// クォータニオンへ変換
		//	XMVECTOR targetQuat = XMQuaternionRotationMatrix(mat);

		//	auto rotQ = myTransform->get_RotationQuaternion();
		//	rotQ = XMQuaternionSlerp(rotQ, targetQuat, 0.5f);
		//	rotQ = XMQuaternionNormalize(rotQ);
		//	myTransform->set_RotationQuaternion(rotQ);

		//	m_MoveDir = myTransform->get_Forward();
		//}


		VEC3 newPos = myTransform->get_VEC3ToPos();

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
	return ANT_STATE::ANT_STATE_PATROL_MOVE;
}
