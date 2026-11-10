#include "pch.h"
#include "Component_VehicleController.h"
#include "V_Durga_StateHeader.h"
#include "Component_SkinnedMeshAnimator.h"
#include "Component_RigidBody.h"
#include "GameObject.h"

using namespace VECTOR3;
using namespace VECTOR4;
using namespace UtilityData;
using namespace VehicleData;

//*---------------------------------------------------------------------------------------
//* @:V_Durga_FlightState Class 
//*【?】開始
//* 引数：1.Vehicle
//* 返値：void
//*----------------------------------------------------------------------------------------
void V_Durga_FlightState::OnEnter(class VehicleController* pOwner)
{
	// 移動ベクトルは0
	pOwner->set_MoveSpeed(0.0f);
	pOwner->set_IsPlayAnim(false);
}

//*---------------------------------------------------------------------------------------
//* @:V_Durga_FlightState Class 
//*【?】終了
//* 引数：1.Vehicle
//* 返値：void
//*----------------------------------------------------------------------------------------
void V_Durga_FlightState::OnExit(class VehicleController* pOwner)
{
}

//*---------------------------------------------------------------------------------------
//* @:V_Durga_FlightState Class 
//*【?】更新
//* 引数：1.Vehicle
//* 返値：void
//*----------------------------------------------------------------------------------------
int V_Durga_FlightState::Update(class VehicleController* pOwner)
{
	const auto& input = pOwner->get_InputResult_Robot();
	auto animator = pOwner->get_AnimatorComponent();
	auto transform = pOwner->get_TransformComponent();
	VEC3 crntPos = transform->get_VEC3ToPos();			// 現在の座標
	const float AnimDuration = animator->get_AnimDurationSeconds(INT_CAST(DURGA_ANIM_ID::WALK_F_ENTER));	// 歩行アニメーションの再生時間を取得
	auto rigidBody = pOwner->get_RigidBodyComponent();

	return DURGA_V_STATE::DURGA_V_STATE_FLIGHT_CHARGE;
}
