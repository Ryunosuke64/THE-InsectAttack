#pragma once
#include "Component_Render.h"

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:WaterRenderer Class --- */
//
//  šŒp³FIComponent š
//
// y?z…–Ê‚Ì•`‰æ‚ğs‚¤
// 
// ***************************************************************************************
class WaterRenderer : public Render
{
private:
	std::weak_ptr<class IMeshResource> m_pMeshResource;	// ƒƒbƒVƒ…î•ñ‚ÌQÆ

public:
	WaterRenderer(std::weak_ptr<GameObject> pOwner, int updateRank = 100);
	~WaterRenderer();

	void Start(RendererEngine& renderer) override;		// ‰Šú‰»
	void Update(RendererEngine& renderer) override;		// XVˆ—
	void Draw(RendererEngine& renderer) override;		// •`‰æˆ—
	bool IsVisible(const DirectX::BoundingFrustum& _frustum) const override;

	void set_MeshResource(std::weak_ptr<class IMeshResource> meshResource);
};

