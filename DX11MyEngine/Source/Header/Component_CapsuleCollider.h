#pragma once
#include "Component_Collider.h"

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:CapsuleCollider Class --- */
//
//  š Œp³ š
//
// y?zƒJƒvƒZƒ‹”»’èƒRƒ‰ƒCƒ_[
//
// ***************************************************************************************
class CapsuleCollider : public Collider
{
private:
	float m_Height;		// ‚‚³
	float m_Radius;		// ”¼Œa

public:
	CapsuleCollider(std::weak_ptr<GameObject> pOwner, int updateRank = 100);
	~CapsuleCollider();

	void Start(RendererEngine& renderer) override;		// ‰Šú‰»
	PhysicsData::PhysicsShapeDesc GetShapeDesc() const override;



	void set_Height(float _h) { m_Height = _h; }
	void set_Radius(float _r) { m_Radius = _r; }

	float get_Height()const { return m_Height; }
	float get_Radius()const { return m_Radius; }
};

