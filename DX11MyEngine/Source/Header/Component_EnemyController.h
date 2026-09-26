#pragma once
#include "IComponent.h"
#include "ConstantUtilityData.h"

// ひるみデータ
struct StaggerInfo
{
	float _threshold = 0.0f;		// ひるみ耐久値
	float _cumulativeValue = 0.0f;	// 蓄積値

	//
	// ダメージの蓄積量が耐久を上回ったか
	//
	const bool IsStagger()
	{
		if (_cumulativeValue >= _threshold)
		{
			_cumulativeValue = 0.0f;	// 蓄積リセット
			return true;
		}

		return false;
	};
};

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:EnemyController Class --- */
//
//  ★継承：IComponent ★
//
// 【?】敵の挙動を管理
//		実際の挙動はステートマシン内で行っている
//		それぞれのコンポーネントと値を行き来させる箱みたいな感じ
//		
// TODO:汎用性のない、ちぐはぐな設計になってしまっているので今後直す。
//		エネミーに必要な共通処理だけ持たせて、外から、ステートを設定するだけで、使えるようにするのがベスト
//		武器と同じように、jsonでパラメータを設定できるようにする
//		
// ***************************************************************************************
class EnemyController :  public IComponent
{
private:
	StateMachine<EnemyController> m_StateMachine;	// ステートマシン

	class Health* m_pHealthComp;					// 体力管理コンポーネント
	class SkinnedMeshAnimator* m_pAnimatorComp;		// アニメータコンポーネント
	class Collider* m_pColliderComp;				// コライダーコンポーネント
	class MoveLogic* m_pMoveLogicComp;				// 移動コンポーネント
	class Physics* m_pPhysicsComp;					// 物理コンポーネント
	class MyTransform* m_pTransformComp;			// トランスフォームコンポーネント
	const GameObject* m_pTarget;					// 攻撃目標

	StaggerInfo m_StaggerInfo;						// 怯み情報

	VECTOR3::VEC3 m_MoveVelocity;					// 移動
	VECTOR3::VEC3 m_StartPos;						// 開始位置
	const EnemyData::BaseEnemyData* m_pEnemyData;	// 読み取り専用データ

	int m_CrntAnimID;				// 現在のアニメーション番号
	float m_MoveSpeed;				// 移動速度
	float m_StateTimer;				// ステート時間
	float m_Gravity;				// 重力
	float m_GravityVelocity;		// 地面方向に掛かるベロシティ
	float m_AnimSpeed;				// アニメーション速度
	//bool m_IsDead;
	bool m_IsAnim;					// アニメーション中かどうか
	bool m_IsGrounded;				// 接地しているか
	bool m_IsOnDamage;				// ダメージフラグ

	EnemyData::EnemyID m_MyID;					// 自分自身のID
	EnemyData::EnemyGroupID m_MyGroupID;		// 自分が所属しているグループ（-1なら所属無し）

public:
	EnemyController(std::weak_ptr<GameObject> pOwner, int updateRank);
	~EnemyController();

	void Start(RendererEngine& renderer) override;		// 初期化
	void LateUpdate(RendererEngine& renderer) override;	// 更新
	void Update(RendererEngine& renderer) override;		// 更新
	void Draw(RendererEngine& renderer) override;		// 描画
	void OnCollisionEnter(const PhysicsData::CollisionInfo &_other)override;	// 衝突時処理
	void set_EnemyData(const EnemyData::BaseEnemyData* _enemyData) { m_pEnemyData = _enemyData; }	// エネミーデータの設定
	const EnemyData::BaseEnemyData* get_EnemyData()const;

	/// <summary> ステートマシンの登録 </summary>
	void RegisterStateMachine(const StateMachine<EnemyController>& _state) { m_StateMachine = _state; };

	/// <summary>ステートの変更 </summary>
	void ChangeState(const int _state);

	/// <summary>アニメーションの変更 </summary>
	void ChangeAnimation(const int _newId);

	/* アニメーション速度 */
	void set_AnimSpeed(float _time) { m_AnimSpeed = _time; }
	float get_AnimSpeed()const { return m_AnimSpeed; }

	/* アニメーションID */
	void set_AnimID(const int _id) { m_CrntAnimID = _id; }
	int get_AnimID()const { return m_CrntAnimID; }

	/* 移動速度 */
	float get_MoveSpeed() const { return m_MoveSpeed; }
	void set_MoveSpeed(float _spd) { m_MoveSpeed = _spd; }

	/* 移動ベクトル */
	const VECTOR3::VEC3 &get_MoveVelocity() { return m_MoveVelocity; }
	void set_MoveVelocity(const VECTOR3::VEC3 &_vel) { m_MoveVelocity = _vel; }

	/* アニメーションの再生させるかどうか */
	bool get_IsAnim() const { return m_IsAnim; }
	void set_IsAnim(bool _flag) { m_IsAnim = _flag; }

	/* 接地フラグ */
	bool get_IsGrounded() const { return m_IsGrounded; }

	/* 攻撃目標 */
	const GameObject* get_Target() const { return m_pTarget; }
	//void set_Target(std::shared_ptr<GameObject> _pObj) { m_pTarget = _pObj; }

	/* 攻撃目標のトランスフォームを取得 */
	std::shared_ptr<class MyTransform> get_TargetTransform() const;

	/* ステート時間 */
	float get_StateTimer()const { return m_StateTimer; }
	void clear_StateTimer() { m_StateTimer = 0.0f; }	

	/* 移動ロジックの切り替え */
	void set_MoveLogicState(UtilityData::MOVE_BEHAVIOUR_TYPE _moveType);

	/* 開始時の位置を取得する*/
	const VECTOR3::VEC3& get_StartPos() { return m_StartPos; }

	/* 死亡フラグ */
	bool get_IsDead()const;

	/* ダメージフラグ */
	bool get_IsOnDamage()const { return m_IsOnDamage; }
	float get_DamageAmount()const;

	/* 怯み */
	void set_StaggerThreshold(float _val) { m_StaggerInfo._threshold = _val; }	// 怯み耐久
	bool get_IsStagger() { return m_StaggerInfo.IsStagger(); }		// 怯み状態か

	/* エネミーIDの設定
	* ※グループでない場合は、**EnemyGroupID** は-1に設定してね
	*/
	void set_EnemyID(const EnemyData::EnemyID& _id, const EnemyData::EnemyGroupID _groupID)
	{
		m_MyID = _id;
		m_MyGroupID = _groupID;
	};

	/* エネミーIDの取得 */
	const EnemyData::EnemyID& get_EnemyID()const { return m_MyID; }
	const EnemyData::EnemyGroupID& get_GroupID()const { return m_MyGroupID; };


	// =========================================================================================
	//	各コンポーネントの取得
	// =========================================================================================
	class MoveLogic* get_MoveLogicComponent() const { return m_pMoveLogicComp; };
	class SkinnedMeshAnimator* get_AnimatorComponent() const { return m_pAnimatorComp; };
	class Collider* get_ColliderComponent() const { return m_pColliderComp; };
	class Physics* get_PhysicsComponent() const { return m_pPhysicsComp; };
	class Health* get_HealthComponent() const { return m_pHealthComp; }
	class MyTransform* get_TransformComponent() const { return m_pTransformComp; }
};
