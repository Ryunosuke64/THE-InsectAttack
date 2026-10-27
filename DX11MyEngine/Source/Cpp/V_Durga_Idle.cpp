#include "pch.h"
#include "Component_Vehicle.h"
#include "V_Durga_StateHeader.h"
#include "Component_MoveLogic.h"
#include "GameObject.h"

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
void V_Durga_IdleState::OnEnter(class Vehicle* pOwner)
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
void V_Durga_IdleState::OnExit(class Vehicle* pOwner)
{

}

//*---------------------------------------------------------------------------------------
//* @:V_Durga_IdleState Class 
//*【?】更新
//* 引数：1.Vehicle
//* 返値：void
//*----------------------------------------------------------------------------------------
int V_Durga_IdleState::Update(class Vehicle* pOwner)
{
	return DURGA_V_STATE::DURGA_V_STATE_IDLE;
}
