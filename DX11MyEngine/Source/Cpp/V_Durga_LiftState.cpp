#include "pch.h"
#include "Component_VehicleController.h"
#include "V_Durga_StateHeader.h"
#include "Component_MoveLogic.h"
#include "Component_SkinnedMeshAnimator.h"
#include "Component_RigidBody.h"
#include "Component_MeshCollider.h"
#include "GameObject.h"

using namespace VECTOR3;
using namespace VECTOR4;
using namespace UtilityData;
using namespace VehicleData;


//*---------------------------------------------------------------------------------------
//* @:V_Durga_LiftState Class 
//*【?】開始
//* 引数：1.Vehicle
//* 返値：void
//*----------------------------------------------------------------------------------------
void V_Durga_LiftState::OnEnter(class VehicleController* pOwner)
{
	// 移動ベクトルは0
	pOwner->set_MoveSpeed(0.0f);
	pOwner->set_IsPlayAnim(true);
	//pOwner->get_RigidBodyComponent()->ResetPhysics();

	// 速度制御をなくす
	auto move = pOwner->get_MoveLogicComponent();
	move->set_LinearVelocityControl(false);
	move->ClearSpeedEasing();
	pOwner->set_MoveLogicState(MOVE_BEHAVIOUR_TYPE::NONE);
	pOwner->get_AnimatorComponent()->set_IsPlaybackPaused(false);

	// 持ち上げアニメーションに切り替える
	pOwner->ChangeAnimation(INT_CAST(DURGA_ANIM_ID::LIFT), 0.5f);



	auto animator = pOwner->get_AnimatorComponent();
	auto transform = pOwner->get_TransformComponent();
	VEC3 crntPos = transform->get_VEC3ToPos();			// 現在の座標
	VEC3 forward = transform->get_Forward();			// 向いてる方向
	m_HandR_LocalMat = animator->get_BoneLocalWorldMatrix("Hand.R");
	m_HandL_LocalMat = animator->get_BoneLocalWorldMatrix("Hand.L");
	const float GrabRange = 10.0f;						// 手の届く範囲
	const float GrabCollisionRadius = 50.0f;			// 掴むための当たり判定の半径
	VEC3 grabCollisionSpherePos = crntPos + forward * GrabRange;	// 手の届く範囲の球の中心座標


	// 掴む対象
	unsigned mask = 
		UINT_CAST(COLLISION_CATEGORY::ENEMY)|
		UINT_CAST(COLLISION_CATEGORY::PLAYER)|
		UINT_CAST(COLLISION_CATEGORY::DESTRUCTION_BUILDING);

	//
	// 球判定チェックで対象を取得する
	//
	auto grabTargets = Master::m_pPhysicsEngine->CheckSphere(grabCollisionSpherePos, GrabCollisionRadius, mask);

	float minDistance = FLT_MAX;
	for (auto& target : grabTargets)
	{
		auto targetTransform = target.lock()->get_Transform().lock();
		VEC3 targetPos = targetTransform->get_VEC3ToPos();
		float distance = (targetPos - crntPos).Length();
		//if (distance < minDistance)
		{
			minDistance = distance;
			m_pLiftTargets.push_back(target);
		}
	}

	if (!m_pLiftTargets.empty())
	{
		// get_BoneLocalWorldMatrix() はモデル内の行列なので、
		// 機体のワールド行列を掛けてワールド座標にする
		auto handWorld_R = m_HandR_LocalMat * transform->get_WorldMtx();
		auto handWorld_L = m_HandL_LocalMat * transform->get_WorldMtx();

		VEC3 handR_Pos = VEC3::FromXMVECTOR(handWorld_R.r[3]);
		VEC3 handL_Pos = VEC3::FromXMVECTOR(handWorld_L.r[3]);
		VEC3 handCenterPos = (handR_Pos + handL_Pos) * 0.5f;

		for (auto& target : m_pLiftTargets)
		{
			auto targetObj = target.lock();

			//
			// 持ち上げているオブジェクトとして他のステートからも参照できるよう登録する
			//
			pOwner->add_TargetObject(targetObj);

			auto rb = targetObj->get_Component<RigidBody>();
			if (rb)
			{
				rb->SetEnabled(false);
			}

			// 掴む対象がある場合、手の位置に合わせる
			auto targetTransform = targetObj->get_Transform().lock();
			//targetTransform->set_OffsetWorldTransfomationMatrix(m_HandR_LocalMat);

			//
			// 掴む際に衝突が続いてしまうので、除外する
			//
			auto collider = targetObj->get_Component < MeshCollider >();
			if (collider)
			{
				collider->remove_CollisionBitMask(COLLISION_CATEGORY::VEHICLE);
			}
		}
	}
}

//*---------------------------------------------------------------------------------------
//* @:V_Durga_LiftState Class 
//*【?】終了
//* 引数：1.Vehicle
//* 返値：void
//*----------------------------------------------------------------------------------------
void V_Durga_LiftState::OnExit(class VehicleController* pOwner)
{
	pOwner->get_AnimatorComponent()->set_IsPlaybackPaused(true);
	m_pLiftTargets.clear();
}

//*---------------------------------------------------------------------------------------
//* @:V_Durga_LiftState Class 
//*【?】更新
//* 引数：1.Vehicle
//* 返値：void
//*----------------------------------------------------------------------------------------
int V_Durga_LiftState::Update(class VehicleController* pOwner)
{
	const auto& input = pOwner->get_InputResult_Robot();
	auto animator = pOwner->get_AnimatorComponent();
	auto transform = pOwner->get_TransformComponent();
	auto rigidBody = pOwner->get_RigidBodyComponent();
	const float AnimDuration = animator->get_AnimDurationSeconds(INT_CAST(DURGA_ANIM_ID::LIFT));	// アニメーションの再生時間を取得
	float animProcTime = animator->get_AnimProcTime();

	//=========================================================================================
	//
	//				丁度投げ持ち上げるアニメーションになったところで、
	//				実際にオブジェクトを手に追従させる
	// 
	//=========================================================================================
	if (AnimDuration - 1.5 <= animProcTime)
	{
		// 掴む対象がある場合、手の位置に合わせる
		m_HandR_LocalMat = animator->get_BoneLocalWorldMatrix("Hand.R");
		m_HandL_LocalMat = animator->get_BoneLocalWorldMatrix("Hand.L");


		// get_BoneLocalWorldMatrix() はモデル内の行列なので、
		// 機体のワールド行列を掛けてワールド座標にする
		auto handWorld_R = m_HandR_LocalMat * transform->get_WorldMtx();
		auto handWorld_L = m_HandL_LocalMat * transform->get_WorldMtx();
		VEC3 handR_Pos = VEC3::FromXMVECTOR(handWorld_R.r[3]);
		VEC3 handL_Pos = VEC3::FromXMVECTOR(handWorld_L.r[3]);
		VEC3 handCenterPos = (handR_Pos + handL_Pos) * 0.5f;	// 左右の手のちょうど真ん中の位置に合わせる
		VEC3 targetPos = handCenterPos;

		//
		// 持ち上げられているオブジェクトたち
		//
		for (auto& target : m_pLiftTargets)
		{
			if (auto targetObj = target.lock())
			{
				auto rb = targetObj->get_Component<RigidBody>();
				if (rb)
				{
					//rb->SetEnabled(false);
					rb->SetWorldTransform(targetPos, VEC4(0.0f, 0.0f, 0.0f, 1.0f));
				}
			}
		}
	}


	//=========================================================================================
	//
	//						入力チェック
	//
	//=========================================================================================
	const float turnInput =
		(input.isRotR ? 1.0f : 0.0f) -
		(input.isRotL ? 1.0f : 0.0f);

	//=========================================================================================
	//
	//				アニメーションが終了したら、待機ステートへ
	// 
	//=========================================================================================
	if (AnimDuration - 0.1f <= animProcTime)
	{
		pOwner->get_AnimatorComponent()->set_IsPlaybackPaused(true);

		// 何も掴めなかった
		if (m_pLiftTargets.empty())
		{
			pOwner->get_AnimatorComponent()->set_IsPlaybackPaused(false);

			return DURGA_V_STATE::DURGA_V_STATE_IDLE;
		}
	}

	if (input.isThrow)
	{
		return DURGA_V_STATE::DURGA_V_STATE_THROW;
	}

	return DURGA_V_STATE::DURGA_V_STATE_LIFT;
}
