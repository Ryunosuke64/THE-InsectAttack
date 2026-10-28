#pragma once
#include "IState.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//						アリのステートのヘッダクラス定義をまとめたヘッダ
// 
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// PT:巡回中
// AT:戦闘態勢

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:V_Durga_IdleState Class --- */
//
//  ★継承：IState ★
//
// 【?】待機時ステート
//		
// ***************************************************************************************
class V_Durga_IdleState : public IState<class VehicleController>
{
private:


public:
	void OnEnter(class VehicleController* pOwner) override;
	void OnExit(class VehicleController* pOwner)override;
	int Update(class VehicleController* pOwner)override;
};

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:V_Durga_MoveState Class --- */
//
//  ★継承：IState ★
//
// 【?】移動ステート
//		
// ***************************************************************************************
class V_Durga_MoveState : public IState<class VehicleController>
{
private:
	/// <summary> 踏み出しの状態 </summary>
	enum class FOOT_STATE
	{
		FOOT_R_ENTER,	// 右足から踏み出し始める
		FOOT_R,
		FOOT_L,
	};

	VECTOR3::VEC3 m_InputDir = VECTOR3::VEC3();				// 入力の方向
	FOOT_STATE m_NextFootState = FOOT_STATE::FOOT_R_ENTER;	// 現在の踏み出し状態

public:
	void OnEnter(class VehicleController* pOwner) override;
	void OnExit(class VehicleController* pOwner)override;
	int Update(class VehicleController* pOwner)override;

private:
	void SetWalkAnimation(class VehicleController* pOwner);
};
