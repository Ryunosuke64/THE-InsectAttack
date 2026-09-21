#pragma once
#include "Component_Collider.h"

enum class COLLISION_JUDGMENT : unsigned char
{
	AABB,
	OBB
};


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:BoxCollider Class --- */
//
//  ★ 継承 ★
//
// 【?】箱判定コライダー
//
// ***************************************************************************************
class BoxCollider : public Collider
{
private:
	VECTOR3::VEC3 m_HarfSize;							// 大きさ
	std::unique_ptr<class DebugMesh> m_pBoxMesh;	// デバッグ用メッシュ
	COLLISION_JUDGMENT m_CollisionJudgmentType;		// 衝突判定の種類

public:
	BoxCollider(std::weak_ptr<GameObject> pOwner, int updateRank = 100);
	~BoxCollider();

	void Start(RendererEngine &renderer) override;		// 初期化
	void Update(RendererEngine &renderer) override;		// 更新処理
	void Draw(RendererEngine &renderer)override;		// 描画処理

	PhysicsData::PhysicsShapeDesc GetShapeDesc() const override;

	// 大きさ
	void set_Size(const VECTOR3::VEC3 &_vIn) { m_HarfSize = _vIn; }
	VECTOR3::VEC3 get_Size()const { return m_HarfSize; }

	//const CollInData_OBB& get_WorldOBB() const { return m_WorldOBB; }
	//const CollInData_AABB& get_BroadPhaseAABB() const { return m_BroadPhaseAABB; }

	// OBBフラグ
	COLLISION_JUDGMENT get_CollisionJudgmentType()const { return m_CollisionJudgmentType; }
	void set_CollisionJudgmentType(COLLISION_JUDGMENT _flag) { m_CollisionJudgmentType = _flag; }
};

