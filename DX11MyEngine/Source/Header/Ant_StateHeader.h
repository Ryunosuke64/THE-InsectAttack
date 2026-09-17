#pragma once
#include "IState.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//						アリのステートのヘッダクラス定義をまとめたヘッダ
// 
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// PT:巡回中
// AT:戦闘態勢

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:Ant_PT_IdleState Class --- */
//
//  ★継承：IState ★
//
// 【?】[非アクティブ]
//		待機時ステート
//		
// ***************************************************************************************
class Ant_PT_IdleState : public IState<class EnemyController>
{
private:
	const float SEARCH_RANGE = 40.0f;				// 索敵範囲
	const float SEARCH_FIELD_OF_VIEW_DEG = 70.0f;	// 視界
	const float IDLE_MIN_TIME = 0.5f;				// 待機の最小時間
	const float IDLE_MAX_TIME = 7.5f;				// 待機の最大時間
	float m_IdleDuration = 0.0f;	// 待機状態の時間

public:
	void OnEnter(class EnemyController* pOwner) override;
	void OnExit(class EnemyController* pOwner)override;
	int Update(class EnemyController* pOwner)override;

};

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:Ant_PT_MoveState Class --- */
//
//  ★継承：IState ★
//
// 【?】[非アクティブ]
//		移動ステート
//		
// ***************************************************************************************
class Ant_PT_MoveState : public IState<class EnemyController>
{
private:
	const float SEARCH_RANGE = 40.0f;				// 索敵範囲
	const float SEARCH_FIELD_OF_VIEW_DEG = 70.0f;	// 視界
	const float MOVE_TIME_MIN = 0.5f;				// 移動の最小時間
	const float MOVE_TIME_MAX = 7.5f;				// 移動の最大時間
	const float MOVE_RANGE = 30.0f;					// 移動範囲
	const float DIR_RAND_MAX = Tool::G_PI_4_F;		// 移動の方向 ランダム最大 
	const float DIR_RAND_MIN = -Tool::G_PI_4_F;		// 移動の方向 ランダム最小 
	const float MOVE_SPEED = 15.0f;
	const float ANIM_SPEED = 1.25f;					// アニメーション速度

	float m_MoveDuration = 0.0f;	// 移動時間
	VECTOR3::VEC3 m_MoveDir;		// 移動方向
	bool m_IsDirChange = false;		// 方向転換したか（生物感を出すために、一回方向転換させる）

public:
	void OnEnter(class EnemyController* pOwner) override;
	void OnExit(class EnemyController* pOwner)override;
	int Update(class EnemyController* pOwner)override;

};


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:Ant_AT_TrackingState Class --- */
//
//  ★継承：IState ★
//
// 【?】[アクティブ]
//		追従ステート
//		
// ***************************************************************************************
class Ant_AT_TrackingState : public IState<class EnemyController>
{
private:

	const float TRACKING_TIME_MAX = 7.0f;	// 追跡の最大時間
	const float TRACKING_TIME_MIN = 1.0f;	// 追跡の最小時間
	const float ATTACK_POSSIBLE_RANGE_MAX = 60.0f;	// 攻撃可能最大距離
	const float ATTACK_POSSIBLE_RANGE_MIN = 5.0f;	// 攻撃可能最小距離
	const float MOVE_SPEED = 12.0f;
	const float ANIM_SPEED = 1.6f;			// アニメーション速度

	float m_AttackPossibleRange;// 攻撃可能距離
	float m_TrackingDuration;	// 追従時間

public:
	void OnEnter(class EnemyController *pOwner) override;
	void OnExit(class EnemyController *pOwner)override;
	int Update(class EnemyController *pOwner)override;
};


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:Ant_AT_MoveState Class --- */
//
//  ★継承：IState ★
//
// 【?】[アクティブ]
//		移動ステート
//		
// ***************************************************************************************
class Ant_AT_MoveState : public IState<class EnemyController>
{
private:
	const float SEARCH_RANGE = 40.0f;				// 索敵範囲
	const float SEARCH_FIELD_OF_VIEW_DEG = 70.0f;	// 視界
	const float MOVE_TIME_MIN = 0.5f;				// 移動の最小時間
	const float MOVE_TIME_MAX = 7.5f;				// 移動の最大時間
	const float MOVE_RANGE = 50.0f;					// 移動範囲
	const float DIR_RAND_MAX = Tool::G_PI_4_F;		// 移動の方向 ランダム最大 
	const float DIR_RAND_MIN = -Tool::G_PI_4_F;		// 移動の方向 ランダム最小 
	const float MOVE_SPEED = 15.0f;
	const float ANIM_SPEED = 1.5f;					// アニメーション速度

	float m_MoveDuration = 0.0f;	// 移動時間
	VECTOR3::VEC3 m_MoveDir;		// 移動方向
	bool m_IsDirChange;				// 方向転換したか

public:
	void OnEnter(class EnemyController *pOwner) override;
	void OnExit(class EnemyController *pOwner)override;
	int Update(class EnemyController *pOwner)override;
};


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:Ant_AT_AttackAcidState Class --- */
//
//  ★継承：IState ★
//
// 【?】[アクティブ]
//		酸攻撃ステート
//		
// ***************************************************************************************
class Ant_AT_AttackAcidState : public IState<class EnemyController>
{
private:
	const float PREATTACK_STUN_TIME_MAX = 2.0f;		// 攻撃前の硬直最大時間
	const float PREATTACK_STUN_TIME_MIN = 0.5f;		// 攻撃前の硬直最小時間 
	const float SOUND_RADIUS = 600.0f;				// 発射音が聞こえる範囲

	float m_PreAttackStunDuration = 0.0f;	// 攻撃前の硬直時間

public:
	void OnEnter(class EnemyController *pOwner) override;
	void OnExit(class EnemyController *pOwner)override;
	int Update(class EnemyController *pOwner)override;
};

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:Ant_AT_HitStunState Class --- */
//
//  ★継承：IState ★
//
// 【?】[アクティブ]
//		被弾時
//		
// ***************************************************************************************
class Ant_AT_HitStunState : public IState<class EnemyController>
{
private:
	const float SOUND_HIT_RADIUS = 300.0f;				// 被弾音が聞こえる範囲
	const float STUN_DURATION = 1.0f;
	float m_StunTimer = 0.0f;

public:
	void OnEnter(class EnemyController *pOwner) override;
	void OnExit(class EnemyController *pOwner)override;
	int Update(class EnemyController *pOwner)override;

private:
	void SpawnHitEffect(const std::string &effectTag, const VECTOR3::VEC3 &pos, const VECTOR3::VEC3 &rot, const VECTOR3::VEC3& scale);
	void SpawnHitDecal(const std::string &effectTag, const VECTOR3::VEC3 &pos, const VECTOR3::VEC3 &rot, const VECTOR3::VEC3& scale);

};


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:Ant_AT_AttackAcidState Class --- */
//
//  ★継承：IState ★
//
// 【?】[アクティブ]
//		死亡ステート
//		
// ***************************************************************************************
class Ant_AT_DeadState : public IState<class EnemyController>
{
private:
	const float OVERTURN_TIME = 0.5f;		// ひっくり返るまでの時間
	const float DELETE_TIME = 10.0f;		// 死亡までの時間
	const float SOUND_DEAD_RADIUS = 600.0f;				// 死亡音が聞こえる範囲
	const int DROP_ITEM_MIN = 0;						// 落とすアイテムの最小数
	const int DROP_ITEM_MAX = 1;						// 落とすアイテムの最大数
	int m_FrameCounter = 0;

	DirectX::XMVECTOR m_TargetRotQ;	// ひっくり返った後のクオータニオン
	DirectX::XMVECTOR m_StartRotQ;	// ひっくり返った後のクオータニオン

public:
	void OnEnter(class EnemyController *pOwner) override;
	void OnExit(class EnemyController *pOwner)override;
	int Update(class EnemyController *pOwner)override;

private:
	void SpawnDeadEffect(class EnemyController* pOwner);

};

/// <summary>
/// 共通処理
/// </summary>
class Ant_CommonStateProcess {
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