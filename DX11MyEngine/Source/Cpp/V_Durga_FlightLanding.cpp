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
//* @:V_Durga_FlightLandingState Class 
//*【?】開始
//* 引数：1.Vehicle
//* 返値：void
//*----------------------------------------------------------------------------------------
void V_Durga_FlightLandingState::OnEnter(class VehicleController* pOwner)
{
	// 移動ベクトルは0
	pOwner->set_MoveSpeed(0.0f);

	// アニメーションON
	pOwner->set_IsPlayAnim(true);

	// 飛行-着地アニメーションに変更
	pOwner->ChangeAnimation(INT_CAST(DURGA_ANIM_ID::FLIGHT_LANDING), 0.3f, false);
	pOwner->get_AnimatorComponent()->set_IsPlaybackPaused(false);
	
	m_ElapsedTime = 0.0f;
}

//*---------------------------------------------------------------------------------------
//* @:V_Durga_FlightLandingState Class 
//*【?】終了
//* 引数：1.Vehicle
//* 返値：void
//*----------------------------------------------------------------------------------------
void V_Durga_FlightLandingState::OnExit(class VehicleController* pOwner)
{

}

//*---------------------------------------------------------------------------------------
//* @:V_Durga_FlightLandingState Class 
//*【?】更新
//* 引数：1.Vehicle
//* 返値：void
//*----------------------------------------------------------------------------------------
int V_Durga_FlightLandingState::Update(class VehicleController* pOwner)
{
	const auto& input = pOwner->get_InputResult_Robot();
	auto animator = pOwner->get_AnimatorComponent();
	auto transform = pOwner->get_TransformComponent();
	VEC3 crntPos = transform->get_VEC3ToPos();			// 現在の座標
	auto rigidBody = pOwner->get_RigidBodyComponent();

	//=========================================================================================
	//
	//			アニメーションが終了していれば、待機ステートへ
	//
	//=========================================================================================
	if (animator->get_IsCrntAnimationHasFinished())
	{
		return DURGA_V_STATE::DURGA_V_STATE_IDLE;
	}

	return DURGA_V_STATE::DURGA_V_STATE_FLIGHT_LANDING;
}
