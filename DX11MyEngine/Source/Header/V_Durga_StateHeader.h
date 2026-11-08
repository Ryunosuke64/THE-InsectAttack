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
public:
	/// <summary> 踏み出しの状態 </summary>
	enum class FOOT_STATE
	{
		FOOT_R_ENTER,	// 右足から踏み出し始める
		FOOT_R,
		FOOT_L,
	};

private:

	VECTOR3::VEC3 m_InputDir = VECTOR3::VEC3();				// 入力の方向
	FOOT_STATE m_NextFootState = FOOT_STATE::FOOT_R_ENTER;	// 現在の踏み出し状態
	FOOT_STATE m_CrntFootState = FOOT_STATE::FOOT_R_ENTER;	// 現在の踏み出し状態
	VehicleData::DURGA_ANIM_ID m_CrntAnimID = 
		VehicleData::DURGA_ANIM_ID::WALK_F_ENTER;	// 現在のアニメーションID
	bool m_IsDustEffectPlay = false;;

public:
	void OnEnter(class VehicleController* pOwner) override;
	void OnExit(class VehicleController* pOwner)override;
	int Update(class VehicleController* pOwner)override;

private:
	void SetWalkAnimation(
		class VehicleController* pOwner,
		FOOT_STATE& outCrntFootState,
		FOOT_STATE& outNextFootState,
		VehicleData::DURGA_ANIM_ID& outCrnrAnimID
	);
};

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:V_Durga_LiftState Class --- */
//
//  ★継承：IState ★
//
// 【?】持ち上げステート
//		
// ***************************************************************************************
class V_Durga_LiftState : public IState<class VehicleController>
{
private:
	std::vector<std::weak_ptr<GameObject>> m_pLiftTargets;	// 持ち上げる対象
	DirectX::XMMATRIX m_HandR_LocalMat;	// 右手のローカル座標
	DirectX::XMMATRIX m_HandL_LocalMat;	// 右手のローカル座標
	bool m_IsPlayGrabSound = false;
public:
	void OnEnter(class VehicleController* pOwner) override;
	void OnExit(class VehicleController* pOwner)override;
	int Update(class VehicleController* pOwner)override;
};

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:V_Durga_LiftIdleState Class --- */
//
//  ★継承：IState ★
//
// 【?】持ち上げ中待機ステート
//		
// ***************************************************************************************
class V_Durga_LiftIdleState : public IState<class VehicleController>
{
private:


public:
	void OnEnter(class VehicleController* pOwner) override;
	void OnExit(class VehicleController* pOwner)override;
	int Update(class VehicleController* pOwner)override;
};

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:V_Durga_LiftMoveState Class --- */
//
//  ★継承：IState ★
//
// 【?】持ち上げ中移動ステート
//		
// ***************************************************************************************
class V_Durga_LiftMoveState : public IState<class VehicleController>
{
private:
	using FootState = V_Durga_MoveState::FOOT_STATE;

	VECTOR3::VEC3 m_InputDir = VECTOR3::VEC3();				// 入力の方向
	FootState m_NextFootState = FootState::FOOT_R;	// 現在の踏み出し状態
	FootState m_CrntFootState = FootState::FOOT_R;	// 現在の踏み出し状態
	VehicleData::DURGA_ANIM_ID m_CrntAnimID =
		VehicleData::DURGA_ANIM_ID::WALK_F_ENTER;	// 現在のアニメーションID
	bool m_IsDustEffectPlay = false;;


	std::vector<std::weak_ptr<GameObject>> m_pLiftObjects;	// 持ち上げる対象
	DirectX::XMMATRIX m_HandR_LocalMat;	// 右手のローカル座標
	DirectX::XMMATRIX m_HandL_LocalMat;	// 右手のローカル座標
	bool m_IsPlayGrabSound = false;
public:
	void OnEnter(class VehicleController* pOwner) override;
	void OnExit(class VehicleController* pOwner)override;
	int Update(class VehicleController* pOwner)override;

private:
	void SetWalkAnimation(
		class VehicleController* pOwner,
		FootState& outCrntFootState,
		FootState& outNextFootState,
		VehicleData::DURGA_ANIM_ID& outCrnrAnimID
	);
};

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:V_Durga_ThrowState Class --- */
//
//  ★継承：IState ★
//
// 【?】投げ飛ばすステート
//		
// ***************************************************************************************
class V_Durga_ThrowState : public IState<class VehicleController>
{
private:
	std::vector<std::weak_ptr<GameObject>> m_pLiftObjects;	// 持ち上げているオブジェクト
	DirectX::XMMATRIX m_HandR_LocalMat;			// 右手のローカル座標
	DirectX::XMMATRIX m_HandL_LocalMat;			// 右手のローカル座標
	VECTOR3::VEC3 m_GrabOffset;					// 掴む対象のオフセット座標
	bool m_IsThrow = false;	// 投げ終わったか
public:
	void OnEnter(class VehicleController* pOwner) override;
	void OnExit(class VehicleController* pOwner)override;
	int Update(class VehicleController* pOwner)override;
};