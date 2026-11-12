#include "pch.h"
#include "Component_VehicleController.h"
#include "V_Durga_StateHeader.h"
#include "Component_MoveLogic.h"
#include "Component_SkinnedMeshAnimator.h"
#include "Component_RigidBody.h"
#include "Component_ModelChange.h"
#include "GameObject.h"

using namespace VECTOR4;
using namespace VECTOR3;
using namespace VECTOR2;
using namespace UtilityData;
using namespace VehicleData;

//*---------------------------------------------------------------------------------------
//* @:V_Durga_IdleState Class 
//*【?】開始
//* 引数：1.Vehicle
//* 返値：void
//*----------------------------------------------------------------------------------------
void V_Durga_IdleState::OnEnter(class VehicleController* pOwner)
{
	// 移動ベクトルは0
	pOwner->set_MoveSpeed(0.0f);
	pOwner->set_IsPlayAnim(false);
	//pOwner->get_RigidBodyComponent()->ResetPhysics();

	// 速度制御をなくす
	auto move = pOwner->get_MoveLogicComponent();
	move->set_LinearVelocityControl(false);
	move->ClearSpeedEasing();
	pOwner->set_MoveLogicState(MOVE_BEHAVIOUR_TYPE::NONE);


}

//*---------------------------------------------------------------------------------------
//* @:V_Durga_IdleState Class 
//*【?】終了
//* 引数：1.Vehicle
//* 返値：void
//*----------------------------------------------------------------------------------------
void V_Durga_IdleState::OnExit(class VehicleController* pOwner)
{
}

//*---------------------------------------------------------------------------------------
//* @:V_Durga_IdleState Class 
//*【?】更新
//* 引数：1.Vehicle
//* 返値：void
//*----------------------------------------------------------------------------------------
int V_Durga_IdleState::Update(class VehicleController* pOwner)
{
	const auto& input = pOwner->get_InputResult_Robot();
	auto animator = pOwner->get_AnimatorComponent();
	auto transform = pOwner->get_TransformComponent();
	VEC3 crntPos = transform->get_VEC3ToPos();			// 現在の座標
	const float AnimDuration = animator->get_AnimDurationSeconds(INT_CAST(DURGA_ANIM_ID::WALK_F_ENTER));	// 歩行アニメーションの再生時間を取得

	//=========================================================================================
	//
	//						移動入力があるかチェック
	//
	//=========================================================================================
	if (input.CheckMove() || input.CheckRot())
	{
		return DURGA_V_STATE::DURGA_V_STATE_MOVE;
	}
	//=========================================================================================
	//
	//						持ち上げ入力があるかチェック
	//
	//=========================================================================================
	if (input.isLift)
	{
		return DURGA_V_STATE::DURGA_V_STATE_LIFT;
	}
	if (input.isFlight)
	{
		return DURGA_V_STATE::DURGA_V_STATE_FLIGHT_CHARGE;
	}
	auto rigidBody = pOwner->get_RigidBodyComponent();

	return DURGA_V_STATE::DURGA_V_STATE_IDLE;
}
