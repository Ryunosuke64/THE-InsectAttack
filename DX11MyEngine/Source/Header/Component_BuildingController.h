#pragma once
#include "IComponent.h"

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:BuildingController Class --- */
//
//  ★継承：IComponent ★
//
// 【?】建物の挙動を管理
//		実際の挙動はステートマシン内で行っている
//		それぞれのコンポーネントと値を行き来させる箱みたいな感じ
//		
// ***************************************************************************************
class BuildingController : public IComponent
{
public:

private:
	class Health* m_pHealthComp;		// 体力管理コンポーネント
	class RigidBody* m_pRigidBodyComp;	// 剛体コンポーネント
	StateMachine<BuildingController> m_StateMachine;	// ステートマシン
	float m_CollapseTargetAngle;	// 崩壊する角度

	// ステートマシンの状態を管理するためのフラグ
	bool m_IsDestruction;	// 破壊されたかどうか
	bool m_IsOnDamage;	// ダメージを受けたかどうか

public:
	BuildingController(std::weak_ptr<GameObject> pOwner, int updateRank);
	~BuildingController();

	void Start(RendererEngine& renderer) override;	// 初期化
	void Update(RendererEngine& renderer) override;	// 更新
	void OnCollisionEnter(const PhysicsData::CollisionInfo& _other)override;

	bool get_IsDestruction()const { return m_IsDestruction; }	// 破壊されたかどうかの取得
	bool get_IsOnDamage()const { return m_IsOnDamage; };			// ダメージを受けたかどうかの取得
	void set_CollapseTargetAngle(float _angle) { m_CollapseTargetAngle = _angle; }
	float get_CollapseTargetAngle()const { return m_CollapseTargetAngle; }	// 崩壊する角度の取得
	const class Health* get_HealthComp()const { return m_pHealthComp; }	// 体力管理コンポーネントの取得
	class RigidBody* get_RigidBodyComp()const { return m_pRigidBodyComp; }	// 剛体コンポーネントの取得
};

