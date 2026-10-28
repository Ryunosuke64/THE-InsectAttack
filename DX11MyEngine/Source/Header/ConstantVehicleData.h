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
		DURGA_V_STATE_THROW,	// 投げる
		DURGA_V_STATE_DOWN,		// 倒れる
		DURGA_V_STATE_GETUP,	// 立ち上がる
		DURGA_V_STATE_DEAD,		// 死
	};

	/// <summary>
	/// アニメーションID - ドルガ -
	/// </summary>
	enum class DURGA_ANIM_ID
	{
		DOWN,			// 倒れる
		GETUP,			// 起き上がる
		LIFT,			// 持ち上げる
		PUNCH_A,		// パンチ - A
		THROW,			// 投げる
		TPOSE,			// Tポーズ
		WALK_F_ENTER,	// 歩き始め
		WALK_F_L,		// 左足前進
		WALK_F_R,		// 右足前進
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