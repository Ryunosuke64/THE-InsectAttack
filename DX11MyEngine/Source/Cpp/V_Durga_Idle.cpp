#include "pch.h"
#include "Component_Vehicle.h"
#include "V_Durga_StateHeader.h"
#include "GameObject.h"

using namespace VECTOR3;
using namespace VECTOR2;
using namespace UtilityData;
using namespace EnemyData;

//*---------------------------------------------------------------------------------------
//* @:V_Durga_IdleState Class 
//*【?】開始
//* 引数：1.Vehicle
//* 返値：void
//*----------------------------------------------------------------------------------------
void V_Durga_IdleState::OnEnter(class Vehicle* pOwner)
{
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
	return ANT_STATE::ANT_STATE_PATROL_IDLE;
}
