#include "pch.h"
#include "Component_EnemyController.h"
#include "Octahedron_StateHeader.h"
#include "Component_MoveLogic.h"
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

}

//*---------------------------------------------------------------------------------------
//* @:Octahedron_AT_AttackLaser01State Class 
//*【?】終了
//* 引数：1.EnemyController
//* 返値：void
//*----------------------------------------------------------------------------------------
void Octahedron_AT_AttackLaser01State::OnExit(class EnemyController* pOwner)
{
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
		movePram._moveSpeed = 1.0f;
		movePram._turnSpeed = 0.5f;
		movePram._targetPos = targetPos;
		movePram._isPhysicsMove = true;
		auto move = pOwner->get_MoveLogicComponent();
		move->set_MoveParam(movePram);	// 移動ロジックにパラメータを渡す

		/* 硬直時間を終えたら、攻撃 */
		if (pOwner->get_StateTimer() > m_PreAttackStunDuration)
		{
			// ****************************************************
			//				 発射音再生
			// ****************************************************
			Master::m_pSoundManager->Play_3D(SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::ENEMY_ANT_ACID_SHOOT), myPos, SOUND_RADIUS);

			auto diffLevel = Master::m_pDataManager->get_DifficultyLevel();
			int id = 0;
			switch (diffLevel) {
			case UtilityData::DIFFICULTY_LEVEL::EASY:id = 0; break;
			case UtilityData::DIFFICULTY_LEVEL::NORMAL:id = 1; break;
			case UtilityData::DIFFICULTY_LEVEL::HARD:id = 2; break;
			case UtilityData::DIFFICULTY_LEVEL::DISASTER:id = 3; break;
			case UtilityData::DIFFICULTY_LEVEL::IMPOSSIBLE:id = 4; break;
			default:break;
			}
			auto data = static_cast<const WeaponData::GunWeaponData*>(Master::m_pWeaponDataManager->FindEnemysWeaponData(5));
			data->_bulletData;

			XMVECTOR rotQuat = myTransform->get_RotationQuaternion();

			// 前方を向く回転行列（LookTo）
			XMMATRIX lookAtRotationMatrix = XMMatrixLookToLH(XMVectorZero(), targetDir, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));

			// 行列からクォータニオンへ変換
			XMVECTOR targetQuat = XMQuaternionRotationMatrix(XMMatrixTranspose(lookAtRotationMatrix));

			// 攻撃処理
			for (int i = 0; i < data->_bulletSimultaneousNum; i++)
			{
				// 弾のバラつき
				float accuracy = data->_accuracy;
				VEC3 accuracyRot;
				accuracyRot.x += Master::m_pRandomManager->GetFloatRandom(-accuracy, accuracy);
				accuracyRot.y += Master::m_pRandomManager->GetFloatRandom(-accuracy, accuracy);
				accuracyRot.z += Master::m_pRandomManager->GetFloatRandom(-accuracy, accuracy);


				// バラつきクォータニオン
				XMVECTOR spreadQuat = XMQuaternionRotationRollPitchYaw(accuracyRot.x, accuracyRot.y, accuracyRot.z);

				// 最終的なクォータニオン作成
				XMVECTOR finalRotQuat = XMQuaternionMultiply(targetQuat, spreadQuat);
				finalRotQuat = XMQuaternionNormalize(finalRotQuat); // 念のため正規化

				// 弾のトランスフォーム
				BulletTransformData bulletTrans;
				bulletTrans._pos = myPos;
				bulletTrans._scale = 1.0;
				bulletTrans._rotQ = finalRotQuat;

				BulletSpawnContext spawnContext;
				spawnContext._transform = bulletTrans;

				// 弾データを共用体で持っているので、弾タイプにあったパラメータを入れるようにする
				Master::m_pBulletManager->Shot(*m_pRenderer, spawnContext, data->_bulletData);

			}
			//=========================================================================================
			//
			//						攻撃が終了したら、移動ステートへ
			//
			//=========================================================================================
			return OCTAHEDRON_STATE::OCTAHEDRON_STATE_ACTIVE_MOVE;
		}
	}

	return OCTAHEDRON_STATE::OCTAHEDRON_STATE_ACTIVE_ATTACK_LASER01;
};
