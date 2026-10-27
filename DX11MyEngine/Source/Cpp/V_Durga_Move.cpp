#include "pch.h"
#include "Component_Vehicle.h"
#include "V_Durga_StateHeader.h"
#include "Component_MoveLogic.h"
#include "Component_RigidBody.h"
#include "Component_SkinnedMeshAnimator.h"
#include "GameObject.h"

using namespace VECTOR3;
using namespace VECTOR2;
using namespace UtilityData;
using namespace VehicleData;

namespace
{
	constexpr float MOVE_ACCELERATION = 10.0f;	// 移動加速度
	constexpr float MOVE_DECELERATION = 30.0f;	// 移動減速
}

//*---------------------------------------------------------------------------------------
//* @:V_Durga_MoveState Class 
//*【?】開始
//* 引数：1.Vehicle
//* 返値：void
//*----------------------------------------------------------------------------------------
void V_Durga_MoveState::OnEnter(class Vehicle* pOwner)
{
	// 移動ベクトルは0
	pOwner->set_MoveSpeed(20.0f);
	pOwner->set_IsPlayAnim(false);
	//pOwner->get_RigidBodyComponent()->ResetPhysics();

	// 速度制御をなくす
	auto move = pOwner->get_MoveLogicComponent();
	move->set_LinearVelocityControl(false);
	move->ClearSpeedEasing();
	pOwner->set_MoveLogicState(MOVE_BEHAVIOUR_TYPE::NONE);
}

//*---------------------------------------------------------------------------------------
//* @:V_Durga_MoveState Class 
//*【?】終了
//* 引数：1.Vehicle
//* 返値：void
//*----------------------------------------------------------------------------------------
void V_Durga_MoveState::OnExit(class Vehicle* pOwner)
{

}

//*---------------------------------------------------------------------------------------
//* @:V_Durga_MoveState Class 
//*【?】更新
//* 引数：1.Vehicle
//* 返値：void
//*----------------------------------------------------------------------------------------
int V_Durga_MoveState::Update(class Vehicle* pOwner)
{
	VEC3 inputDir = VEC3(0.0f, 0.0f, 0.0f);
	auto transform = pOwner->get_TransformComponent();
	auto rigidBody = pOwner->get_RigidBodyComponent();
	VEC3 crntPos = transform->get_VEC3ToPos();			// 現在の座標
	VEC3 crntRot = transform->get_VEC3ToRotateToRad();	// 現在の回転
	float moveSpeed = pOwner->get_MoveSpeed();
	float deltaTime = Master::m_pTimeManager->get_DeltaTime();

	// 前方向と右方向ベクトルを作る 
	VEC3 forward = transform->get_Forward();
	forward = forward.Normalize();
	VEC3 upVec = VEC3(0.0f, 1.0f, 0.0f);
	VEC3 right = VEC3::Cross(upVec, forward);

	// 正規化
	right = right.Normalize();

	if (GetInput(GAME_CONFIG::MOVE_FORWARD))	// 前進
	{
		inputDir += forward;
	}
	if (GetInput(GAME_CONFIG::MOVE_BACK))		// 後退
	{
		inputDir -= forward;
	}
	if (GetInput(GAME_CONFIG::MOVE_RIGHT))		// 右
	{
		inputDir += right;
	}
	if (GetInput(GAME_CONFIG::MOVE_LEFT))		// 左
	{
		inputDir -= right;
	}

	// 正規化
	inputDir = inputDir.Normalize();

	VEC3 velocity = rigidBody->GetLinearVelocity();
	VEC3 horizontal = VEC3(velocity.x, 0.0f, velocity.z);

	const bool hasInput = inputDir.LengthSq() > 0.0001f;	// 移動入力があったかどうか
	const VEC3 target = inputDir * moveSpeed;				// 目標とする水平速度ベクトル


	VEC3 difference = target - horizontal;
	const float distance = difference.Length();

	// 移動入力があるなら加速し、無いなら減速する
	const float maxChange =
		(hasInput ? MOVE_ACCELERATION : MOVE_DECELERATION) * deltaTime;

	// 移動入力なし
	if (distance <= maxChange)
	{
		horizontal = target;
	}
	// 移動入力あり
	else
	{
		horizontal += difference * (maxChange / distance);
	}

	// Y方向は維持し、重力やジャンプを消さない
	rigidBody->SetLinearVelocity(
		VEC3(horizontal.x, velocity.y, horizontal.z));


	if (GetInput(GAME_CONFIG::DURGA_ROT_RIGHT))	// 右回転
	{

	}
	if (GetInput(GAME_CONFIG::DURGA_ROT_LEFT))	// 左回転
	{

	}

	return DURGA_V_STATE::DURGA_V_STATE_MOVE;
}
