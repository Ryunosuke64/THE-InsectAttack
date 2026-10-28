#include "pch.h"
#include "Component_VehicleController.h"
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


using namespace VECTOR3;
using namespace VECTOR2;
using namespace UtilityData;
using namespace VehicleData;

//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//* [引数]
//* pOwner : オーナーオブジェクト
//* updateRank : 更新レイヤー
//*----------------------------------------------------------------------------------------
VehicleController::VehicleController(std::weak_ptr<GameObject> pOwner, int updateRank)
	:IComponent(pOwner, updateRank),
	m_StateMachine(this),
	m_ControllMode(V_CONTROLL_MODE::REMOTE),
	m_InputType(V_INPUT_TYPE::ROBOT),
	m_StateTimer(0.0f),
	m_AnimSpeed(0.0f),
	m_MoveSpeed(0.0f),
	m_WeaponID(-1),
	m_IsPlayAnim(false),
	m_pHealthComp(nullptr),
	m_pAnimatorComp(nullptr),
	m_pMoveLogicComp(nullptr),
	m_pRigidBodyComp(nullptr),
	m_pTransformComp(nullptr),
	m_pTarget(nullptr),
	m_pAIGunWeapon(nullptr)
{
	this->set_Tag("VehicleController");
}

//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
VehicleController::~VehicleController()
{

}

//*---------------------------------------------------------------------------------------
//*【?】初期化
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void VehicleController::Start(RendererEngine& renderer)
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
void VehicleController::LateUpdate(RendererEngine& renderer)
{
	float deltaTime = Master::m_pTimeManager->get_DeltaTime();
	m_StateTimer += deltaTime;	// タイマー進める

	//
	// 入力チェック
	//
	switch (m_InputType)
	{
	case VehicleData::V_INPUT_TYPE::ROBOT:
		m_InputResult = CheckInputRobot();
		break;
	default:
		break;
	}

	// ステートの実行
	m_StateMachine.Update();

	if (m_pAnimatorComp != nullptr)
	{
		//m_pAnimatorComp->set_IsAnim(m_IsPlayAnim);
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
void VehicleController::ChangeState(const int _state)
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
void VehicleController::ChangeAnimation(const int _newId)
{
	// 同じまたは停止中なら返す
	if (m_IsPlayAnim == false)
	{
		return;
	}
	m_pAnimatorComp->ChangeAnim(_newId);
}

//*---------------------------------------------------------------------------------------
//*【?】死亡フラグの取得
//*
//* [引数]なし
//* [返値]なし
//*----------------------------------------------------------------------------------------
bool VehicleController::get_IsDead()const
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
void VehicleController::set_MoveLogicState(UtilityData::MOVE_BEHAVIOUR_TYPE _moveType)
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

//*---------------------------------------------------------------------------------------
//*【?】ロボットの入力をチェックする
//*
//* [引数] 
//* なし
//* [返値]
//* 入力情報 
//*----------------------------------------------------------------------------------------
const VehicleData::VehicleInputResult VehicleController::CheckInputRobot()
{
	VehicleInputResult_Robot result;
	result.isMoveF = GetInput(GAME_CONFIG::MOVE_FORWARD);
	result.isMoveB = GetInput(GAME_CONFIG::MOVE_BACK);
	result.isMoveR = GetInput(GAME_CONFIG::MOVE_RIGHT);
	result.isMoveL = GetInput(GAME_CONFIG::MOVE_LEFT);
	result.isRotR = GetInput(GAME_CONFIG::DURGA_ROT_RIGHT);
	result.isRotL = GetInput(GAME_CONFIG::DURGA_ROT_LEFT);
	result.isLift = GetInput(GAME_CONFIG::DURGA_LIFT);
	result.isThrow = GetInput(GAME_CONFIG::DURGA_THROW);
	return result;
}
//*---------------------------------------------------------------------------------------
//*【?】ロボットの入力情報を取得する
//*
//* [引数] 
//* なし
//* [返値]
//* 入力情報 
//*----------------------------------------------------------------------------------------
const VehicleData::VehicleInputResult_Robot& 
VehicleController::get_InputResult_Robot()const
{
	const VehicleInputResult_Robot* input =
		std::get_if<VehicleInputResult_Robot>(&m_InputResult);

	// 変換失敗
	if (input == nullptr)
	{
		assert(false);
		return{};
	}

	return *input;
}
