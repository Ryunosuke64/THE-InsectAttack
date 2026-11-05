#pragma once
#include "IComponent.h"
#include "CollisionInfo.h"



// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:ImpactDamage Class --- */
//
//  ★継承：IComponent ★
//
// 【?】主にドルガによって投げ飛ばされる対象につける
//		投げ飛ばされた後の衝突時の処理など
//		投げ飛ばされた対象と、その対象に当たったオブジェクトにもダメージを与える	
// 
// ***************************************************************************************
class ImpactDamage : public IComponent
{
private:
	std::string m_ImpactEffectTag;		// 衝突時のエフェクト
	VECTOR3::VEC3 m_ImpactEffectScale;	// 衝突時のエフェクトの大きさ
	float m_Damage;						// ダメージ
	float m_DamageActiveDuration;		// ダメージを与えることが可能な時間
	float m_ElapsedTime;				// 経過時間
	bool m_IsImapctEnabled;				// 衝撃ダメージの使用状態か

public:
	ImpactDamage(std::weak_ptr<GameObject> pOwner, int updateRank);
	~ImpactDamage();

	void Start(RendererEngine& renderer) override;	// 初期化
	void Update(RendererEngine& renderer) override; // 更新
	void OnCollisionEnter(const PhysicsData::CollisionInfo& _other)override;	// 衝突時処理

	void set_IsImpactEnabled(bool flag) { m_IsImapctEnabled = flag; };
};

