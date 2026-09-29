#include "pch.h"
#include "Component_EnemyController.h"
#include "Ant_StateHeader.h"
#include "Component_MoveLogic.h"
#include "ConstantWeaponData.h"
#include "GameObject.h"

using namespace VECTOR3;
using namespace VECTOR2;
using namespace UtilityData;
using namespace EnemyData;
using namespace BulletData;
using namespace DirectX;

//*---------------------------------------------------------------------------------------
//* @:Ant_AT_AttackAcidState Class 
//*【?】開始
//* 引数：1.EnemyController
//* 返値：void
//*----------------------------------------------------------------------------------------
void Ant_AT_AttackAcidState::OnEnter(class EnemyController* pOwner)
{
	// 移動ベクトルは0
	pOwner->set_MoveVelocity(VEC3());

	pOwner->set_IsAnim(true);

	pOwner->set_MoveLogicState(MOVE_BEHAVIOUR_TYPE::HOMING);

	// 攻撃前の硬直時間
	m_PreAttackStunDuration = Master::m_pRandomManager->GetFloatRandom(PREATTACK_STUN_TIME_MIN, PREATTACK_STUN_TIME_MAX);
}

//*---------------------------------------------------------------------------------------
//* @:Ant_AT_AttackAcidState Class 
//*【?】終了
//* 引数：1.EnemyController
//* 返値：void
//*----------------------------------------------------------------------------------------
void Ant_AT_AttackAcidState::OnExit(class EnemyController* pOwner)
{
	pOwner->clear_StateTimer();
}

//*---------------------------------------------------------------------------------------
//* @:Ant_AT_AttackAcidState Class 
//*【?】更新
//* 引数：1.EnemyController
//* 返値：void
//*----------------------------------------------------------------------------------------
int Ant_AT_AttackAcidState::Update(class EnemyController* pOwner)
{
	auto target = pOwner->get_Target();	// 目標オブジェクト
	if (target == nullptr || pOwner->get_OwnerObj().expired())
	{
		MessageBoxA(NULL, "ターゲットがいません", "Ant_PT_MoveState", MB_OK);
		assert(false);
	}
	else
	{
		int commonRes = Ant_CommonStateProcess::CommonProcess(pOwner);
		if (commonRes != -1)
		{
			return commonRes;
		}

		auto targetTransform = target->get_Transform().lock();
		VEC3 targetPos = targetTransform->get_VEC3ToPos();	// 目標位置
		auto myTransform = pOwner->get_TransformComponent();
		VEC3 myPos = myTransform->get_VEC3ToPos();			// 自分の位置

		VEC3 targetDir = (targetPos - myPos).Normalize();	// 目標方向

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
			//				 酸発射音再生
			// ****************************************************
			Master::m_pSoundManager->Play_3D(SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::ENEMY_ANT_ACID_SHOOT), myPos, SOUND_RADIUS);

			auto diffLevel = Master::m_pDataManager->get_DifficultyLevel();
			int id = 0;
			switch (diffLevel){
			case UtilityData::DIFFICULTY_LEVEL::EASY:id = 0; break;
			case UtilityData::DIFFICULTY_LEVEL::NORMAL:id = 1; break;
			case UtilityData::DIFFICULTY_LEVEL::HARD:id = 2; break;
			case UtilityData::DIFFICULTY_LEVEL::DISASTER:id = 3; break;
			case UtilityData::DIFFICULTY_LEVEL::IMPOSSIBLE:id = 4; break;
			default:break;
			}
			auto data = static_cast<const WeaponData::GunWeaponData*>(Master::m_pWeaponDataManager->FindEnemysWeaponData(id));
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
				bulletTrans._pos.y += 3.0f;		// そのままだと、地面に当たってしまうので少し上の方から発射する

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
			return ANT_STATE::ANT_STATE_ACTIVE_MOVE;
		}
	}

	return ANT_STATE::ANT_STATE_ACTIVE_ATTACK_ACID;
}
