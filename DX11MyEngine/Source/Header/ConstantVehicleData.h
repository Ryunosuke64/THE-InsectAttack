#pragma once

/// <summary>
/// ビークルデータをまとめた名前空間
/// </summary>
namespace VehicleData
{
	/// <summary>
	/// 操作方法
	/// </summary>
	enum class V_CONTROLL_MODE
	{
		BOARDING,	// 搭乗
		REMOTE,		// リモート（鉄人形式）

		NUM
	};

	/// <summary>
	/// 入力タイプ
	/// </summary>
	enum class V_INPUT_TYPE
	{
		ROBOT,		// ロボ操作

		NUM
	};

	/// <summary>
	/// ビークルの種類
	/// </summary>
	enum class VEHICLE_TYPE
	{
		DURGA,
	};

	/// <summary>
	/// ドルガのステート
	/// </summary>
	enum DURGA_V_STATE
	{
		DURGA_V_STATE_IDLE,		// 待機
		DURGA_V_STATE_MOVE,		// 移動
		DURGA_V_STATE_LIFT,		// 持ち上げる
		DURGA_V_STATE_LIFT_IDLE,// 持ち上げ中 待機
		DURGA_V_STATE_LIFT_MOVE,// 持ち上げ中 移動
		DURGA_V_STATE_THROW,	// 投げる

		DURGA_V_STATE_FLIGHT_CHARGE,	// 飛行 - 飛行準備
		DURGA_V_STATE_FLIGHT,			// 飛行 - 飛行中
		DURGA_V_STATE_FLIGHT_FALL,		// 飛行 - 落下
		DURGA_V_STATE_FLIGHT_LANDING,	// 飛行 - 着地

		DURGA_V_STATE_DOWN,		// 倒れる
		DURGA_V_STATE_GETUP,	// 立ち上がる
		DURGA_V_STATE_DEAD,		// 死
	};

	/// <summary>
	/// アニメーションID - ドルガ -
	/// </summary>
	enum class DURGA_ANIM_ID
	{
		DOWN			= 0,	// 倒れる

		FLIGHT			= 1,	// 飛行 - 飛行中
		FLIGHT_CHARGE	= 2,	// 飛行 - 準備
		FLIGHT_FALL		= 3,	// 飛行 - 落下
		FLIGHT_LANDING	= 4,	// 飛行 - 着地

		GETUP			= 5,	// 起き上がる
		IDLE			= 6,	// 起き上がる
		LIFT			= 7,	// 持ち上げる
		PUNCH_A			= 8,	// パンチ - A

		RP_REATTACH			= 9,	// ロケットパンチ - 再装着
		RP_REATTACH_STAND	= 10,	// ロケットパンチ - 再装着待ち
		RP_SHOT				= 11,	// ロケットパンチ - 発射
		RP_READY			= 12,	// ロケットパンチ - 発射準備

		THROW			= 13,	// 投げる
		TPOSE			= 14,	// Tポーズ
		WALK_F_ENTER	= 15,	// 歩き始め
		WALK_F_L		= 16,	// 左足前進
		WALK_F_L_LIFT	= 17,	// 左足前進 - 持ち上げ状態
		WALK_F_R		= 18,	// 右足前進
		WALK_F_R_LIFT	= 19,	// 右足前進 - 持ち上げ状態
	};

	// ビークル出現時のセットアップデータ
	struct VehicleSpawnData
	{
		VEHICLE_TYPE type;			// 種類
		VECTOR3::VEC3 position;     // 位置
		VECTOR3::VEC3 rotation;     // 回転
		float hp = 0.0f;			// HP
	};

	/// <summary>
	/// ビークルの入力情報を格納する
	/// </summary>
	struct VehicleInputResult_Robot
	{
		bool isMoveF = false;		// 移動 - 前進
		bool isMoveB = false;		// 移動 - 後退
		bool isMoveR = false;		// 移動 - 右
		bool isMoveL = false;		// 移動 - 左
		bool isRotR = false;		// 移動 - 右回転
		bool isRotL = false;		// 移動 - 左回転
		bool isLift = false;		// 持ち上げる
		bool isThrow = false;		// 投げる
		bool isPunchA = false;		// パンチ - A
		bool isFlight = false;		// 飛行

		bool CheckMove()const {
			return isMoveF || isMoveB || isMoveR || isMoveL;
		};
		bool CheckRot()const {
			return isRotL || isRotR;
		}
	};

	//
	// ビークルごとに入力情報を別々に作っていく
	//
	

	/// <summary>
	/// 入力情報の格納用共用体
	/// </summary>
	using VehicleInputResult = std::variant<
		VehicleInputResult_Robot
	>;
};