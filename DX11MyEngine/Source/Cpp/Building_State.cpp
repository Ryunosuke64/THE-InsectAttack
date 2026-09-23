#include "pch.h"
#include "Building_State.h"
#include "RendererEngine.h"
#include "Component_3DCamera.h"
#include "Component_BuildingController.h"
#include "Component_RigidBody.h"
#include "Component_BoxCollider.h"
#include "Component_MeshCollider.h"
#include "Component_Health.h"

using namespace VECTOR2;
using namespace VECTOR3;
using namespace VECTOR4;
using namespace BuildingData;
using namespace UtilityData;
using namespace Tool;

/* 倒壊パラメータ */
constexpr float BUILDING_COLLAPSE_TIME_MIN = 2.0f;			// 倒壊にかかる時間の最小値
constexpr float BUILDING_COLLAPSE_TIME_MAX = 3.5f;			// 倒壊にかかる時間の最大値
constexpr float BUILDING_COLLAPSE_SPEED = 25.0f;			// 建物の倒壊スピード
constexpr float BUILDING_COLLAPSE_END_TIME = 3.0f;			// 倒壊終了から落下までの時間
constexpr float BUILDING_FALL_SPEED = 20.0f;				// 建物の落下スピード
constexpr float BUILDING_COLLAPSE_TWEEN_DURATION = 2.0f;	// 倒壊のTweenの時間
constexpr float BUILDING_COLLAPSE_SUNK_POSY_AMOUNT = 6.0f;	// 倒壊のY座標の沈み量

/* サウンド */
constexpr float BUILDING_DESTRUCTION_SOUND_RADIUS = 500.0f;	// 破壊時の音の聞こえる範囲
constexpr float BUILDING_FALL_SOUND_RADIUS = 500.0f;		// 落下時の音の聞こえる範囲

/* カメラシェイク */
constexpr float SHAKE_DURATION = 3.0f;					// 持続時間
constexpr float SHAKE_LENGTH = 0.3f;					// 強さ
constexpr float SHAKE_MAX_RANGE = 300.0f;				// 揺れが影響する最大距離
constexpr float COLLAPSE_IN_SHAKE_DURATION = 3.0f;		// 倒壊始めのカメラシェイクの持続時間
constexpr float COLLAPSE_IN_SHAKE_LENGTH = 0.05f;		// 倒壊始めのカメラシェイクの強さ


//////////////////////////////////////////////////////////////////////////////////////////

//								待機ステート
 
//////////////////////////////////////////////////////////////////////////////////////////
void Building_IdleState::OnEnter(BuildingController* pOwner)
{
	m_FrameCounter = 0;
}

void Building_IdleState::OnExit(BuildingController* pOwner)
{

}

int Building_IdleState::Update(BuildingController* pOwner)
{
	const auto healthComp = pOwner->get_HealthComp();

	// ダメージを受けた場合、エフェクトを再生する
	if (pOwner->get_IsOnDamage())
	{
		const auto transform = pOwner->get_OwnerObj().lock()->get_Transform().lock();
		VEC3 pos = healthComp->get_CollisionInfo().get_HitPoint();
		VEC3 hitNormal = healthComp->get_CollisionInfo().get_HitNormal();	// 衝突法線
		VEC3 effectRotAxis = VEC3(0.0f);
		float effectRotAngle = 0;

		// 法線から回転を求める
		GetRotationFromNormal(hitNormal, VEC3(0.0f, 0.0f, -1.0f), effectRotAxis, effectRotAngle);

		VEC3 effectScale = 3.0f;
		int handle = Master::m_pEffectManager->PlayEffect("Destruction_Fragments_Small");

		Master::m_pEffectManager->SetPositionEffect(handle, pos.x, pos.y, pos.z);
		Master::m_pEffectManager->SetRotationEffect(handle, effectRotAxis, effectRotAngle);
		Master::m_pEffectManager->SetScaleEffect(handle, effectScale.x, effectScale.y, effectScale.z);
	}

	float halfMaxHP = healthComp->get_MaxHP() * 0.5f;	// 最大HPの半分の値

	// 体力が半分を下回ったら、倒壊始めステートへ遷移する
	if (healthComp->get_CrntHP() < halfMaxHP){
		return BUILDING_STATE::BUILDING_STATE_CLLAPSE_IN;	// 倒壊始めステートへ
	}

	return BUILDING_STATE::BUILDING_STATE_IDLE;
}


//////////////////////////////////////////////////////////////////////////////////////////

//								倒壊始めステート

//////////////////////////////////////////////////////////////////////////////////////////
void Building_CllapseInState::OnEnter(BuildingController* pOwner)
{
	auto ownerObj =pOwner->get_OwnerObj().lock();

	VEC3 pos = ownerObj->get_Transform().lock()->get_VEC3ToPos();
	VEC3 rot = ownerObj->get_Transform().lock()->get_VEC3ToRotateToRad();
	Master::m_pSoundManager->Play_3D(SOUND_TYPE::SE, INT_CAST(SOUND_ID::BUILDING_DESTRUCTION), pos, BUILDING_DESTRUCTION_SOUND_RADIUS);


	//
	// 倒壊のY座標をTweenで沈める
	//
	float tweenEndPosY = pos.y - BUILDING_COLLAPSE_SUNK_POSY_AMOUNT;	// 目標のY座標を設定
	float tweenDuration = BUILDING_COLLAPSE_TWEEN_DURATION;
	Master::m_pTweenManager->AddTween(&m_SunkTweenPosY, pos.y, tweenEndPosY, tweenDuration, TweenType::EASE_IN);

	//
	// 崩壊の目標角度を設定する
	//
	float collapseTargetAngle = Master::m_pRandomManager->GetFloatRandom(-0.3f, 0.3f);	// 目標角度を決める
	pOwner->set_CollapseTargetAngle(collapseTargetAngle);								// 目標角度を設定（崩壊中にも使用するため）

	//
	// 倒壊の回転角度をTweenで変化させる
	//
	VEC3 tweenEndRot = VEC3(collapseTargetAngle, rot.y, collapseTargetAngle);	// 目標角度を設定(yに入れると崩壊の仕方がおかしくなるのでxとzのみ)
	Master::m_pTweenManager->AddTween(&m_SunkTweenRot, rot, tweenEndRot, tweenDuration, TweenType::LINEAR);

	// ****************************************************
	//				 カメラシェイク
	// ****************************************************
	std::shared_ptr<Camera3D> camera;
	if (camera = Master::m_pDataManager->get_CameraComponent().lock())
	{
		camera->DistanceDecay(COLLAPSE_IN_SHAKE_DURATION, VEC3(COLLAPSE_IN_SHAKE_LENGTH), pos, SHAKE_MAX_RANGE);
	}


	//*****************************************************************************************
	//						エフェクト再生
	//					煙がブワッと出てくる感じ
	//*****************************************************************************************
	float scale = 10.0f;

	for (int i = 0; i < 5; i++)
	{
		rot.x = Master::m_pRandomManager->GetFloatRandom(-G_PI_F, G_PI_F);
		rot.y = Master::m_pRandomManager->GetFloatRandom(-G_PI_F, G_PI_F);
		rot.z = Master::m_pRandomManager->GetFloatRandom(-G_PI_F, G_PI_F);

		int handle = Master::m_pEffectManager->PlayEffect("Explosion_Smoke_02");
		Master::m_pEffectManager->SetPositionEffect(handle, pos.x, pos.y, pos.z);
		Master::m_pEffectManager->SetRotationEffect(handle, rot.x, rot.y, rot.z);
		Master::m_pEffectManager->SetScaleEffect(handle, scale, scale, scale);
	}
}

void Building_CllapseInState::OnExit(BuildingController* pOwner)
{
	auto ownerObj = pOwner->get_OwnerObj().lock();

	VEC3 pos = ownerObj->get_Transform().lock()->get_VEC3ToPos();
	Master::m_pSoundManager->Play_3D(SOUND_TYPE::SE, INT_CAST(SOUND_ID::BUILDING_DESTRUCTION), pos, BUILDING_DESTRUCTION_SOUND_RADIUS);

	// 完全に破壊されたら、コライダーをオフにする
	auto collider = ownerObj->get_Component<MeshCollider>();
	if (collider) {
		collider->remove_CollisionBitMask(COLLISION_CATEGORY::ENEMY_BULLET);
		collider->remove_CollisionBitMask(COLLISION_CATEGORY::PLAYER_BULLET);
		collider->remove_CollisionBitMask(COLLISION_CATEGORY::PLAYER);
		collider->remove_CollisionBitMask(COLLISION_CATEGORY::ENEMY);
	}

	ownerObj->set_IsStatic(false);	// 動的オブジェクトに変更
}

int Building_CllapseInState::Update(BuildingController* pOwner)
{
	// 完全に破壊されたら、倒壊中ステートへ遷移する
	if (pOwner->get_IsDestruction())
	{
		return BUILDING_STATE::BUILDING_STATE_CLLAPSE_NOW;	// 倒壊中ステートへ
	}

	auto rigidBody = pOwner->get_RigidBodyComp();
	//auto transform = pOwner->get_OwnerObj().lock()->get_Transform().lock();

	// 倒壊のY座標を更新
	//VEC3 pos = transform->get_VEC3ToPos();
	VEC3 pos = rigidBody->GetWorldPotision();
	pos.y = m_SunkTweenPosY;	// 倒壊のY座標を更新

	// クオータニオンへ変換
	VEC4 rot = VEC4::FromXMVECTOR(
		DirectX::XMQuaternionRotationRollPitchYaw(
			m_SunkTweenRot.x,
			m_SunkTweenRot.y,
			m_SunkTweenRot.z
		)
	);
	rigidBody->SetWorldTransform(pos, rot);

	// 倒壊の回転角度を更新
	//VEC3 rot = transform->get_VEC3ToRotateToRad();
	//rot.x = m_SunkTweenRot.x;
	//rot.z = m_SunkTweenRot.z;

	//transform->set_Pos(pos);
	//transform->set_RotateToRad(rot);

	m_FrameCounter++;
	
	//*****************************************************************************************
	//						エフェクト再生
	//					煙がごわごわ出てくる感じ
	//*****************************************************************************************
	if (m_FrameCounter % 180 == 0)
	{
		float scale = 10.0f;

		VEC3 effectRot;
		effectRot.x = Master::m_pRandomManager->GetFloatRandom(-G_PI_F, G_PI_F);
		effectRot.y = Master::m_pRandomManager->GetFloatRandom(-G_PI_F, G_PI_F);
		effectRot.z = Master::m_pRandomManager->GetFloatRandom(-G_PI_F, G_PI_F);

		int handle = Master::m_pEffectManager->PlayEffect("Explosion_Smoke_02");
		Master::m_pEffectManager->SetPositionEffect(handle, pos);
		Master::m_pEffectManager->SetRotationEffect(handle, effectRot);
		Master::m_pEffectManager->SetScaleEffect(handle, scale, scale, scale);
	}
	return BUILDING_STATE::BUILDING_STATE_CLLAPSE_IN;
}


//////////////////////////////////////////////////////////////////////////////////////////

//								倒壊中ステート

//////////////////////////////////////////////////////////////////////////////////////////
void Building_CllapseNowState::OnEnter(BuildingController* pOwner)
{
	auto transform = pOwner->get_OwnerObj().lock()->get_Transform().lock();
	m_CollapseTime = Master::m_pRandomManager->GetFloatRandom(BUILDING_COLLAPSE_TIME_MIN, BUILDING_COLLAPSE_TIME_MAX);	// 倒壊にかかる時間をランダムで決める

	m_CrntCollapseTime = 0.0f;									// 現在の倒壊時間を初期化
	m_SunkRateY = 0.0f;											// どのくらい沈んだかを初期化
	m_FrameCounter = 0;											// フレームカウンターを初期化
	m_StartRot = transform->get_VEC3ToRotateToRad();			// 開始時の回転角度を取得する
	m_CollapseTargetAngle = pOwner->get_CollapseTargetAngle();	// 崩壊開始時に設定した目標角度を取得する
}

void Building_CllapseNowState::OnExit(BuildingController* pOwner)
{
	auto transform = pOwner->get_OwnerObj().lock()->get_Transform().lock();

	VEC3 pos = transform->get_VEC3ToPos();

	// ****************************************************
	//				 カメラシェイク
	// ****************************************************
	std::shared_ptr<Camera3D> camera;
	if (camera = Master::m_pDataManager->get_CameraComponent().lock())
	{
		camera->DistanceDecay(SHAKE_DURATION, VEC3(SHAKE_LENGTH), pos, SHAKE_MAX_RANGE);
	}

	//*****************************************************************************************
	//						エフェクト再生
	//				煙と破片と、ドデカい爆発が起きる感じ
	//*****************************************************************************************
	float scale = 10.0f;
	int handle = Master::m_pEffectManager->PlayEffect("Smoke_02");
	Master::m_pEffectManager->SetPositionEffect(handle, pos.x, pos.y + m_SunkRateY, pos.z);
	Master::m_pEffectManager->SetRotationEffect(handle, 0.0f, 0.0f, 0.0f);
	Master::m_pEffectManager->SetScaleEffect(handle, scale, scale, scale);

	handle = Master::m_pEffectManager->PlayEffect("Destruction_Fragments");
	Master::m_pEffectManager->SetPositionEffect(handle, pos.x, pos.y + m_SunkRateY, pos.z);
	Master::m_pEffectManager->SetRotationEffect(handle, 0.0f, 0.0f, 0.0f);
	Master::m_pEffectManager->SetScaleEffect(handle, scale, scale, scale);

	VEC3 explosionRot = 0.0f;
	VEC3 explosionScale = 5.0f;
	handle = Master::m_pEffectManager->PlayEffect("Explosion_03");
	Master::m_pEffectManager->SetPositionEffect(handle, pos.x, pos.y + m_SunkRateY, pos.z);
	Master::m_pEffectManager->SetRotationEffect(handle, explosionRot.x, explosionRot.y, explosionRot.z);
	Master::m_pEffectManager->SetScaleEffect(handle, explosionScale.x, explosionScale.y, explosionScale.z);
}

int Building_CllapseNowState::Update(BuildingController* pOwner)
{
	float deltaTime = Master::m_pTimeManager->get_DeltaTime();
	m_CrntCollapseTime += deltaTime;	// 倒壊時間を更新

	if (pOwner->get_IsActiveOwnerObj())
	{
		//auto transform = pOwner->get_OwnerObj().lock()->get_Transform().lock();
		auto rigidBody = pOwner->get_RigidBodyComp();

		VEC3 pos = rigidBody->GetWorldPotision();

		float t = m_CrntCollapseTime / m_CollapseTime;
		float easeIn = Tool::Easing::EaseInSine(t);

		float speed = BUILDING_COLLAPSE_SPEED * easeIn;;
		
		/* 沈んでいくような感じに */
		pos.y -= speed * deltaTime;

		// どのくらい沈んだかを保持
		m_SunkRateY += speed * deltaTime;

		/* 倒れるような感じに */
		float crntAngle = m_CollapseTargetAngle * easeIn;

		// クオータニオンへ変換
		VEC4 rot = VEC4::FromXMVECTOR(
			DirectX::XMQuaternionRotationRollPitchYaw(
				m_StartRot.x + crntAngle,
				m_StartRot.y,
				m_StartRot.z + crntAngle
			)
		);
		// リジッドボディへ設定
		rigidBody->SetWorldTransform(pos, rot);



		m_FrameCounter++;

		//*****************************************************************************************
		//						エフェクト再生
		//				崩れていく際に、炎が出ている感じにする
		//*****************************************************************************************
		if (m_FrameCounter % 30 == 0)
		{
			VEC3 explosionRot;
			VEC3 explosionScale = 4.0f;
			explosionRot.x = Master::m_pRandomManager->GetFloatRandom(-G_PI_F, G_PI_F);
			explosionRot.y = Master::m_pRandomManager->GetFloatRandom(-G_PI_F, G_PI_F);
			explosionRot.z = Master::m_pRandomManager->GetFloatRandom(-G_PI_F, G_PI_F);
			float explosionPosY_Offset;
			explosionPosY_Offset = Master::m_pRandomManager->GetFloatRandom(1.0f, 50.0f);
			int handle = Master::m_pEffectManager->PlayEffect("Explosion_02");
			Master::m_pEffectManager->SetPositionEffect(handle, pos.x, pos.y + explosionPosY_Offset, pos.z);
			Master::m_pEffectManager->SetRotationEffect(handle, explosionRot.x, explosionRot.y, explosionRot.z);
			Master::m_pEffectManager->SetScaleEffect(handle, explosionScale.x, explosionScale.y, explosionScale.z);
		}
	}

	if(m_CollapseTime <= m_CrntCollapseTime )
	{
		return BUILDING_STATE::BUILDING_STATE_CLLAPSE_END;	// 倒壊終了ステートへ
	}

	return BUILDING_STATE::BUILDING_STATE_CLLAPSE_NOW;
}


//////////////////////////////////////////////////////////////////////////////////////////

//								倒壊終了ステート

//////////////////////////////////////////////////////////////////////////////////////////
void Building_CllapseEndState::OnEnter(BuildingController* pOwner)
{
	VEC3 pos = pOwner->get_OwnerObj().lock()->get_Transform().lock()->get_VEC3ToPos();
	Master::m_pSoundManager->Play_3D(SOUND_TYPE::SE, INT_CAST(SOUND_ID::BUILDING_FALL), pos, BUILDING_FALL_SOUND_RADIUS);


	m_CrntCollapseEndTime = 0.0f;

	m_Rot = pOwner->get_OwnerObj().lock()->get_Transform().lock()->get_VEC3ToRotateToRad();
}

void Building_CllapseEndState::OnExit(BuildingController* pOwner)
{

}

int Building_CllapseEndState::Update(BuildingController* pOwner)
{
	float deltaTime = Master::m_pTimeManager->get_DeltaTime();

	m_CrntCollapseEndTime += deltaTime;	// 倒壊終了時間を更新

	if (pOwner->get_IsActiveOwnerObj())
	{
		auto rigidBody = pOwner->get_RigidBodyComp();

		VEC3 pos = rigidBody->GetWorldPotision();

		float t = m_CrntCollapseEndTime / BUILDING_COLLAPSE_END_TIME;
		float easeBack = Tool::Easing::EaseOutBack(t);

		// クオータニオンへ変換
		VEC4 rot = VEC4::FromXMVECTOR(
			DirectX::XMQuaternionRotationRollPitchYaw(
				Tool::Lerp(m_Rot.x, m_Rot.x + 0.1f, easeBack),
				0.0f,
				Tool::Lerp(m_Rot.z, m_Rot.z + 0.1f, easeBack)
			)
		);
		// リジッドボディへ設定
		rigidBody->SetWorldTransform(pos, rot);
	}

	// 倒壊終了時間が一定以上経過したら落下ステートへ
	if (BUILDING_COLLAPSE_END_TIME <= m_CrntCollapseEndTime)
	{
		return BUILDING_STATE::BUILDING_STATE_FALL;	// 落下ステートへ
	}

	return BUILDING_STATE::BUILDING_STATE_CLLAPSE_END;
}


//////////////////////////////////////////////////////////////////////////////////////////

//					倒壊が終わって、裏世界へ落ちていくステート

//////////////////////////////////////////////////////////////////////////////////////////
void Building_FallState::OnEnter(BuildingController* pOwner)
{

}

void Building_FallState::OnExit(BuildingController* pOwner)
{

}

int Building_FallState::Update(BuildingController* pOwner)
{
	float deltaTime = Master::m_pTimeManager->get_DeltaTime();

	if (pOwner->get_IsActiveOwnerObj())
	{
		auto rigidBody = pOwner->get_RigidBodyComp();

		VEC3 pos = rigidBody->GetWorldPotision();

		pos.y -= BUILDING_FALL_SPEED * deltaTime;

		rigidBody->Teleport(pos);

		if (pos.y < -100.0f)
		{
			return BUILDING_STATE::BUILDING_STATE_END;	// 終了ステートへ
		}
	}
	return BUILDING_STATE::BUILDING_STATE_FALL;
}


//////////////////////////////////////////////////////////////////////////////////////////

//								終了ステート

//////////////////////////////////////////////////////////////////////////////////////////
void Building_EndState::OnEnter(BuildingController* pOwner)
{

}

void Building_EndState::OnExit(BuildingController* pOwner)
{

}

int Building_EndState::Update(BuildingController* pOwner)
{
	pOwner->get_OwnerObj().lock()->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_DELETE);

	return BUILDING_STATE::BUILDING_STATE_END;
}