#pragma once
#include "Component_Collider.h"


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:BoneCollider Class --- */
//
//  ★ 継承 ★
//
// 【?】ボーン用コライダー
//
// ***************************************************************************************
class BoneCollider :  public Collider
{
private:


public:
	BoneCollider(std::weak_ptr<GameObject> pOwner, int updateRank = 100);
	~BoneCollider();
	
	void Start(RendererEngine& renderer) override;		// 初期化
	void Update(RendererEngine& renderer) override;		// 更新処理
	
	PhysicsData::PhysicsShapeDesc GetShapeDesc() const override;
};

