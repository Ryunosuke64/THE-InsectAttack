#include "pch.h"
#include "Component_VehicleController.h"
#include "V_Durga_StateHeader.h"
#include "Component_MoveLogic.h"
#include "Component_RigidBody.h"
#include "Component_SkinnedMeshAnimator.h"
#include "Component_3DCamera.h"
#include "GameObject.h"
#include "Component_Collider.h"
#include "Component_Health.h"

using namespace VECTOR4;
using namespace VECTOR3;
using namespace UtilityData;
using namespace VehicleData;
using namespace PhysicsData;

namespace
{
	constexpr float MOVE_ACCELERATION = 1000.0f;	// 移動加速度
	constexpr float MOVE_DECELERATION = 0.1f;	// 移動減速
	constexpr float MOVE_SPEED = 80.0f;	// 移動速度
}

//*---------------------------------------------------------------------------------------
//* @:V_Durga_MoveState Class 
//*【?】開始
//* 引数：1.Vehicle
//* 返値：void
//*----------------------------------------------------------------------------------------
void V_Durga_MoveState::OnEnter(class VehicleController* pOwner)
{
	const auto& input = pOwner->get_InputResult_Robot();
	auto transform = pOwner->get_TransformComponent();
	m_InputDir = VEC3(0.0f, 0.0f, 0.0f);
	// 前方向と右方向ベクトルを作る 
	VEC3 forward = transform->get_Forward();
	forward = forward.Normalize();
	VEC3 upVec = VEC3(0.0f, 1.0f, 0.0f);
	VEC3 right = VEC3::Cross(upVec, forward);
	// 正規化
	right = right.Normalize();
	if (input.isMoveF)		// 前進
	{
		m_InputDir += forward;
	}
	if (input.isMoveB)		// 後退
	{
		m_InputDir -= forward;
	}
	if (input.isMoveR)		// 右
	{
		m_InputDir += right;
	}
	if (input.isMoveL)		// 左
	{
		m_InputDir -= right;
	}

	// 正規化
	m_InputDir = m_InputDir.Normalize();

	// 移動ベクトルは0
	pOwner->set_MoveSpeed(MOVE_SPEED);
	pOwner->set_IsPlayAnim(true);
	pOwner->get_AnimatorComponent()->set_IsPlaybackPaused(false);
	//pOwner->get_RigidBodyComponent()->ResetPhysics();

	// 速度制御をなくす
	auto move = pOwner->get_MoveLogicComponent();
	move->set_LinearVelocityControl(false);
	move->ClearSpeedEasing();
	pOwner->set_MoveLogicState(MOVE_BEHAVIOUR_TYPE::NONE);

	// アニメーションの設定
	SetWalkAnimation(pOwner);
}

//*---------------------------------------------------------------------------------------
//* @:V_Durga_MoveState Class 
//*【?】終了
//* 引数：1.Vehicle
//* 返値：void
//*----------------------------------------------------------------------------------------
void V_Durga_MoveState::OnExit(class VehicleController* pOwner)
{
	pOwner->get_AnimatorComponent()->set_IsPlaybackPaused(true);
}

//*---------------------------------------------------------------------------------------
//* @:V_Durga_MoveState Class 
//*【?】更新
//* 引数：1.Vehicle
//* 返値：void
//*----------------------------------------------------------------------------------------
int V_Durga_MoveState::Update(class VehicleController* pOwner)
{
	const auto& input = pOwner->get_InputResult_Robot();

	auto animator = pOwner->get_AnimatorComponent();
	auto transform = pOwner->get_TransformComponent();
	auto rigidBody = pOwner->get_RigidBodyComponent();
	VEC3 crntPos = transform->get_VEC3ToPos();			// 現在の座標
	float moveSpeed = pOwner->get_MoveSpeed();
	float deltaTime = Master::m_pTimeManager->get_DeltaTime();
	float walkAnimDuration = animator->get_AnimDurationSeconds(INT_CAST(DURGA_ANIM_ID::WALK_F_ENTER));	// 歩行アニメーションの再生時間を取得


	const bool hasInput = m_InputDir.LengthSq() > 0.0001f;	// 移動入力があったかどうか
	VEC3 velocity = rigidBody->GetLinearVelocity();
	VEC3 horizontal = VEC3(velocity.x, 0.0f, velocity.z);

	const VEC3 target = m_InputDir * moveSpeed;				// 目標とする水平速度ベクトル

	VEC3 difference = target - horizontal;
	const float distance = difference.Length();

	// 移動入力があるなら加速し、無いなら減速する
	const float maxChange =
		(hasInput ? MOVE_ACCELERATION : MOVE_DECELERATION) * deltaTime;

	// 移動入力なし
	if (distance <= maxChange)
	{
		horizontal = target;
	}
	// 移動入力あり
	else
	{
		horizontal += difference * (maxChange / distance);
	}


	// Y方向は維持し、重力やジャンプを消さない
	rigidBody->SetLinearVelocity(
		VEC3(horizontal.x, velocity.y, horizontal.z));

	const float turnInput =
		(input.isRotR ? 1.0f : 0.0f) -
		(input.isRotL ? 1.0f : 0.0f);

	// 入力がないときは姿勢を書き換えない
	if (turnInput != 0.0f)
	{
		const float turnSpeed = DirectX::XMConvertToRadians(60.0f); // 毎秒60度
		const float deltaYaw = turnInput * turnSpeed * deltaTime;

		const auto rotation = rigidBody->GetRotation();
		const auto currentQ = DirectX::XMVectorSet(
			rotation.x, rotation.y, rotation.z, rotation.w);

		const auto deltaQ =
			DirectX::XMQuaternionRotationRollPitchYaw(
				0.0f, deltaYaw, 0.0f);

		const auto nextQ = DirectX::XMQuaternionNormalize(
			DirectX::XMQuaternionMultiply(currentQ, deltaQ));

		DirectX::XMFLOAT4 result;
		DirectX::XMStoreFloat4(&result, nextQ);

		rigidBody->SetRotation(VEC4(
			result.x, result.y, result.z, result.w));
	}

	m_InputDir = VEC3(0.0f);
	
	
	// 前方向と右方向ベクトルを作る 
	VEC3 forward = transform->get_Forward();
	forward = forward.Normalize();
	VEC3 upVec = VEC3(0.0f, 1.0f, 0.0f);
	VEC3 right = VEC3::Cross(upVec, forward);
	// 正規化
	right = right.Normalize();
	
	if (walkAnimDuration - 0.67f <= animator->get_AnimProcTime() &&!m_IsDustEffectPlay)
	{
		// ****************************************************
		//				 土埃エフェクト
		// ****************************************************
		auto transform = pOwner->get_TransformComponent();
		int dustEffectHandle = Master::m_pEffectManager->PlayEffect("Dust_01");
		VEC3 footPos = transform->get_VEC3ToPos();
		VEC3 footOffset;
		switch (m_CrntFootState)
		{
		case V_Durga_MoveState::FOOT_STATE::FOOT_R_ENTER:
			footOffset = right * 5.0;
			break;
		case V_Durga_MoveState::FOOT_STATE::FOOT_R:
			footOffset = right * 5.0;
			break;
		case V_Durga_MoveState::FOOT_STATE::FOOT_L:
			footOffset = right * -5.0;
			break;
		default:
			break;
		}
		footOffset += forward * 8.0f;

		footPos += footOffset;
		VEC3 scale = VEC3(1.0f, 1.0f, 1.0f);
		Master::m_pEffectManager->SetPositionEffect(dustEffectHandle, footPos);
		Master::m_pEffectManager->SetScaleEffect(dustEffectHandle, scale);
		// ****************************************************
		//				 カメラシェイク
		// ****************************************************
		std::shared_ptr<Camera3D> camera;
		if (camera = Master::m_pDataManager->get_CameraComponent().lock())
		{
			camera->DistanceDecay(1.0f, VEC3(0.1f, 0.5f, 0.1f), crntPos, 1000.0f);
		}
		// ****************************************************
		//				 踏みつけサウンド再生
		// ****************************************************
		Master::m_pSoundManager->Play_3D("vehicle.Stomp", crntPos, 1000.0f);

		// ****************************************************
		//				 範囲内にダメージ
		// ****************************************************
		unsigned mask = 
			UINT_CAST(COLLISION_CATEGORY::ENEMY) | 
			UINT_CAST(COLLISION_CATEGORY::DESTRUCTION_BUILDING) | 
			UINT_CAST(COLLISION_CATEGORY::PROP);
		const float damageRadius = 15.0f;
		const float knockback = 100.0f;
		const float damage = 700.0f;
		// 範囲内チェック
		auto targets = Master::m_pPhysicsEngine->CheckSphere(footPos, damageRadius, mask);

		CollisionInfo damageCollisionInfo ;

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
					VECTOR3::VEC3 knockbackDir = targetPos - footPos;   // 爆心地から外（衝突オブジェクト）へ向かうベクトル
					knockbackDir = knockbackDir.Normalize();

					// 少し上（Y軸）に向かせることで、放物線を描いて飛ばせる
					knockbackDir.y += 10.5f;
					knockbackDir.x += Master::m_pRandomManager->GetFloatRandom(-0.5f, 0.5f);
					knockbackDir.z += Master::m_pRandomManager->GetFloatRandom(-0.5f, 0.5f);
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

		m_IsDustEffectPlay = true;
	}

	//=========================================================================================
	//
	//				アニメーションが終了し、移動入力がない場合、待機ステートへ
	//
	//=========================================================================================
	if (walkAnimDuration - 0.28f <= animator->get_AnimProcTime())
	{
		m_IsDustEffectPlay = false;

		if (!input.CheckMove())
		{
			return DURGA_V_STATE::DURGA_V_STATE_IDLE;
		}
		else
		{
			// アニメーションの設定
			SetWalkAnimation(pOwner);


			m_InputDir = VEC3(0.0f, 0.0f, 0.0f);

			if (input.isMoveF)		// 前進
			{
				m_InputDir += forward;
			}
			if (input.isMoveB)		// 後退
			{
				m_InputDir -= forward;
			}
			if (input.isMoveR)		// 右
			{
				m_InputDir += right;
			}
			if (input.isMoveL)		// 左
			{
				m_InputDir -= right;
			}

			// 正規化
			m_InputDir = m_InputDir.Normalize();
		}
	}

	return DURGA_V_STATE::DURGA_V_STATE_MOVE;
}

void V_Durga_MoveState::SetWalkAnimation(class VehicleController* pOwner)
{
	m_CrntFootState = m_NextFootState;

	// 現在の足の踏み出し状況により、アニメーションを切り替える
	// 次に踏み出す足の状態も入れる
	switch (m_NextFootState)
	{
	case V_Durga_MoveState::FOOT_STATE::FOOT_R_ENTER:
		pOwner->ChangeAnimation(INT_CAST(DURGA_ANIM_ID::WALK_F_ENTER));
		m_NextFootState = FOOT_STATE::FOOT_L;
		break;
	case V_Durga_MoveState::FOOT_STATE::FOOT_R:
		pOwner->ChangeAnimation(INT_CAST(DURGA_ANIM_ID::WALK_F_R));
		m_NextFootState = FOOT_STATE::FOOT_L;
		break;
	case V_Durga_MoveState::FOOT_STATE::FOOT_L:
		pOwner->ChangeAnimation(INT_CAST(DURGA_ANIM_ID::WALK_F_L));
		m_NextFootState = FOOT_STATE::FOOT_R;
		break;
	default:
		break;
	}
	auto transform = pOwner->get_TransformComponent();
	VEC3 crntPos = transform->get_VEC3ToPos();

	// ****************************************************
	//				 駆動サウンド再生
	// ****************************************************
	Master::m_pSoundManager->Play_3D("vehicle.WalkingNoise", crntPos, 500.0f);
}
