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
void VehicleStateFactory::Create(StateMachine<VehicleController>& _out, int _createState, RendererEngine& _renderer)
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
void VehicleStateFactory::CreateDurgaState(StateMachine<class VehicleController>& _out, RendererEngine& _renderer)
{
	// ルート作成
	std::shared_ptr<V_Durga_IdleState> pIdle = std::make_shared<V_Durga_IdleState>();							// 待機
	std::shared_ptr<V_Durga_MoveState> pMove = std::make_shared<V_Durga_MoveState>();							// 移動
	std::shared_ptr<V_Durga_LiftState> pLift = std::make_shared<V_Durga_LiftState>();							// 持ち上げる
	std::shared_ptr<V_Durga_LiftIdleState> pLiftIdle = std::make_shared<V_Durga_LiftIdleState>();				// 持ち上げ待機
	std::shared_ptr<V_Durga_LiftMoveState> pLiftMove = std::make_shared<V_Durga_LiftMoveState>();				// 持ち上げ移動
	std::shared_ptr<V_Durga_ThrowState> pThrow = std::make_shared<V_Durga_ThrowState>();						// 投げ飛ばす
	std::shared_ptr<V_Durga_FlightChargeState> pFlightCharge = std::make_shared<V_Durga_FlightChargeState>();	// 飛行準備
	std::shared_ptr<V_Durga_FlightState> pFlight = std::make_shared<V_Durga_FlightState>();	// 飛行準備
	std::shared_ptr<V_Durga_FlightFallState> pFlightFall = std::make_shared<V_Durga_FlightFallState>();	// 飛行準備
	std::shared_ptr<V_Durga_FlightLandingState> pFlightLanding = std::make_shared<V_Durga_FlightLandingState>();	// 飛行準備

	//描画インターフェイス設定
	// add_childの中で子にもセットしてるのでaddする前に呼んで！
	pIdle->set_Renderer(&_renderer);
	pMove->set_Renderer(&_renderer);
	pLift->set_Renderer(&_renderer);
	pLiftIdle->set_Renderer(&_renderer);
	pLiftMove->set_Renderer(&_renderer);
	pThrow->set_Renderer(&_renderer);
	pFlightCharge->set_Renderer(&_renderer);
	pFlight->set_Renderer(&_renderer);
	pFlightFall->set_Renderer(&_renderer);
	pFlightLanding->set_Renderer(&_renderer);

	//ステートマシンに登録
	_out.RegisterState(DURGA_V_STATE::DURGA_V_STATE_IDLE, std::move(pIdle));
	_out.RegisterState(DURGA_V_STATE::DURGA_V_STATE_MOVE, std::move(pMove));
	_out.RegisterState(DURGA_V_STATE::DURGA_V_STATE_LIFT, std::move(pLift));
	_out.RegisterState(DURGA_V_STATE::DURGA_V_STATE_LIFT_IDLE, std::move(pLiftIdle));
	_out.RegisterState(DURGA_V_STATE::DURGA_V_STATE_LIFT_MOVE, std::move(pLiftMove));
	_out.RegisterState(DURGA_V_STATE::DURGA_V_STATE_THROW, std::move(pThrow));
	_out.RegisterState(DURGA_V_STATE::DURGA_V_STATE_FLIGHT_CHARGE, std::move(pFlightCharge));
	_out.RegisterState(DURGA_V_STATE::DURGA_V_STATE_FLIGHT, std::move(pFlight));
	_out.RegisterState(DURGA_V_STATE::DURGA_V_STATE_FLIGHT_FALL, std::move(pFlightFall));
	_out.RegisterState(DURGA_V_STATE::DURGA_V_STATE_FLIGHT_LANDING, std::move(pFlightLanding));
}