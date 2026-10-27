#pragma once
#include "IComponent.h"
#include "ConstantVehicleData.h"

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:Vehicle Class --- */
//
//  ★継承：IComponent ★
//
// 【?】ビークルに付けるコンポーネント
//		
// ***************************************************************************************
class Vehicle : public IComponent
{
public:
	/// <summary>
	/// 操作方法
	/// </summary>
	enum class CONTROLL_MODE
	{
		AI,			// AI行動
		BOARDING,	// 搭乗
		REMOTE,		// リモート（鉄人形式）
	};

	/// <summary>
	/// ビークルのセットアップ情報
	/// </summary>
	struct VehicleSetupInfo
	{

	};

private:
	StateMachine<Vehicle>m_StateMachine;	// ステートマシン
	CONTROLL_MODE m_ControllMode;			// 操作方法
	float m_StateTimer;						// ステート時間
	float m_AnimSpeed;						// アニメーション速度
	bool m_IsPlayAnim;						// アニメーションさせるかどうか
	float m_MoveSpeed;						// 移動速度
	int m_WeaponID;							// 使用する武器ID

	class Health* m_pHealthComp;					// 体力管理コンポーネント
	class SkinnedMeshAnimator* m_pAnimatorComp;		// アニメータコンポーネント
	class MoveLogic* m_pMoveLogicComp;				// 移動コンポーネント
	class RigidBody* m_pRigidBodyComp;				// 物理コンポーネント
	class MyTransform* m_pTransformComp;			// トランスフォームコンポーネント
	const GameObject* m_pTarget;					// 攻撃目標
	class AI_GunWeapon* m_pAIGunWeapon;				// 銃火器コンポーネント
public:
	Vehicle(std::weak_ptr<GameObject> pOwner, int updateRank);
	~Vehicle();

	void Start(RendererEngine& renderer) override;	// 初期化
	void LateUpdate(RendererEngine& renderer) override;	// 更新


	bool Setup(const VehicleSetupInfo& _info);	// セットアップ

	/// <summary> ステートマシンの登録 </summary>
	void RegisterStateMachine(const StateMachine<Vehicle>& _state) { m_StateMachine = _state; };

	/// <summary>ステートの変更 </summary>
	void ChangeState(const int _state);


	/// <summary>アニメーションの変更 </summary>
	void ChangeAnimation(const int _newId);

	/* アニメーションの再生させるかどうか */
	bool get_IsPlayAnim() const { return m_IsPlayAnim; }
	void set_IsPlayAnim(bool _flag) { m_IsPlayAnim = _flag; }

	/* アニメーション速度 */
	void set_AnimSpeed(float _time) { m_AnimSpeed = _time; }
	float get_AnimSpeed()const { return m_AnimSpeed; }

	/* 死亡フラグ */
	bool get_IsDead()const;

	/* 移動ロジックの切り替え */
	void set_MoveLogicState(UtilityData::MOVE_BEHAVIOUR_TYPE _moveType);

	/* 移動速度 */
	float get_MoveSpeed() const { return m_MoveSpeed; }
	void set_MoveSpeed(float _spd) { m_MoveSpeed = _spd; }

	/* 武器ID */
	void set_WeaponID(int _id) { m_WeaponID = _id; }
	int get_WeaponID()const { return m_WeaponID; }
	// =========================================================================================
	//	各コンポーネントの取得
	// =========================================================================================
	class MoveLogic* get_MoveLogicComponent() const { return m_pMoveLogicComp; };
	class SkinnedMeshAnimator* get_AnimatorComponent() const { return m_pAnimatorComp; };
	class RigidBody* get_RigidBodyComponent() const { return m_pRigidBodyComp; };
	class Health* get_HealthComponent() const { return m_pHealthComp; }
	class MyTransform* get_TransformComponent() const { return m_pTransformComp; }
	class AI_GunWeapon* get_AIGunWeapon() const { return m_pAIGunWeapon; }
};