#include "pch.h"
#include "Component_VehicleController.h"
#include "V_Durga_StateHeader.h"
#include "Component_SkinnedMeshAnimator.h"
#include "Component_RigidBody.h"
#include "GameObject.h"
#include "Component_Collider.h"
#include "Component_Health.h"
#include "Component_3DCamera.h"

using namespace VECTOR3;
using namespace VECTOR4;
using namespace PhysicsData;
using namespace UtilityData;
using namespace VehicleData;

namespace
{
	constexpr float TIME_TO_APEX = 2.5f;       // 頂点までの時間
	constexpr float HORIZONTAL_SPEED = 120.0f; // 水平方向の速度
	constexpr float FLIGHT_FALL_DURATION = 4.0f; // 降下終了時間
	constexpr float EFFECT_SPAWN_DURATION = 0.1f; // 降下終了時間
	constexpr float FRICTION = 0.01f;
}

//*---------------------------------------------------------------------------------------
//* @:V_Durga_FlightFallState Class 
//*【?】開始
//* 引数：1.Vehicle
//* 返値：void
//*----------------------------------------------------------------------------------------
void V_Durga_FlightFallState::OnEnter(class VehicleController* pOwner)
{
	// 移動ベクトルは0
	pOwner->set_MoveSpeed(0.0f);

	// アニメーションON
	pOwner->set_IsPlayAnim(true);

	// 飛行-降下アニメーションに変更
	pOwner->ChangeAnimation(INT_CAST(DURGA_ANIM_ID::FLIGHT_FALL), 0.3f, false);
	pOwner->get_AnimatorComponent()->set_IsPlaybackPaused(false);




	auto transform = pOwner->get_TransformComponent();
	auto rigidBody = pOwner->get_RigidBodyComponent();

	// 元の摩擦を記録
	m_OriginalFriction = rigidBody->GetFriction();
	// 摩擦を下げる
	rigidBody->SetFriction(FRICTION);

	// 離陸した瞬間の「水平な前方向」を取得する
	VEC3 forward = transform->get_Forward();
	forward.y = 0.0f;
	forward = forward.Normalize();

	// 現在設定されている重力から、上向き速度を計算する
	const float gravityStrength = rigidBody->GetGravity().y;
	const float initialUpSpeed = gravityStrength * TIME_TO_APEX;

	// 前方向の速度と上向き速度を合成する
	VEC3 launchVelocity = forward * HORIZONTAL_SPEED;
	launchVelocity.y = initialUpSpeed * 6.0f;

	// 一度だけ設定する。以降は重力に任せる
	rigidBody->SetLinearVelocity(launchVelocity);

	m_ElapsedTime = 0.0f;
}

//*---------------------------------------------------------------------------------------
//* @:V_Durga_FlightFallState Class 
//*【?】終了
//* 引数：1.Vehicle
//* 返値：void
//*----------------------------------------------------------------------------------------
void V_Durga_FlightFallState::OnExit(class VehicleController* pOwner)
{
	// 摩擦を元に戻す
	auto rigidBody = pOwner->get_RigidBodyComponent();
	rigidBody->SetFriction(m_OriginalFriction);
}
//*---------------------------------------------------------------------------------------
//* @:V_Durga_FlightFallState Class 
//*【?】更新
//* 引数：1.Vehicle
//* 返値：void
//*----------------------------------------------------------------------------------------
int V_Durga_FlightFallState::Update(class VehicleController* pOwner)
{
	const auto& input = pOwner->get_InputResult_Robot();
	auto animator = pOwner->get_AnimatorComponent();
	auto transform = pOwner->get_TransformComponent();
	VEC3 crntPos = transform->get_VEC3ToPos();			// 現在の座標
	auto rigidBody = pOwner->get_RigidBodyComponent();
	float deltaTime = Master::m_pTimeManager->get_DeltaTime();

	//*****************************************************************************************
	// 土埃エフェクトの再生
	//*****************************************************************************************
	if (EFFECT_SPAWN_DURATION <= m_FootDustEffectSpawnTime)
	{
		m_FootDustEffectSpawnTime = 0.0f;
		// 前方向と右方向ベクトルを作る 
		VEC3 forward = transform->get_Forward();
		forward = forward.Normalize();
		VEC3 upVec = VEC3(0.0f, 1.0f, 0.0f);
		VEC3 right = VEC3::Cross(upVec, forward);
		// 正規化
		right = right.Normalize();

		// ****************************************************
		//				 土埃エフェクト
		// ****************************************************
		auto transform = pOwner->get_TransformComponent();
		int footRDustEffectHandle = Master::m_pEffectManager->PlayEffect("Dust_03");
		int footLDustEffectHandle = Master::m_pEffectManager->PlayEffect("Dust_03");
		VEC3 footRPos = crntPos;
		VEC3 footLPos = crntPos;
		VEC3 footOffsetR;
		VEC3 footOffsetL;
		footOffsetR = right * 5.0;
		footOffsetL = right * -5.0;
		footOffsetR += forward * 8.0f;
		footOffsetL += forward * 8.0f;

		footRPos += footOffsetR;
		footLPos += footOffsetL;
		VEC3 scale = VEC3(1.2f, 1.2f, 1.2f);
		Master::m_pEffectManager->SetPositionEffect(footRDustEffectHandle, footRPos);
		Master::m_pEffectManager->SetPositionEffect(footLDustEffectHandle, footLPos);
		Master::m_pEffectManager->SetScaleEffect(footRDustEffectHandle, scale);
		Master::m_pEffectManager->SetScaleEffect(footLDustEffectHandle, scale);
		
		// ****************************************************
		//				 カメラシェイク
		// ****************************************************
		std::shared_ptr<Camera3D> camera;
		if (camera = Master::m_pDataManager->get_CameraComponent().lock())
		{
			camera->DistanceDecay(1.0f, VEC3(0.1f, 0.1f, 0.1f), crntPos, 400.0f);
		}

		// ****************************************************
		//				 着地サウンド再生
		// ****************************************************
		Master::m_pSoundManager->Play_3D("vehicle.Stomp", crntPos, 200.0f);
	}


	// ****************************************************
	//				 範囲内にダメージ
	// ****************************************************
	unsigned mask =
		UINT_CAST(COLLISION_CATEGORY::ENEMY) |
		UINT_CAST(COLLISION_CATEGORY::DESTRUCTION_BUILDING) |
		UINT_CAST(COLLISION_CATEGORY::PROP);
	const float damageRadius = 30.0f;
	const float knockback = 100.0f;
	const float damage = 600.0f;
	VEC3 collisionPos = crntPos;	// 当たり判定の位置（足元）
	// 範囲内チェック
	auto targets = Master::m_pPhysicsEngine->CheckSphere(crntPos, damageRadius, mask);

	CollisionInfo damageCollisionInfo;
	// 範囲内の全員にダメージ
	for (auto& target : targets)
	{
		if (auto obj = target.lock())
		{
			auto targetTransform = obj->get_Transform().lock();
			VEC3 targetPos = targetTransform->get_VEC3ToPos();

			// ダメージ
			if (auto health = obj->get_Component<Health>())
			{
				health->TakeDamage(damage, damageCollisionInfo);
			}

			// 吹っ飛び
			if (auto rb = obj->get_Component<RigidBody>())
			{
				VECTOR3::VEC3 knockbackDir = targetPos - collisionPos;   // 爆心地から外（衝突オブジェクト）へ向かうベクトル
				knockbackDir = knockbackDir.Normalize();

				// 衝撃ベクトルの設定
				rb->AddImpulse(knockbackDir * knockback);

				// 回転にも衝撃を与える
				knockbackDir.y = Master::m_pRandomManager->GetFloatRandom(-3, 3);
				knockbackDir.x = Master::m_pRandomManager->GetFloatRandom(-3, 3);
				knockbackDir.z = Master::m_pRandomManager->GetFloatRandom(-3, 3);
				rb->AddAngularImpulse(knockbackDir * knockback);
			}
		}
	}

	//=========================================================================================
	//
	//						降下時間を超えたら、着地ステートへ
	//
	//=========================================================================================
	if (FLIGHT_FALL_DURATION <= m_ElapsedTime)
	{
		return DURGA_V_STATE::DURGA_V_STATE_FLIGHT_LANDING;
	}

	// 経過時間
	m_ElapsedTime += deltaTime;
	m_FootDustEffectSpawnTime += deltaTime;

	return DURGA_V_STATE::DURGA_V_STATE_FLIGHT_FALL;
}
