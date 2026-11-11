#include "pch.h"
#include "Component_VehicleController.h"
#include "V_Durga_StateHeader.h"
#include "Component_SkinnedMeshAnimator.h"
#include "Component_RigidBody.h"
#include "GameObject.h"
#include "Component_Collider.h"
#include "Component_Health.h"
#include "Component_3DCamera.h"
using namespace DirectX;
using namespace VECTOR3;
using namespace VECTOR4;
using namespace PhysicsData;
using namespace UtilityData;
using namespace VehicleData;


namespace
{
	constexpr float TIME_TO_APEX = 1.2f;       // 頂点までの時間
	constexpr float HORIZONTAL_SPEED = 160.0f; // 水平方向の速度
	constexpr float LIGHT_SPAWN_DURATION = 0.1f;
}

//*---------------------------------------------------------------------------------------
//* @:V_Durga_FlightState Class 
//*【?】開始
//* 引数：1.Vehicle
//* 返値：void
//*----------------------------------------------------------------------------------------
void V_Durga_FlightState::OnEnter(class VehicleController* pOwner)
{
	// アニメーションON
	pOwner->set_IsPlayAnim(true);

	// 飛行中アニメーションに変更
	pOwner->ChangeAnimation(INT_CAST(DURGA_ANIM_ID::FLIGHT), 0.3f, false);
	pOwner->get_AnimatorComponent()->set_IsPlaybackPaused(false);


	// エフェクト再生
	m_JetEffectHandleR = Master::m_pEffectManager->PlayEffect("JetSpray_Flight");
	m_JetEffectHandleL = Master::m_pEffectManager->PlayEffect("JetSpray_Flight");


	auto transform = pOwner->get_TransformComponent();
	auto rigidBody = pOwner->get_RigidBodyComponent();
	VEC3 crntPos = transform->get_VEC3ToPos();

	// 離陸した瞬間の「水平な前方向」を取得する
	VEC3 forward = transform->get_Forward();
	forward.y = 0.0f;
	forward = forward.Normalize();

	// 現在設定されている重力から、上向き速度を計算する
	const float gravityStrength = -rigidBody->GetGravity().y;
	const float initialUpSpeed = gravityStrength * TIME_TO_APEX;

	// 前方向の速度と上向き速度を合成する
	VEC3 launchVelocity = forward * HORIZONTAL_SPEED;
	launchVelocity.y = initialUpSpeed * 7.5f;

	// 一度だけ設定する。以降は重力に任せる
	rigidBody->SetLinearVelocity(launchVelocity);

	// ****************************************************
	//				 離陸サウンド再生
	// ****************************************************
	Master::m_pSoundManager->Play_3D("vehicle.Takeoff", crntPos, 800.0f);
}

//*---------------------------------------------------------------------------------------
//* @:V_Durga_FlightState Class 
//*【?】終了
//* 引数：1.Vehicle
//* 返値：void
//*----------------------------------------------------------------------------------------
void V_Durga_FlightState::OnExit(class VehicleController* pOwner)
{
	// エフェクト停止
	Master::m_pEffectManager->StopEffect(m_JetEffectHandleR);
	Master::m_pEffectManager->StopEffect(m_JetEffectHandleL);
}

//*---------------------------------------------------------------------------------------
//* @:V_Durga_FlightState Class 
//*【?】更新
//* 引数：1.Vehicle
//* 返値：void
//*----------------------------------------------------------------------------------------
int V_Durga_FlightState::Update(class VehicleController* pOwner)
{
	const auto& input = pOwner->get_InputResult_Robot();
	auto animator = pOwner->get_AnimatorComponent();
	auto transform = pOwner->get_TransformComponent();
	VEC3 crntPos = transform->get_VEC3ToPos();			// 現在の座標
	auto rigidBody = pOwner->get_RigidBodyComponent();
	float moveSpeed = pOwner->get_MoveSpeed();
	float deltaTime = Master::m_pTimeManager->get_DeltaTime();


	// 掴む対象がある場合、手の位置に合わせる
	m_JetR_LocalMat = animator->get_BoneLocalWorldMatrix("Jet_R");
	m_JetL_LocalMat = animator->get_BoneLocalWorldMatrix("Jet_L");

	// get_BoneLocalWorldMatrix() はモデル内の行列なので、
	// 機体のワールド行列を掛けてワールド座標にする
	auto jetWorld_R = m_JetR_LocalMat * transform->get_WorldMtx();
	auto jetWorld_L = m_JetL_LocalMat * transform->get_WorldMtx();
	VEC3 jetR_Pos = VEC3::FromXMVECTOR(jetWorld_R.r[3]);
	VEC3 jetL_Pos = VEC3::FromXMVECTOR(jetWorld_L.r[3]);
	// 位置・回転・スケールに分解
	XMVECTOR scale;
	XMVECTOR rotation;
	XMVECTOR position;
	const float jetEffectSize = 1.0f;

	if (XMMatrixDecompose(
		&scale, &rotation, &position, jetWorld_R))
	{
		// クオータニオンを回転軸と角度に変換
		XMVECTOR axis;
		float angle;

		XMQuaternionToAxisAngle(
			&axis,
			&angle,
			rotation
		);

		XMFLOAT3 rotAxis;

		XMStoreFloat3(&rotAxis, axis);

		Master::m_pEffectManager->SetRotationEffect(m_JetEffectHandleR, VEC3(rotAxis.x, rotAxis.y, rotAxis.z), angle);
		Master::m_pEffectManager->SetPositionEffect(m_JetEffectHandleR, jetR_Pos);
		Master::m_pEffectManager->SetScaleEffect(m_JetEffectHandleR, jetEffectSize);
	}
	if (XMMatrixDecompose(
		&scale, &rotation, &position, jetWorld_L))
	{
		// クオータニオンを回転軸と角度に変換
		XMVECTOR axis;
		float angle;

		XMQuaternionToAxisAngle(
			&axis,
			&angle,
			rotation
		);

		XMFLOAT3 rotAxis;

		XMStoreFloat3(&rotAxis, axis);

		Master::m_pEffectManager->SetRotationEffect(m_JetEffectHandleL, VEC3(rotAxis.x, rotAxis.y, rotAxis.z), angle);
		Master::m_pEffectManager->SetPositionEffect(m_JetEffectHandleL, jetL_Pos);
		Master::m_pEffectManager->SetScaleEffect(m_JetEffectHandleL, jetEffectSize);
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
	VEC3 collisionPos = crntPos;	// 当たり判定の位置
	collisionPos.y += 30.0f;		// 頭の方に判定が来るように
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

				// 少し上（Y軸）に向かせることで、放物線を描いて飛ばせる
				//knockbackDir.y += 10.5f;
				//knockbackDir.x += Master::m_pRandomManager->GetFloatRandom(-0.5f, 0.5f);
				//knockbackDir.z += Master::m_pRandomManager->GetFloatRandom(-0.5f, 0.5f);
				//knockbackDir = knockbackDir.Normalize();

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


	//*****************************************************************************************
	//						パッと光らせる
	//*****************************************************************************************
	if (LIGHT_SPAWN_DURATION <= m_LightSpawnTimer)
	{
		float lightIntensity = Master::m_pRandomManager->GetFloatRandom(6.0f, 10.0f);
		TransientPointLightDesc desc;
		desc._color = VEC3(0.8f, 0.4f, 0.1f);
		desc._startRange = 1000.0f;
		desc._endRange = 0.0f;
		desc._startIntensity = lightIntensity;
		desc._endIntensity = 0.0f;
		desc._duration = 3.0f;
		auto handle = Master::m_pLightManager->PlayTransientPointLight(
			jetR_Pos,
			desc);

		m_LightSpawnTimer = 0.0f;
	}

	// ****************************************************
	//				 カメラシェイク
	// ****************************************************
	std::shared_ptr<Camera3D> camera;
	if (camera = Master::m_pDataManager->get_CameraComponent().lock())
	{
		camera->DistanceDecay(0.1f, VEC3(0.02f, 0.02f, 0.02f), crntPos, 300.0f);
	}

	//=========================================================================================
	//
	//					上向き速度がなくなったら、頂点に到達した
	//
	//=========================================================================================
	//
	if (rigidBody->GetLinearVelocity().y <= 0.0f)
	{
		return DURGA_V_STATE::DURGA_V_STATE_FLIGHT_FALL;
	}

	// 経過時間
	m_ElapsedTime += deltaTime;
	m_LightSpawnTimer += deltaTime;
	return DURGA_V_STATE::DURGA_V_STATE_FLIGHT;
}
