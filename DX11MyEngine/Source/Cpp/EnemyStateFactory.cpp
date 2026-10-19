#include "pch.h"
#include "EnemyStateFactory.h"
#include "Component_EnemyController.h"
#include "RendererEngine.h"

// アリ
#include "Ant_StateHeader.h"
// 八面体
#include "Octahedron_StateHeader.h"
// ドローンタイプA
#include "DroneTypeA_StateHeader.h"

using namespace EnemyData;


//*---------------------------------------------------------------------------------------
//*【?】ステートの作成
//*
//* [引数]
//* &_out : 出力先
//* &_createState : 作成する敵ステート
//* &renderer : 描画エンジンの参照
//*
//* [返値]
//* なし 
//*----------------------------------------------------------------------------------------
void EnemyStateFactory::Create(StateMachine<EnemyController>& _out, int _createState, RendererEngine& _renderer)
{
	switch ((ENEMY_TYPE)_createState)
	{
		/*
		* アリ
		*/
	case ENEMY_TYPE::GIANT_ANT_Normal:
		CreateAntState(_out, _renderer);
		break;

		/*
		* 八面体
		*/
	case ENEMY_TYPE::OCTAHEDRON:
		CreateOctahedronState(_out, _renderer);
		break;
		/*
		* ドローン タイプA
		*/
	case ENEMY_TYPE::DRONE_TYPE_A:
		CreateDroneTypeAState(_out, _renderer);
		break;
	default:
		break;
	};
}


//*---------------------------------------------------------------------------------------
//*【?】蟻ステートの作成
//*
//* [引数]
//* &_out : 出力先
//* &renderer : 描画エンジンの参照
//*
//* [返値]
//* なし 
//*----------------------------------------------------------------------------------------
void EnemyStateFactory::CreateAntState(StateMachine<class EnemyController>& _out, RendererEngine& _renderer)
{
	// ルート作成
	std::shared_ptr<Ant_PT_IdleState> pPTIdle = std::make_shared<Ant_PT_IdleState>();					// PT 待機
	std::shared_ptr<Ant_PT_MoveState> pPTMove = std::make_shared<Ant_PT_MoveState>();					// PT 移動
	std::shared_ptr<Ant_AT_TrackingState> pATTracking = std::make_shared<Ant_AT_TrackingState>();		// AT 追従
	std::shared_ptr<Ant_AT_MoveState> pATMove = std::make_shared<Ant_AT_MoveState>();					// AT 移動
	std::shared_ptr<Ant_AT_AttackAcidState> pATAttackAcid = std::make_shared<Ant_AT_AttackAcidState>();	// AT 酸攻撃
	std::shared_ptr<Ant_AT_HitStunState> pATHitStun = std::make_shared<Ant_AT_HitStunState>();					// AT死亡
	std::shared_ptr<Ant_AT_DeadState> pATDead = std::make_shared<Ant_AT_DeadState>();					// AT死亡

	//描画インターフェイス設定
	// add_childの中で子にもセットしてるのでaddする前に呼んで！
	pPTIdle->set_Renderer(&_renderer);
	pPTMove->set_Renderer(&_renderer);
	pATTracking->set_Renderer(&_renderer);
	pATMove->set_Renderer(&_renderer);
	pATAttackAcid->set_Renderer(&_renderer);
	pATHitStun->set_Renderer(&_renderer);
	pATDead->set_Renderer(&_renderer);

	// ********************************************************************************
	// 子ステート作成
	// ********************************************************************************
	//std::shared_ptr<c_Title_LoadProcess> c_LoadProcess = std::make_shared<c_Title_LoadProcess>();		// ロード処理

	// 親を設定
	//c_LoadProcess->set_Parent(pTitleScene);

	// 子を登録
	//pTitleScene->add_Child(c_TITLE::c_TITLE_LOAD_PROCESS, (c_LoadProcess));

	//ステートマシンに登録
	_out.RegisterState(ANT_STATE::ANT_STATE_PATROL_IDLE, std::move(pPTIdle));
	_out.RegisterState(ANT_STATE::ANT_STATE_PATROL_MOVE, std::move(pPTMove));
	_out.RegisterState(ANT_STATE::ANT_STATE_ACTIVE_TRACKING, std::move(pATTracking));
	_out.RegisterState(ANT_STATE::ANT_STATE_ACTIVE_MOVE, std::move(pATMove));
	_out.RegisterState(ANT_STATE::ANT_STATE_ACTIVE_ATTACK_ACID, std::move(pATAttackAcid));
	_out.RegisterState(ANT_STATE::ANT_STATE_ACTIVE_HIT_STUN, std::move(pATHitStun));
	_out.RegisterState(ANT_STATE::ANT_STATE_ACTIVE_DEAD, std::move(pATDead));
}

//*---------------------------------------------------------------------------------------
//*【?】八面体ステートの作成
//*
//* [引数]
//* &_out : 出力先
//* &renderer : 描画エンジンの参照
//*
//* [返値]
//* なし 
//*----------------------------------------------------------------------------------------
void EnemyStateFactory::CreateOctahedronState(StateMachine<class EnemyController>& _out, RendererEngine& _renderer)
{
	// ルート作成
	std::shared_ptr<Octahedron_AT_IdleState> pPTIdle = std::make_shared<Octahedron_AT_IdleState>();					// PT 待機
	std::shared_ptr<Octahedron_AT_MoveState> pPTMove = std::make_shared<Octahedron_AT_MoveState>();					// PT 移動
	std::shared_ptr<Octahedron_AT_TrackingState> pATTracking = std::make_shared<Octahedron_AT_TrackingState>();		// AT 追従
	std::shared_ptr<Octahedron_AT_AttackLaser01State> pATAttackLaser01 = std::make_shared<Octahedron_AT_AttackLaser01State>();		// AT 追従
	std::shared_ptr<Octahedron_AT_DeadState> pATDead = std::make_shared<Octahedron_AT_DeadState>();		// AT 追従

	//描画インターフェイス設定
	// add_childの中で子にもセットしてるのでaddする前に呼んで！
	pPTIdle->set_Renderer(&_renderer);
	pPTMove->set_Renderer(&_renderer);
	pATTracking->set_Renderer(&_renderer);
	pATAttackLaser01->set_Renderer(&_renderer);
	pATDead->set_Renderer(&_renderer);

	//ステートマシンに登録
	_out.RegisterState(OCTAHEDRON_STATE::OCTAHEDRON_STATE_ACTIVE_IDLE, std::move(pPTIdle));
	_out.RegisterState(OCTAHEDRON_STATE::OCTAHEDRON_STATE_ACTIVE_MOVE, std::move(pPTMove));
	_out.RegisterState(OCTAHEDRON_STATE::OCTAHEDRON_STATE_ACTIVE_TRACKING, std::move(pATTracking));
	_out.RegisterState(OCTAHEDRON_STATE::OCTAHEDRON_STATE_ACTIVE_ATTACK_LASER01, std::move(pATAttackLaser01));
	_out.RegisterState(OCTAHEDRON_STATE::OCTAHEDRON_STATE_ACTIVE_DEAD, std::move(pATDead));
}

//*---------------------------------------------------------------------------------------
//*【?】ドローンタイプASステートの作成
//*
//* [引数]
//* &_out : 出力先
//* &renderer : 描画エンジンの参照
//*
//* [返値]
//* なし 
//*----------------------------------------------------------------------------------------
void EnemyStateFactory::CreateDroneTypeAState(StateMachine<class EnemyController>& _out, RendererEngine& _renderer)
{
	// ルート作成
	std::shared_ptr<DroneTypeA_AT_IdleState> pPTIdle = std::make_shared<DroneTypeA_AT_IdleState>();					// PT 待機
	std::shared_ptr<DroneTypeA_AT_MoveState> pPTMove = std::make_shared<DroneTypeA_AT_MoveState>();					// PT 移動
	std::shared_ptr<DroneTypeA_AT_TrackingState> pATTracking = std::make_shared<DroneTypeA_AT_TrackingState>();		// AT 追従
	std::shared_ptr<DroneTypeA_AT_AttackState> pATAttack = std::make_shared<DroneTypeA_AT_AttackState>();		// AT 追従
	std::shared_ptr<DroneTypeA_AT_HitStunState> pATHitStun = std::make_shared<DroneTypeA_AT_HitStunState>();		// AT 追従
	std::shared_ptr<DroneTypeA_AT_DeadState> pATDead = std::make_shared<DroneTypeA_AT_DeadState>();		// AT 追従

	//描画インターフェイス設定
	// add_childの中で子にもセットしてるのでaddする前に呼んで！
	pPTIdle->set_Renderer(&_renderer);
	pPTMove->set_Renderer(&_renderer);
	pATTracking->set_Renderer(&_renderer);
	pATAttack->set_Renderer(&_renderer);
	pATHitStun->set_Renderer(&_renderer);
	pATDead->set_Renderer(&_renderer);

	//ステートマシンに登録
	_out.RegisterState(DRONE_TYPE_A_STATE::DRONE_TYPE_A_STATE_ACTIVE_IDLE, std::move(pPTIdle));
	_out.RegisterState(DRONE_TYPE_A_STATE::DRONE_TYPE_A_STATE_ACTIVE_MOVE, std::move(pPTMove));
	_out.RegisterState(DRONE_TYPE_A_STATE::DRONE_TYPE_A_STATE_ACTIVE_TRACKING, std::move(pATTracking));
	_out.RegisterState(DRONE_TYPE_A_STATE::DRONE_TYPE_A_STATE_ACTIVE_ATTACK, std::move(pATAttack));
	_out.RegisterState(DRONE_TYPE_A_STATE::DRONE_TYPE_A_STATE_ACTIVE_HIT_STUN, std::move(pATHitStun));
	_out.RegisterState(DRONE_TYPE_A_STATE::DRONE_TYPE_A_STATE_ACTIVE_DEAD, std::move(pATDead));
}
