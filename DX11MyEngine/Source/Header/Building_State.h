#pragma once
#include "IState.h"
#include "ConstantMapData.h"

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:Building_IdleState Class --- */
//
//  ★継承：IComponent ★
//
// 【?】待機ステート
//		
// ***************************************************************************************
class Building_IdleState : public IState<class BuildingController>
{
private:
	int m_FrameCounter = 0;

public:
	void OnEnter(class BuildingController* pOwner) override;
	void OnExit(class BuildingController* pOwner)override;
	int Update(class BuildingController* pOwner)override;
};


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:Building_CllapseInState Class --- */
//
//  ★継承：IComponent ★
//
// 【?】倒壊始めステート
//		
// ***************************************************************************************
class Building_CllapseInState : public IState<class BuildingController>
{
private:
	float m_SunkTweenPosY = 0.0f;	// 倒壊するまでのY座標の変化量
	VECTOR3::VEC3 m_SunkTweenRot = 0.0f;	// 倒壊するまでの回転角度の変化量
	int m_FrameCounter = 0;	// フレームカウンター

public:
	void OnEnter(class BuildingController* pOwner) override;
	void OnExit(class BuildingController* pOwner)override;
	int Update(class BuildingController* pOwner)override;
};


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:Building_CllapseInState Class --- */
//
//  ★継承：IComponent ★
//
// 【?】倒壊中ステート
//		
// ***************************************************************************************
class Building_CllapseNowState : public IState<class BuildingController>
{
private:
	float m_CrntCollapseTime;	 // 現在の倒壊時間
	float m_CollapseTime;		 // 倒壊にかかる時間
	float m_CollapseTargetAngle; // 倒れる角度 
	float m_SunkRateY;			 // Y座標がどのくらい沈んだか
	int m_FrameCounter;
	VECTOR3::VEC3 m_StartRot;	// 開始時の回転

public:
	void OnEnter(class BuildingController* pOwner) override;
	void OnExit(class BuildingController* pOwner)override;
	int Update(class BuildingController* pOwner)override;
};


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:Building_IdleState Class --- */
//
//  ★継承：IComponent ★
//
// 【?】倒壊終了ステート
//		
// ***************************************************************************************
class Building_CllapseEndState : public IState<class BuildingController>
{
private:
	float m_CrntCollapseEndTime;	// 現在の倒壊終了時間

	VECTOR3::VEC3 m_Rot;

public:
	void OnEnter(class BuildingController* pOwner) override;
	void OnExit(class BuildingController* pOwner)override;
	int Update(class BuildingController* pOwner)override;
};


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:Building_FallState Class --- */
//
//  ★継承：IComponent ★
//
// 【?】倒壊が終わって、裏世界へ落ちていくステート
//		
// ***************************************************************************************
class Building_FallState : public IState<class BuildingController>
{
private:

public:
	void OnEnter(class BuildingController* pOwner) override;
	void OnExit(class BuildingController* pOwner)override;
	int Update(class BuildingController* pOwner)override;
};


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:Building_EndState Class --- */
//
//  ★継承：IComponent ★
//
// 【?】終了ステート
//		
// ***************************************************************************************
class Building_EndState : public IState<class BuildingController>
{
private:

public:
	void OnEnter(class BuildingController* pOwner) override;
	void OnExit(class BuildingController* pOwner)override;
	int Update(class BuildingController* pOwner)override;
};

