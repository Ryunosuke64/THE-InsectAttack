#include "pch.h"
#include "Component_EnemyController.h"
#include "Octahedron_StateHeader.h"
#include "Component_MoveLogic.h"
#include "Component_AI_GunWeapon.h"
#include "GameObject.h"
#include "ConstantWeaponData.h"

using namespace VECTOR3;
using namespace VECTOR2;
using namespace UtilityData;
using namespace EnemyData;
using namespace BulletData;
using namespace DirectX;

//*---------------------------------------------------------------------------------------
//* @:Octahedron_AT_AttackLaser01State Class 
//*【?】開始
//* 引数：1.EnemyController
//* 返値：void
//*----------------------------------------------------------------------------------------
void Octahedron_AT_AttackLaser01State::OnEnter(class EnemyController* pOwner)
{
	pOwner->set_MoveSpeed(MOVE_SPEED * Master::m_pDataManager->get_EnemyDifficultyFactor()._moveSpeedRate);
}

//*---------------------------------------------------------------------------------------
//* @:Octahedron_AT_AttackLaser01State Class 
//*【?】終了
//* 引数：1.EnemyController
//* 返値：void
//*----------------------------------------------------------------------------------------
void Octahedron_AT_AttackLaser01State::OnExit(class EnemyController* pOwner)
{
	m_AttckCount = 0;
	m_AttackTime = 0.0f;
	pOwner->clear_StateTimer();
}

//*---------------------------------------------------------------------------------------
//* @:Octahedron_AT_AttackLaser01State Class 
//*【?】更新
//* 引数：1.EnemyController
//* 返値：void
//*----------------------------------------------------------------------------------------
int Octahedron_AT_AttackLaser01State::Update(class EnemyController* pOwner)
{
	auto target = pOwner->get_Target();	// 目標オブジェクト
	if (target == nullptr || pOwner->get_OwnerObj().expired())
	{
		MessageBoxA(NULL, "ターゲットがいません", "Ant_PT_MoveState", MB_OK);
		assert(false);
	}
	else
	{
		// 共通処理
		int commonRes = Octahedron_CommonStateProcess::CommonProcess(pOwner);
		if (commonRes != -1)
		{
			return commonRes;
		}


		auto targetTransform = target->get_Transform().lock();
		VEC3 targetPos = targetTransform->get_VEC3ToPos();	// 目標位置
		auto myTransform = pOwner->get_TransformComponent();
		VEC3 myPos = myTransform->get_VEC3ToPos();			// 自分の位置

		VEC3 targetDir = (targetPos - myPos).Normalize();	// 目標方向

		// 下方向
		targetDir = VEC3(0.0f, -1.0f, 0.0f);

		// 移動はさせずに、目標への方向だけ向かせる
		MoveParam movePram;
		movePram._moveSpeed = pOwner->get_MoveSpeed();
		movePram._maxSpeed = pOwner->get_MoveSpeed();
		movePram._turnSpeed = 0.5f;
		movePram._targetPos = targetPos;
		movePram._isPhysicsMove = true;
		auto move = pOwner->get_MoveLogicComponent();
		move->set_MoveParam(movePram);	// 移動ロジックにパラメータを渡す

		/* 硬直時間を終えたら、攻撃 */
		if (pOwner->get_StateTimer() > m_PreAttackStunDuration)
		{
			m_AttackTime += Master::m_pTimeManager->get_DeltaTime();

			auto gunWeapon = pOwner->get_AIGunWeapon();
			gunWeapon->set_AimPosition(targetPos);
			gunWeapon->Fire(*m_pRenderer);

			//=========================================================================================
			//
			//						攻撃が終了したら、移動ステートへ
			//
			//=========================================================================================
			if (gunWeapon->IsBurstFireEnd())
			{
				return OCTAHEDRON_STATE::OCTAHEDRON_STATE_ACTIVE_MOVE;
			}
		}
	}

	return OCTAHEDRON_STATE::OCTAHEDRON_STATE_ACTIVE_ATTACK_LASER01;
};
