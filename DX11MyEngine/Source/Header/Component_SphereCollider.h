#pragma once
#include "Component_Collider.h"

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:SphereCollider Class --- */
//
//  ★ 継承 ★
//
// 【?】球判定コライダー
//
// ***************************************************************************************
class SphereCollider : public Collider
{
private:
	float m_Radius;	// 半径
	std::unique_ptr<class DebugMesh> m_pSphereMesh;	// デバッグ用メッシュ

public:
	SphereCollider(std::weak_ptr<GameObject> pOwner, int updateRank = 100);
	~SphereCollider();

	void Start(RendererEngine &renderer) override;		// 初期化
	void Update(RendererEngine &renderer) override;		// 更新処理
	void Draw(RendererEngine &renderer)override;		// 描画処理

	PhysicsData::PhysicsShapeDesc GetShapeDesc() const override;


	// 半径
	void set_Radius(float _r) { m_Radius = _r; }
	float get_Radius()const { return m_Radius; }
};

