#include "pch.h"
#include "VehicleStateFactory.h"
#include "RendererEngine.h"

// ドルガ
#include "V_Durga_StateHeader.h"

using namespace VehicleData;


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
void VehicleStateFactory::Create(StateMachine<Vehicle>& _out, int _createState, RendererEngine& _renderer)
{
	switch ((VEHICLE_TYPE)_createState)
	{
		/*
		* ドルガ
		*/
	case VEHICLE_TYPE::DURGA:
		CreateDurgaState(_out, _renderer);
		break;
	};
}


//*---------------------------------------------------------------------------------------
//*【?】[ドルガ]ステートの作成
//*
//* [引数]
//* &_out : 出力先
//* &renderer : 描画エンジンの参照
//*
//* [返値]
//* なし 
//*----------------------------------------------------------------------------------------
void VehicleStateFactory::CreateDurgaState(StateMachine<class Vehicle>& _out, RendererEngine& _renderer)
{
	//// ルート作成
	//std::shared_ptr<Ant_PT_IdleState> pPTIdle = std::make_shared<Ant_PT_IdleState>();					// PT 待機
	//std::shared_ptr<Ant_PT_MoveState> pPTMove = std::make_shared<Ant_PT_MoveState>();					// PT 移動
	//std::shared_ptr<Ant_AT_TrackingState> pATTracking = std::make_shared<Ant_AT_TrackingState>();		// AT 追従
	//std::shared_ptr<Ant_AT_MoveState> pATMove = std::make_shared<Ant_AT_MoveState>();					// AT 移動
	//std::shared_ptr<Ant_AT_AttackAcidState> pATAttackAcid = std::make_shared<Ant_AT_AttackAcidState>();	// AT 酸攻撃
	//std::shared_ptr<Ant_AT_HitStunState> pATHitStun = std::make_shared<Ant_AT_HitStunState>();					// AT死亡
	//std::shared_ptr<Ant_AT_DeadState> pATDead = std::make_shared<Ant_AT_DeadState>();					// AT死亡

	////描画インターフェイス設定
	//// add_childの中で子にもセットしてるのでaddする前に呼んで！
	//pPTIdle->set_Renderer(&_renderer);
	//pPTMove->set_Renderer(&_renderer);
	//pATTracking->set_Renderer(&_renderer);
	//pATMove->set_Renderer(&_renderer);
	//pATAttackAcid->set_Renderer(&_renderer);
	//pATHitStun->set_Renderer(&_renderer);
	//pATDead->set_Renderer(&_renderer);

	//// ********************************************************************************
	//// 子ステート作成
	//// ********************************************************************************
	////std::shared_ptr<c_Title_LoadProcess> c_LoadProcess = std::make_shared<c_Title_LoadProcess>();		// ロード処理

	//// 親を設定
	////c_LoadProcess->set_Parent(pTitleScene);

	//// 子を登録
	////pTitleScene->add_Child(c_TITLE::c_TITLE_LOAD_PROCESS, (c_LoadProcess));

	////ステートマシンに登録
	//_out.RegisterState(ANT_STATE::ANT_STATE_PATROL_IDLE, std::move(pPTIdle));
	//_out.RegisterState(ANT_STATE::ANT_STATE_PATROL_MOVE, std::move(pPTMove));
	//_out.RegisterState(ANT_STATE::ANT_STATE_ACTIVE_TRACKING, std::move(pATTracking));
	//_out.RegisterState(ANT_STATE::ANT_STATE_ACTIVE_MOVE, std::move(pATMove));
	//_out.RegisterState(ANT_STATE::ANT_STATE_ACTIVE_ATTACK_ACID, std::move(pATAttackAcid));
	//_out.RegisterState(ANT_STATE::ANT_STATE_ACTIVE_HIT_STUN, std::move(pATHitStun));
	//_out.RegisterState(ANT_STATE::ANT_STATE_ACTIVE_DEAD, std::move(pATDead));
}