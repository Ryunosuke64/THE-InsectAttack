#pragma once
#include "ConstantVehicleData.h"


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:VehicleStateFactory Class --- */
//
// 【?】ビークルのステートの作成を行う
//
// ***************************************************************************************
class VehicleStateFactory
{
public:
	/// <summary>
	/// ステートを作成して挿入する
	/// </summary>
	/// <param name="_out">出力先</param>
	/// <param name="_createScene">作成するシーン</param>
	/// <param name="_renderer">描画エンジンの参照</param>
	static void Create(StateMachine<class VehicleController>& _out, int _createState, RendererEngine& _renderer);

private:
	/// <summary>
	/// ドルガの作成
	/// </summary>
	/// <param name="_out">出力先</param>
	/// <param name="_renderer">描画エンジンの参照</param>
	static void CreateDurgaState(StateMachine<class VehicleController>& _out, RendererEngine& _renderer);


};

