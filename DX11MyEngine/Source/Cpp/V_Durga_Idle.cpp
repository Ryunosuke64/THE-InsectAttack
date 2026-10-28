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

	//=========================================================================================
	//
	//						移動入力があるかチェック
	//
	//=========================================================================================
	if (input.CheckMove())
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
		//return DURGA_V_STATE::DURGA_V_STATE_LIFT;
	}
	if (input.isPunchA)
	{
	}
	auto rigidBody = pOwner->get_RigidBodyComponent();
	const float turnInput =
		(input.isRotR ? 1.0f : 0.0f) -
		(input.isRotL ? 1.0f : 0.0f);

	// 入力がないときは姿勢を書き換えない
	if (turnInput != 0.0f)
	{
		const float turnSpeed = DirectX::XMConvertToRadians(60.0f); // 毎秒60度
		const float dt = Master::m_pTimeManager->get_DeltaTime();
		const float deltaYaw = turnInput * turnSpeed * dt;

		const auto rotation = rigidBody->GetRotation();
		const auto currentQ = DirectX::XMVectorSet(
			rotation.x, rotation.y, rotation.z, rotation.w);

		const auto deltaQ =
			DirectX::XMQuaternionRotationRollPitchYaw(
				0.0f, deltaYaw, 0.0f);

		const auto nextQ = DirectX::XMQuaternionNormalize(
			DirectX::XMQuaternionMultiply(currentQ, deltaQ));

		DirectX::XMFLOAT4 result;
		DirectX::XMStoreFloat4(&result, nextQ);

		rigidBody->SetRotation(VEC4(
			result.x, result.y, result.z, result.w));
	}


	return DURGA_V_STATE::DURGA_V_STATE_IDLE;
}
