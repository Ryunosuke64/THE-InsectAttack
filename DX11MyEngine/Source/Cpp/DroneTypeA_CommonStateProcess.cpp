#include "pch.h"
#include "DroneTypeA_StateHeader.h"
#include "Component_EnemyController.h"
#include "ConstantEnemyData.h"

using namespace VECTOR3;
using namespace EnemyData;

//*---------------------------------------------------------------------------------------
//*【?】共通処理
//*
//* [引数]
//* *pOwner : 親
//*
//* [返値]
//* ステートID（-1の場合は変更なし） 
//*----------------------------------------------------------------------------------------
int DroneTypeA_CommonStateProcess::CommonProcess(class EnemyController* pOwner)
{
	// 死亡ステートへ
	if (pOwner->get_IsDead())
	{
		return DRONE_TYPE_A_STATE::DRONE_TYPE_A_STATE_ACTIVE_DEAD;
	}
	else if (pOwner->get_IsOnDamage())
	{
		// 怯み状態ならヒットスタンステートへ
		if (pOwner->get_IsStagger()) {
			return DRONE_TYPE_A_STATE::DRONE_TYPE_A_STATE_ACTIVE_HIT_STUN;
		}
	}

	// 変更なし
	return -1;
}