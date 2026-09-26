#pragma once
#include "IComponent.h"
#include "RootSceneState.h"
#include "StateMachine.h"
#include "ConstantPlayerData.h"





/// プレイヤーのアニメーション
/// 末尾にRMとついているものはアニメーション自体に移動成分が入っているもの
/// 
/// [注意] モデルを変えたのでもう使わない ***********************************
/*
enum class PLAYER_ANIMATION_ID
{
	NONE = -1,

	T_POSE = 0,
	CROUCH_FWD_LOOP,
	CROUCH_IDLE_LOOP,
	DANCE_LOOP,
	DEATH01,
	DRIVING_LOOP,
	FIXING_KNEELING,
	HIT_CHEST,
	HIT_HEAD,
	IDLE_LOOP,
	IDLE_TALKING_LOOP,
	IDLE_TORCH_LOOP,
	INTERACT,
	JOG_FWD_LOOP,
	JUMP_LAND,
	JUMP_LOOP,
	JUMP_START,
	PICKUP_TABLE,
	PISTOL_AIM_DOWN,
	PISTOL_AIM_NEUTRAL,
	PISTOL_AIM_UP,
	PISTOL_IDLE_LOOP,
	PISTOL_RELOAD,
	PISTOL_SHOOT,
	PANCH_CROSS,
	PUNCH_JAB,
	PUSH_LOOP,
	ROLL,
	ROLL_RM,
	SITTING_ENTER,
	SITTING_EXIT,
	SITTING_IDLE_LOOP,
	SITTING_TALKING_LOOP,
	SPELL_SIMPLE_ENTER,
	SPELL_SIMPLE_EXIT,
	SPELL_SIMPLE_IDLE_LOOP,
	SPRINT_LOOP,
	SWIM_FWD_LOOP,
	SWIM_IDLE_LOOP,
	SWORD_ATTACK,
	SWORD_ATTACK_RM,
	SWORD_IDLE,
	WALK_FORMAL_LOOP,
	WALK_LOOP,


	NUM,
};

const std::string g_PlayerAnimationNames[] = 
{
	"NONE",
	
	"T_POSE",
	"CROUCH_FWD_LOOP",
	"CROUCH_IDLE_LOOP",
	"DANCE_LOOP",
	"DEATH01",
	"DRIVING_LOOP",
	"FIXING_KNEELING",
	"HIT_CHEST",
	"HIT_HEAD",
	"IDLE_LOOP",
	"IDLE_TALKING_LOOP",
	"IDLE_TORCH_LOOP",
	"INTERACT",
	"JOG_FWD_LOOP",
	"JUMP_LAND",
	"JUMP_LOOP",
	"JUMP_START",
	"PICKUP_TABLE",
	"PISTOL_AIM_DOWN",
	"PISTOL_AIM_NEUTRAL",
	"PISTOL_AIM_UP",
	"PISTOL_IDLE_LOOP",
	"PISTOL_RELOAD",
	"PISTOL_SHOOT",
	"PANCH_CROSS",
	"PUNCH_JAB",
	"PUSH_LOOP",
	"ROLL",
	"ROLL_RM",
	"SITTING_ENTER",
	"SITTING_EXIT",
	"SITTING_IDLE_LOOP",
	"SITTING_TALKING_LOOP",
	"SPELL_SIMPLE_ENTER",
	"SPELL_SIMPLE_EXIT",
	"SPELL_SIMPLE_IDLE_LOOP",
	"SPRINT_LOOP",
	"SWIM_FWD_LOOP",
	"SWIM_IDLE_LOOP",
	"SWORD_ATTACK",
	"SWORD_ATTACK_RM",
	"SWORD_IDLE",
	"WALK_FORMAL_LOOP",
	"WALK_LOOP",
};
*/

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:PlayerController Class --- */
//
//  ★継承：IComponent ★
//
// 【?】プレイヤー操作を行う
//		
// ***************************************************************************************
class PlayerController : public IComponent
{
private:
	std::weak_ptr<class Camera3D> m_pCameraComp;
	std::weak_ptr<class MyTransform> m_pMyTransformComp;
	std::weak_ptr<class SkinnedMeshAnimator> m_pAnimatorComp;		// アニメータコンポーネント
	std::weak_ptr<class Health> m_pHealthComp;						// 体力管理コンポーネント
	std::weak_ptr<class WeaponController> m_pWeaponController;		// 武器制御用
	std::weak_ptr<class Physics> m_pPhysicsComp;					// 物理コンポーネント

    bool m_IsAnim;					// アニメーション中かどうか
    float m_MoveSpeed;				// 移動速度
	bool m_IsJump;					// ジャンプしたか
	VECTOR3::VEC3 m_MoveVelocity;	// 移動
	float m_JumpVelocity;			// ジャンプベクトル
	bool m_IsGrounded;				// 地面にいるか
	PlayerData::PLAYER_RANGER_ANIM_ID m_CrntAnimID;	// 現在のアニメーションID
	bool m_IsRolling;

	// リジッドボディコンポーネントを作って移す
	float m_JumpForce;	// ジャンプ力
	float m_Gravity;	// 重力

	bool m_IsDead;
	bool m_IsContinuousAngle ;	// マウスに合わせて継続的に回転させるか

	float m_RollingDuration;			// ローリングの持続時間	
	float m_RollingElapsedTime;			// ローリング経過時間

public:
	PlayerController(std::weak_ptr<GameObject> pOwner, int updateRank = 100);
	~PlayerController();

	void Start(RendererEngine& renderer) override;		// 初期化
	void Update(RendererEngine& renderer) override;	// 更新処理
	void LateUpdate(RendererEngine& renderer) override;	// 更新処理

	void RollingUpdate();	// ローリング更新

	void OnCollisionEnter(const PhysicsData::CollisionInfo &other)override;

	void Reset();	// パラメータ等をリセットする（ゲームの終了時などに呼ぶ）

	/* 移動速度 */
    float get_MoveSpeed() const { return m_MoveSpeed; }
    void set_MoveSpeed(float speed) { m_MoveSpeed = speed; }

	/* アニメーションID */
	void set_AnimID(PlayerData::PLAYER_RANGER_ANIM_ID id) { m_CrntAnimID = id; };
	PlayerData::PLAYER_RANGER_ANIM_ID get_AnimID()const { return m_CrntAnimID; };

	/* 移動速度ベクトル */
	void set_MoveVelocity(const VECTOR3::VEC3 &vec) { m_MoveVelocity = vec; }
	VECTOR3::VEC3 get_MoveVelocity()const { return m_MoveVelocity; }

	/* ジャンプフラグ */
	bool get_IsJump()const { return m_IsJump; };

	/* ジャンプ力 */
	void set_JumpFoce(float jf) { m_JumpForce = jf; }
	float get_JumpFoce()const { return m_JumpForce; }

	/* 重力 */
	void set_Gravity(float gv) { m_Gravity = gv; }
	float get_Gravity()const { return m_Gravity; }

	/* 視点に合わせて継続的に回転させるか */
	void set_IsContinuousAngle(bool _flag) { m_IsContinuousAngle = _flag; };
	bool get_IsContinuousAngle()const { return m_IsContinuousAngle; };

	/* 接地中か */
	bool get_IsGrounded()const { return m_IsGrounded; };

private:
	/// <summary>
	/// アニメーションの変更
	/// </summary>
	/// <param name="id"></param>
	void ChangeAnimation(PlayerData::PLAYER_RANGER_ANIM_ID id, float _blendTime = 0.1f);


	/// <summary>
	/// マウスに合わせて継続的に回転する
	/// </summary>
	void ContinuousAngle(const VECTOR3::VEC3 &_crntRot);

	/// <summary>
	/// 移動方向に回転する
	/// </summary>
	void MovedAngle(const VECTOR3::VEC3& _crntRot,const VECTOR3::VEC3& _velocity);
};

