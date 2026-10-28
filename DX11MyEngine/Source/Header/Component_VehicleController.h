#pragma once
#include "IComponent.h"
#include "ConstantVehicleData.h"

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:VehicleController Class --- */
//
//  ★継承：IComponent ★
//
// 【?】プレイヤーが操作するビークルに付けるコンポーネント
//		※ AI用は別に作る
//		
// ***************************************************************************************
class VehicleController : public IComponent
{
public:
	/// <summary>
	/// ビークルのセットアップ情報
	/// </summary>
	struct VehicleSetupInfo
	{

	};

private:
	StateMachine<VehicleController>m_StateMachine;	// ステートマシン
	VehicleData::V_CONTROLL_MODE m_ControllMode;	// 操作方法
	VehicleData::V_INPUT_TYPE m_InputType;			// 操作するための入力タイプ
	float m_StateTimer;								// ステート時間
	float m_AnimSpeed;								// アニメーション速度
	bool m_IsPlayAnim;								// アニメーションさせるかどうか
	float m_MoveSpeed;								// 移動速度
	int m_WeaponID;									// 使用する武器ID
	VehicleData::VehicleInputResult m_InputResult;	// 入力情報の格納

	class Health* m_pHealthComp;					// 体力管理コンポーネント
	class SkinnedMeshAnimator* m_pAnimatorComp;		// アニメータコンポーネント
	class MoveLogic* m_pMoveLogicComp;				// 移動コンポーネント
	class RigidBody* m_pRigidBodyComp;				// 物理コンポーネント
	class MyTransform* m_pTransformComp;			// トランスフォームコンポーネント
	const GameObject* m_pTarget;					// 攻撃目標
	class AI_GunWeapon* m_pAIGunWeapon;				// 銃火器コンポーネント
public:
	VehicleController(std::weak_ptr<GameObject> pOwner, int updateRank);
	~VehicleController();

	void Start(RendererEngine& renderer) override;	// 初期化
	void LateUpdate(RendererEngine& renderer) override;	// 更新


	bool Setup(const VehicleSetupInfo& _info);	// セットアップ

	/// <summary> ステートマシンの登録 </summary>
	void RegisterStateMachine(const StateMachine<VehicleController>& _state) { m_StateMachine = _state; };

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

	/* 入力処理 */
	const VehicleData::VehicleInputResult CheckInputRobot();

	/* 入力結果 */
	const VehicleData::VehicleInputResult_Robot& get_InputResult_Robot()const ;


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