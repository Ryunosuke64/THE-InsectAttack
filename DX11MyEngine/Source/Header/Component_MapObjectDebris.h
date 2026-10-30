#pragma once
#include "IComponent.h"

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:MapObjectDebris Class --- */
//
//  ★継承：IComponent ★
//
// 【?】マップオブジェクトの破片の制御
//		
// ***************************************************************************************
class MapObjectDebris : public IComponent
{
private:
	enum class STATE
	{
		ALIVE,			// 生存中
		UNDERWORLD,	// 裏世界へ
	};

	float m_ElapsedTime;		// 経過時間
	float m_LifeTime;		    // 生存時間
	bool m_IsPlaySmokeEffect;	// 煙エフェクトを再生したか
	std::weak_ptr<class RigidBody> m_pRigidBody;
	std::weak_ptr<class MeshCollider> m_pCollider;
	STATE m_CrntState;
public:
	MapObjectDebris(std::weak_ptr<GameObject> pOwner, int updateRank);
	~MapObjectDebris();

	void Start(RendererEngine& renderer) override;	// 初期化
	void Update(RendererEngine& renderer) override;	// 更新
	void OnCollisionEnter(const PhysicsData::CollisionInfo& _other)override;

};

