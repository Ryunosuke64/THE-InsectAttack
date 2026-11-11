#include "pch.h"
#include "Component_VehicleController.h"
#include "V_Durga_StateHeader.h"
#include "Component_SkinnedMeshAnimator.h"
#include "Component_RigidBody.h"
#include "Component_3DCamera.h"
#include "GameObject.h"

using namespace VECTOR3;
using namespace VECTOR4;
using namespace UtilityData;
using namespace VehicleData;
using namespace DirectX;

namespace
{
	constexpr float FLIGHT_CHARGE_DURATION = 5.0f; // 飛行開始までのチャージ時間
	constexpr float EFFECT_SPAWN_DURATION = 0.1f; // エフェクトのスポーン間隔
}

//*---------------------------------------------------------------------------------------
//* @:V_Durga_FlightChargeState Class 
//*【?】開始
//* 引数：1.Vehicle
//* 返値：void
//*----------------------------------------------------------------------------------------
void V_Durga_FlightChargeState::OnEnter(class VehicleController* pOwner)
{
	// 移動ベクトルは0
	pOwner->set_MoveSpeed(0.0f);

	// アニメーションON
	pOwner->set_IsPlayAnim(true);

	// 飛行準備アニメーションに変更
	pOwner->ChangeAnimation(INT_CAST(DURGA_ANIM_ID::FLIGHT_CHARGE), 0.3f, false);
	pOwner->get_AnimatorComponent()->set_IsPlaybackPaused(false);

	m_ElapsedTime = 0.0f;

	// エフェクト再生
	m_JetEffectHandleR = Master::m_pEffectManager->PlayEffect("JetSpray");
	m_JetEffectHandleL = Master::m_pEffectManager->PlayEffect("JetSpray");

	auto transform = pOwner->get_TransformComponent();
	VEC3 crntPos = transform->get_VEC3ToPos();			// 現在の座標

	// ****************************************************
	//				 ジェット噴射サウンド再生
	// ****************************************************
	Master::m_pSoundManager->Play_3D("vehicle.JetSpray", crntPos, 800.0f);

}

//*---------------------------------------------------------------------------------------
//* @:V_Durga_FlightChargeState Class 
//*【?】終了
//* 引数：1.Vehicle
//* 返値：void
//*----------------------------------------------------------------------------------------
void V_Durga_FlightChargeState::OnExit(class VehicleController* pOwner)
{
	// エフェクト停止
	Master::m_pEffectManager->StopEffect(m_JetEffectHandleR);
	Master::m_pEffectManager->StopEffect(m_JetEffectHandleL);
}

//*---------------------------------------------------------------------------------------
//* @:V_Durga_FlightChargeState Class 
//*【?】更新
//* 引数：1.Vehicle
//* 返値：void
//*----------------------------------------------------------------------------------------
int V_Durga_FlightChargeState::Update(class VehicleController* pOwner)
{
	const auto& input = pOwner->get_InputResult_Robot();
	auto animator = pOwner->get_AnimatorComponent();
	auto transform = pOwner->get_TransformComponent();
	VEC3 crntPos = transform->get_VEC3ToPos();			// 現在の座標
	auto rigidBody = pOwner->get_RigidBodyComponent();
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
	}

	//*****************************************************************************************
	// 衝撃用煙エフェクトの再生
	//*****************************************************************************************
	if (EFFECT_SPAWN_DURATION <= m_SmokeEffectSpawnTimer)
	{
		m_SmokeEffectSpawnTimer = 0.0f;
		float effectSize = 3.0f;
		int smokeEffectHandle = Master::m_pEffectManager->PlayEffect("Smoke_05", crntPos);
		Master::m_pEffectManager->SetScaleEffect(smokeEffectHandle, effectSize);


		//*****************************************************************************************
		//						パッと光らせる
		//*****************************************************************************************
		float lightIntensity = Master::m_pRandomManager->GetFloatRandom(3.0f, 7.0f);
		TransientPointLightDesc desc;
		desc._color = VEC3(0.8f, 0.4f, 0.1f);
		desc._startRange = 300.0f;
		desc._endRange = 0.0f;
		desc._startIntensity = lightIntensity;
		desc._endIntensity = 0.0f;
		desc._duration = 3.0f;
		auto handle = Master::m_pLightManager->PlayTransientPointLight(
			jetR_Pos,
			desc);
	}

	// ****************************************************
	//				 カメラシェイク
	// ****************************************************
	std::shared_ptr<Camera3D> camera;
	if (camera = Master::m_pDataManager->get_CameraComponent().lock())
	{
		camera->DistanceDecay(0.1, VEC3(0.05f, 0.1f, 0.05f), crntPos, 300.0f);
	}

	//=========================================================================================
	//
	//						チャージ時間を超えたら、飛行ステートへ
	//
	//=========================================================================================
	if (FLIGHT_CHARGE_DURATION <= m_ElapsedTime)
	{
		// ****************************************************
		//				 カメラシェイク
		// ****************************************************
		std::shared_ptr<Camera3D> camera;
		if (camera = Master::m_pDataManager->get_CameraComponent().lock())
		{
			camera->DistanceDecay(1.0, VEC3(0.7f, 0.7f, 0.7f), crntPos, 300.0f);
		}

		return DURGA_V_STATE::DURGA_V_STATE_FLIGHT;
	}

	// 経過時間
	m_ElapsedTime += deltaTime;
	m_SmokeEffectSpawnTimer += deltaTime;

	return DURGA_V_STATE::DURGA_V_STATE_FLIGHT_CHARGE;
}
