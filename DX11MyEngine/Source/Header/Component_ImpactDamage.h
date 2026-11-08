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
public:
	// 衝撃ダメージ用の情報
	struct ImpactDamageParam
	{
		std::string impactEffectTag;		// 衝突時のエフェクト
		std::string impactSoundTag;			// 衝突時の音
		VECTOR3::VEC3 impactEffectScale		// 衝突時のエフェクトの大きさ
			= VECTOR3::VEC3();	
		float damageValue = 0.0f;			// ダメージ
		float damageActiveDuration = 0.0f;	// ダメージを与えることが可能な時間
	};

private:
	ImpactDamageParam m_Param;
	float m_ElapsedTime;				// 経過時間
	bool m_IsImapctEnabled;				// 衝撃ダメージの使用状態か


public:
	ImpactDamage(std::weak_ptr<GameObject> pOwner, int updateRank);
	~ImpactDamage();

	void Start(RendererEngine& renderer) override;	// 初期化
	void Update(RendererEngine& renderer) override; // 更新
	void OnCollisionEnter(const PhysicsData::CollisionInfo& _other)override;	// 衝突時処理
	bool get_IsImpactEnabled()const { return m_IsImapctEnabled; }
	void set_ImpactDamageParam(const ImpactDamageParam& _param) { m_Param = _param; };
	void set_ImpactEnabled(bool _flag) { m_IsImapctEnabled = _flag; };
};

