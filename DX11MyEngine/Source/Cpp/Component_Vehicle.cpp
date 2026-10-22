#include "pch.h"
#include "Component_Vehicle.h"
#include "GameObject.h"
#include "Component_SkinnedMeshAnimator.h"
#include "Component_MoveLogic.h"
#include "Component_AI_GunWeapon.h"
#include "Component_Collider.h"
#include "Component_BoxCollider.h"
#include "Component_RigidBody.h"
#include "Component_Health.h"
#include "Component_Physics.h"
#include "RendererEngine.h"
#include "CollisionInfo.h"


using namespace GIGA_Engine;
using namespace VECTOR3;
using namespace VECTOR2;
using namespace EnemyData;
using namespace UtilityData;

//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//* [引数]
//* pOwner : オーナーオブジェクト
//* updateRank : 更新レイヤー
//*----------------------------------------------------------------------------------------
Vehicle::Vehicle(std::weak_ptr<GameObject> pOwner, int updateRank)
	:IComponent(pOwner, updateRank),
	m_StateMachine(this),
	m_ControllMode(CONTROLL_MODE::BOARDING),
	m_StateTimer(0.0f),
	m_AnimSpeed(0.0f),
	m_CrntAnimID(-1),
	m_IsAnim(false),
	m_pHealthComp(nullptr),
	m_pAnimatorComp(nullptr),
	m_pMoveLogicComp(nullptr),
	m_pRigidBodyComp(nullptr),
	m_pTransformComp(nullptr),
	m_pAIGunWeapon(nullptr)
{
	this->set_Tag("Vehicle");
}

//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
Vehicle::~Vehicle()
{

}

//*---------------------------------------------------------------------------------------
//*【?】初期化
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void Vehicle::Start(RendererEngine& renderer)
{
	auto ownerObj = m_pOwner.lock();;

	// アニメーションコンポーネントの取得
	m_pAnimatorComp = ownerObj->get_Component<SkinnedMeshAnimator>().get();

	// 移動コンポーネントの取得
	m_pMoveLogicComp = ownerObj->get_Component<MoveLogic>().get();

	// 物理コンポーネントの取得
	m_pRigidBodyComp = ownerObj->get_Component<RigidBody>().get();

	// HP管理コンポーネントの取得
	m_pHealthComp = m_pOwner.lock()->get_Component<Health>().get();

	// 銃火器コンポーネント
	m_pAIGunWeapon = m_pOwner.lock()->get_Component<AI_GunWeapon>().get();

	// トランスフォームコンポーネントの取得
	m_pTransformComp = ownerObj->get_Transform().lock().get();

	// *********************************************************
	// ステート内で処理を行うようにしたので要らないかも？
	// *********************************************************
	// TODO:処理関数を外から入れるようにする
	// 被弾時の処理登録
	m_pHealthComp->RegisterOnDamage(
		[this, &renderer](float _damage, const PhysicsData::CollisionInfo& collisionInfo)
		{
		}
	);
	// 死亡時の処理登録
	m_pHealthComp->RegisterOnDead(
		[this, &renderer]
		{
		}
	);

	m_pMoveLogicComp->Register(MOVE_BEHAVIOUR_TYPE::LINEAR);
	m_pMoveLogicComp->ChangeBehaviour(MOVE_BEHAVIOUR_TYPE::LINEAR);
}

//*---------------------------------------------------------------------------------------
//*【?】更新
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void Vehicle::LateUpdate(RendererEngine& renderer)
{
	float deltaTime = Master::m_pTimeManager->get_DeltaTime();
	m_StateTimer += deltaTime;	// タイマー進める


	// ステートの実行
	m_StateMachine.Update();

	if (m_pAnimatorComp != nullptr)
	{
		m_pAnimatorComp->set_IsAnim(m_IsAnim);
		m_pAnimatorComp->PlayAnim(deltaTime * m_AnimSpeed);
	}
}

//*---------------------------------------------------------------------------------------
//*【?】ステートの変更
//*
//* [引数]
//*  _state : ステート番号
//* [返値]なし
//*----------------------------------------------------------------------------------------
void Vehicle::ChangeState(const int _state)
{
	m_StateTimer = 0;	// ステート時間をリセット
	m_StateMachine.ChangeState(_state);
}


//*---------------------------------------------------------------------------------------
//*【?】アニメーションの変更
//*
//* [引数]
//*  _id : アニメーション番号
//* [返値]なし
//*----------------------------------------------------------------------------------------
void Vehicle::ChangeAnimation(const int _newId)
{
	// 同じまたは停止中なら返す
	if (_newId == m_CrntAnimID || m_IsAnim == false)
	{
		return;
	}

	// ひとつ前のアニメーションIDセット
	m_pAnimatorComp->set_PrevAnimIndex(static_cast<int>(m_CrntAnimID));

	m_CrntAnimID = _newId;

	// 現在のアニメーションIDセット
	m_pAnimatorComp->set_AnimIndex(static_cast<int>(_newId));
	m_pAnimatorComp->set_AnimTime(0.0f);
}


//*---------------------------------------------------------------------------------------
//*【?】死亡フラグの取得
//*
//* [引数]なし
//* [返値]なし
//*----------------------------------------------------------------------------------------
bool Vehicle::get_IsDead()const
{
	if (m_pHealthComp != nullptr) {
		return m_pHealthComp->get_IsDead();
	}
	else {
		assert(false);
	}
	return false;
}

//*---------------------------------------------------------------------------------------
//*【?】移動ロジックの切り替え
//*
//* [引数]
//*  _moveType : 移動タイプ
//* [返値]なし
//*----------------------------------------------------------------------------------------
void Vehicle::set_MoveLogicState(UtilityData::MOVE_BEHAVIOUR_TYPE _moveType)
{
	if (m_pMoveLogicComp != nullptr)
	{
		m_pMoveLogicComp->ChangeBehaviour(_moveType);
	}
	else 
	{
		assert(false);
	}
}