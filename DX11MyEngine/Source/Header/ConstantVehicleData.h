#pragma once

/// <summary>
/// ビークルデータをまとめた名前空間
/// </summary>
namespace VehicleData
{
	/// <summary>
	/// ドルガのステート
	/// </summary>
	enum DURGA_V_STATE
	{
		DURGA_V_STATE_IDLE,		// 待機
		DURGA_V_STATE_MOVE,		// 移動
		DURGA_V_STATE_LIFT,		// 持ち上げる
		DURGA_V_STATE_THROW,	// 投げる
		DURGA_V_STATE_DOWN,		// 倒れる
		DURGA_V_STATE_GETUP,	// 立ち上がる
		DURGA_V_STATE_DEAD,		// 死
	};
};