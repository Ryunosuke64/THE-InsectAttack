#include "pch.h"
#include "BuildingStateFactory.h"
#include "Building_State.h"

using namespace Map;

//*---------------------------------------------------------------------------------------
//*【?】ステートの作成
//*
//* [引数]
//* &_out : 出力先
//* &_createState : 作成するステート
//* &renderer : 描画エンジンの参照
//*
//* [返値]
//* なし 
//*----------------------------------------------------------------------------------------
void BuildingStateFactory::CreateBuilding(StateMachine<BuildingController>& _out, int _createState, RendererEngine& _renderer)
{
	// ルートシーン作成
	std::shared_ptr<Building_IdleState> pIdle = std::make_shared<Building_IdleState>();						// 待機
	std::shared_ptr<Building_CllapseInState> pCllapseIn = std::make_shared<Building_CllapseInState>();		// 倒壊開始
	std::shared_ptr<Building_CllapseNowState> pCllapseNow = std::make_shared<Building_CllapseNowState>();	// 倒壊中...
	std::shared_ptr<Building_CllapseEndState> pCllapseEnd = std::make_shared<Building_CllapseEndState>();	// 倒壊終了
	std::shared_ptr<Building_FallState> pFall = std::make_shared<Building_FallState>();						// 沈む
	std::shared_ptr<Building_EndState> pEnd = std::make_shared<Building_EndState>();						// 終了

	//描画インターフェイス設定
	// add_childの中で子シーンにもセットしてるのでaddする前に呼んで！
	pIdle->set_Renderer(&_renderer);
	pCllapseIn->set_Renderer(&_renderer);
	pCllapseNow->set_Renderer(&_renderer);
	pCllapseEnd->set_Renderer(&_renderer);
	pFall->set_Renderer(&_renderer);
	pEnd->set_Renderer(&_renderer);

	//ステートマシンに登録
	_out.RegisterState(BUILDING_STATE::BUILDING_STATE_IDLE, std::move(pIdle));
	_out.RegisterState(BUILDING_STATE::BUILDING_STATE_CLLAPSE_IN, std::move(pCllapseIn));
	_out.RegisterState(BUILDING_STATE::BUILDING_STATE_CLLAPSE_NOW, std::move(pCllapseNow));
	_out.RegisterState(BUILDING_STATE::BUILDING_STATE_CLLAPSE_END, std::move(pCllapseEnd));
	_out.RegisterState(BUILDING_STATE::BUILDING_STATE_FALL, std::move(pFall));
	_out.RegisterState(BUILDING_STATE::BUILDING_STATE_END, std::move(pEnd));
}