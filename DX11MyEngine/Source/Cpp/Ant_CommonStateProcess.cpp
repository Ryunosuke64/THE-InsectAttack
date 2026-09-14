#include "pch.h"
#include "Ant_StateHeader.h"
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
int Ant_CommonStateProcess::CommonProcess(class EnemyController* pOwner)
{
	// 死亡ステートへ
	if (pOwner->get_IsDead())
	{
		return ANT_STATE::ANT_STATE_ACTIVE_DEAD;
	}
	else if (pOwner->get_IsOnDamage())
	{
		// 怯み状態ならヒットスタンステートへ
		if (pOwner->get_IsStagger()) {
			return ANT_STATE::ANT_STATE_ACTIVE_HIT_STUN;
		}
	}
	else
	{
		// ステート内で移動処理が実行されるので、移動後の値が取れる
		auto myTransform = pOwner->get_TransformComponent();
		VEC3 newPos = myTransform->get_VEC3ToPos();
		//newPos.y = 0.0f;
		myTransform->set_Pos(newPos);
	}


	// 変更なし
	return -1;
}