#include "pch.h"
#include "Component_VehicleController.h"
#include "V_Durga_StateHeader.h"
#include "Component_MoveLogic.h"
#include "Component_SkinnedMeshAnimator.h"
#include "Component_RigidBody.h"
#include "GameObject.h"

using namespace VECTOR4;
using namespace VECTOR3;
using namespace VECTOR2;
using namespace UtilityData;
using namespace VehicleData;

//*---------------------------------------------------------------------------------------
//* @:V_Durga_LiftIdleState Class 
//*【?】開始
//* 引数：1.Vehicle
//* 返値：void
//*----------------------------------------------------------------------------------------
void V_Durga_LiftIdleState::OnEnter(class VehicleController* pOwner)
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
//* @:V_Durga_LiftIdleState Class 
//*【?】終了
//* 引数：1.Vehicle
//* 返値：void
//*----------------------------------------------------------------------------------------
void V_Durga_LiftIdleState::OnExit(class VehicleController* pOwner)
{
}

//*---------------------------------------------------------------------------------------
//* @:V_Durga_LiftIdleState Class 
//*【?】更新
//* 引数：1.Vehicle
//* 返値：void
//*----------------------------------------------------------------------------------------
int V_Durga_LiftIdleState::Update(class VehicleController* pOwner)
{
	const auto& input = pOwner->get_InputResult_Robot();

	//=========================================================================================
	//
	//						移動入力があるかチェック
	//
	//=========================================================================================
	if (input.CheckMove() || input.CheckRot())
	{
		return DURGA_V_STATE::DURGA_V_STATE_LIFT_MOVE;
	}
	//=========================================================================================
	//
	//						投げつけ入力があるかチェック
	//
	//=========================================================================================
	if (input.isThrow)
	{
		return DURGA_V_STATE::DURGA_V_STATE_THROW;
	}

	return DURGA_V_STATE::DURGA_V_STATE_LIFT_IDLE;
}
