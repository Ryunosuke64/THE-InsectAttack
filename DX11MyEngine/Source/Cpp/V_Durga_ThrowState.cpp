#include "pch.h"
#include "Component_VehicleController.h"
#include "V_Durga_StateHeader.h"
#include "Component_MoveLogic.h"
#include "Component_SkinnedMeshAnimator.h"
#include "Component_RigidBody.h"
#include "Component_Destructible.h"
#include "GameObject.h"

using namespace VECTOR4;
using namespace VECTOR3;
using namespace VECTOR2;
using namespace UtilityData;
using namespace VehicleData;

//*---------------------------------------------------------------------------------------
//* @:V_Durga_ThrowState Class 
//*【?】開始
//* 引数：1.Vehicle
//* 返値：void
//*----------------------------------------------------------------------------------------
void V_Durga_ThrowState::OnEnter(class VehicleController* pOwner)
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

	// 投げ飛ばしアニメーションに切り替える
	pOwner->ChangeAnimation(INT_CAST(DURGA_ANIM_ID::THROW), 0.1f);

	// 持ち上げているオブジェクトを取得
	m_pLiftObjects = pOwner->get_TargetObjects();

	m_IsThrow = false;
}

//*---------------------------------------------------------------------------------------
//* @:V_Durga_ThrowState Class 
//*【?】終了
//* 引数：1.Vehicle
//* 返値：void
//*----------------------------------------------------------------------------------------
void V_Durga_ThrowState::OnExit(class VehicleController* pOwner)
{
	pOwner->get_AnimatorComponent()->set_IsPlaybackPaused(true);

	// オブジェクトを空に
	pOwner->clear_TargetObhects();
	m_pLiftObjects.clear();
}

//*---------------------------------------------------------------------------------------
//* @:V_Durga_ThrowState Class 
//*【?】更新
//* 引数：1.Vehicle
//* 返値：void
//*----------------------------------------------------------------------------------------
int V_Durga_ThrowState::Update(class VehicleController* pOwner)
{
	const auto& input = pOwner->get_InputResult_Robot();
	auto animator = pOwner->get_AnimatorComponent();
	auto transform = pOwner->get_TransformComponent();
	VEC3 crntPos = transform->get_VEC3ToPos();			// 現在の座標
	VEC3 forward = transform->get_Forward();			// 現在の向き
	const float AnimDuration = animator->get_AnimDurationSeconds(INT_CAST(DURGA_ANIM_ID::THROW));	// アニメーションの再生時間を取得

	//=========================================================================================
	//
	//				投げるまではオブジェクトを手の位置に追従させる
	// 
	//=========================================================================================
	// 持ち上げているオブジェクトがあるか
	// まだ、投げ飛ばしていないか
	if (!m_pLiftObjects.empty() && m_IsThrow == false)
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
		VEC3 targetPos = handCenterPos /*+ m_GrabOffset*/;

		// ターゲットたち
		for (auto& target : m_pLiftObjects)
		{
			auto targetObj = target.lock();
			if (!targetObj)
			{
				continue;
			}
			auto targetTransform = targetObj->get_Transform().lock();


			auto liftObj_RigidBody = targetObj->get_Component<RigidBody>();
			if (liftObj_RigidBody)
			{
				liftObj_RigidBody->SetWorldTransform(targetPos, VEC4(0.0f, 0.0f, 0.0f, 1.0f));
			}

			//=========================================================================================
			//
			//				丁度投げ飛ばすアニメーションになったところで、実際にオブジェクトを投げる
			// 
			//=========================================================================================
			if (AnimDuration - 2.0 <= animator->get_AnimProcTime())
			{
				if (liftObj_RigidBody)
				{
					const float ImpulsePower = 300.0f;
					VEC3 impulse = forward * ImpulsePower;
					liftObj_RigidBody->SetEnabled(true);
					liftObj_RigidBody->SetMass(1.0f);
					liftObj_RigidBody->AddImpulse(impulse);
					liftObj_RigidBody->AddAngularImpulse(impulse);

					// 投げ終わった
					m_IsThrow = true;
				}
			}
		}
	}

	//=========================================================================================
	//
	//				アニメーションが終了したら、待機ステートへ
	// 
	//=========================================================================================
	if (AnimDuration - 0.1 <= animator->get_AnimProcTime())
	{
		return DURGA_V_STATE::DURGA_V_STATE_IDLE;
	}


	return DURGA_V_STATE::DURGA_V_STATE_THROW;
}
