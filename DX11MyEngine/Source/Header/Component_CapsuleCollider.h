#pragma once
#include "Component_Collider.h"

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:CapsuleCollider Class --- */
//
//  ★ 継承 ★
//
// 【?】カプセル判定コライダー
//
// ***************************************************************************************
class CapsuleCollider : public Collider
{
private:
	float m_Height;		// 高さ
	float m_Radius;		// 半径
	PhysicsData::CAPSULE_AXIS m_CapsuleAxis;	// カプセルの軸方向

public:
	CapsuleCollider(std::weak_ptr<GameObject> pOwner, int updateRank = 100);
	~CapsuleCollider();

	void Start(RendererEngine& renderer) override;		// 初期化
	PhysicsData::PhysicsShapeDesc GetShapeDesc() const override;


	void set_CapsuleAxis(PhysicsData::CAPSULE_AXIS _axis) { m_CapsuleAxis = _axis; }
	PhysicsData::CAPSULE_AXIS get_CapsuleAxis()const { return m_CapsuleAxis; }

	void set_Height(float _h) { m_Height = _h; }
	void set_Radius(float _r) { m_Radius = _r; }

	float get_Height()const { return m_Height; }
	float get_Radius()const { return m_Radius; }
};

