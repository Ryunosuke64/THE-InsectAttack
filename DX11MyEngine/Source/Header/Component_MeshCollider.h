#pragma once
#include "Component_Collider.h"

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:BoxCollider Class --- */
//
//  ★ 継承 ★
//
// 【?】メッシュ判定コライダー
//
// ***************************************************************************************
class MeshCollider : public Collider
{
private:

public:
	MeshCollider(std::weak_ptr<GameObject> pOwner, int updateRank = 100);
	~MeshCollider();

	void Start(RendererEngine& renderer) override;		// 初期化
	void Update(RendererEngine& renderer) override;		// 更新処理
};

