#pragma once
#pragma once
#include "IState.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//						ドローン タイプAステートのヘッダクラス定義をまとめたヘッダ
// 
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// PT:巡回中
// AT:戦闘態勢


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:Octahedron_AT_IdleState Class --- */
//
//  ★継承：IState ★
//
// 【?】[アクティブ]
//		待機時ステート
//		
// ***************************************************************************************
class DroneTypeA_AT_IdleState : public IState<class EnemyController>
{
private:
	const float SEARCH_RANGE = 40.0f;				// 索敵範囲
	const float SEARCH_FIELD_OF_VIEW_DEG = 70.0f;	// 視界
	const float IDLE_MIN_TIME = 0.5f;				// 待機の最小時間
	const float IDLE_MAX_TIME = 5.0f;				// 待機の最大時間
	float m_IdleDuration = 0.0f;	// 待機状態の時間

public:
	void OnEnter(class EnemyController* pOwner) override;
	void OnExit(class EnemyController* pOwner)override;
	int Update(class EnemyController* pOwner)override;
};


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:Octahedron_AT_MoveState Class --- */
//
//  ★継承：IState ★
//
// 【?】[アクティブ]
//		移動ステート
//		
// ***************************************************************************************
class DroneTypeA_AT_MoveState : public IState<class EnemyController>
{
private:
	const float MOVE_TIME_MIN = 1.0f;				// 移動の最小時間
	const float MOVE_TIME_MAX = 5.0f;				// 移動の最大時間
	const float MOVE_RANGE = 600.0f;				// 移動範囲
	const float DIR_RAND_MAX = Tool::G_PI_4_F;		// 移動の方向 ランダム最大 
	const float DIR_RAND_MIN = -Tool::G_PI_4_F;		// 移動の方向 ランダム最小 
	const float MOVE_SPEED = 60.0f;					// 移動速度

	float m_MoveDuration = 0.0f;	// 移動時間
	VECTOR3::VEC3 m_MoveDir;		// 移動方向

public:
	void OnEnter(class EnemyController* pOwner) override;
	void OnExit(class EnemyController* pOwner)override;
	int Update(class EnemyController* pOwner)override;
};


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:Octahedron_AT_TrackingState Class --- */
//
//  ★継承：IState ★
//
// 【?】[アクティブ]
//		追跡ステート
//		
// ***************************************************************************************
class DroneTypeA_AT_TrackingState : public IState<class EnemyController>
{
private:
	const float TRACKING_TIME_MAX = 10.0f;			// 追跡の最大時間
	const float TRACKING_TIME_MIN = 3.0f;			// 追跡の最小時間
	const float ATTACK_POSSIBLE_RANGE_MAX = 300.0f;	// 攻撃可能最大距離
	const float ATTACK_POSSIBLE_RANGE_MIN = 5.0f;	// 攻撃可能最小距離
	const float MOVE_SPEED = 60.0f;					// 移動速度

	float m_AttackPossibleRange = 0.0f; // 攻撃可能距離
	float m_TrackingDuration = 0.0f;	// 追従時間

public:
	void OnEnter(class EnemyController* pOwner) override;
	void OnExit(class EnemyController* pOwner)override;
	int Update(class EnemyController* pOwner)override;
};


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:Octahedron_AT_DeadState Class --- */
//
//  ★継承：IState ★
//
// 【?】[アクティブ]
//		死亡ステート
//		
// ***************************************************************************************
class DroneTypeA_AT_DeadState : public IState<class EnemyController>
{
private:
	const float DELETE_TIME = 8.0f;						// 死亡までの時間
	const float SOUND_DEAD_RADIUS = 400.0f;				// 死亡音が聞こえる範囲
	const int DROP_ITEM_MIN = 0;						// 落とすアイテムの最小数
	const int DROP_ITEM_MAX = 1;						// 落とすアイテムの最大数
	int m_FrameCounter = 0.0f;	// フレームカウンター
public:
	void OnEnter(class EnemyController* pOwner) override;
	void OnExit(class EnemyController* pOwner)override;
	int Update(class EnemyController* pOwner)override;

private:
	void SpawnDeadEffect(class EnemyController* pOwner, float _effectSize);
	void SpawnDeadFragmentEffect(class EnemyController* pOwner, float _effectSize);
};


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:Octahedron_AT_AttackLaser01State Class --- */
//
//  ★継承：IState ★
//
// 【?】[アクティブ]
//		レーザー攻撃ステート
//		一定時間、対象に照射する。
//		
// ***************************************************************************************
class DroneTypeA_AT_AttackState : public IState<class EnemyController>
{
private:
	const float PREATTACK_STUN_TIME_MAX = 3.0f;		// 攻撃前の硬直最大時間
	const float PREATTACK_STUN_TIME_MIN = 2.5f;		// 攻撃前の硬直最小時間 
	const float SOUND_RADIUS = 1000.0f;				// 発射音が聞こえる範囲
	const float MOVE_SPEED = 0.0f;					// 移動速度

	float m_PreAttackStunDuration = 0.0f;	// 攻撃前の硬直時間
	float m_AttackTime = 0.0f;
	int m_AttckCount = 0;
public:
	void OnEnter(class EnemyController* pOwner) override;
	void OnExit(class EnemyController* pOwner)override;
	int Update(class EnemyController* pOwner)override;
};

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:DroneTypeA_AT_HitStunState Class --- */
//
//  ★継承：IState ★
//
// 【?】[アクティブ]
//		被弾時
//		
// ***************************************************************************************
class DroneTypeA_AT_HitStunState : public IState<class EnemyController>
{
private:
	const float SOUND_HIT_RADIUS = 300.0f;				// 被弾音が聞こえる範囲
	const float STUN_DURATION = 1.0f;
	float m_StunTimer = 0.0f;

public:
	void OnEnter(class EnemyController* pOwner) override;
	void OnExit(class EnemyController* pOwner)override;
	int Update(class EnemyController* pOwner)override;

private:
	void SpawnHitEffect(const std::string& effectTag, const VECTOR3::VEC3& pos, const VECTOR3::VEC3& rot, const VECTOR3::VEC3& scale);
};

/// <summary>
/// 共通処理
/// </summary>
class DroneTypeA_CommonStateProcess {
public:
	/// <summary>
	/// 共通処理
	/// </summary>
	/// <param name="pOwner">ステートの親</param>
	/// <returns>ステートID（-1の場合は変更なし）</returns>
	static int CommonProcess(class EnemyController* pOwner);


	//static void RotateToPlayer(class EnemyController* pOwner);
	//static void ApplyGravity(class EnemyController* pOwner);
};

