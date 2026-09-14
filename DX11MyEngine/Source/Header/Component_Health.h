#pragma once
#include "IComponent.h"
#include "CollisionInfo.h"



// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:Health Class --- */
//
//  ★継承：IComponent ★
//
// 【?】HPを管理する
//		
// ***************************************************************************************
class Health : public IComponent
{
private:
	float m_CrntHP;			// 現在のHP
	float m_MaxHP;			// 最大HP
	bool m_IsDead;			// 死亡フラグ
	float m_DamageAmount;	// 受けたダメージ量(最後に受けたダメージ量が入る)
	//bool m_IsOnDamage;  // ダメージを受けたか

	std::vector<std::function<void(float)>> m_DamageTasks;	// ダメージを受けた際の処理
	std::vector<std::function<void()>> m_DeathTasks;		// 死んだときの処理

	CollisionInfo m_CollisionInfo;	// 衝突情報

public:
	Health(std::weak_ptr<GameObject> pOwner, int updateRank);
	~Health();

	void Start(RendererEngine& renderer) override;	// 初期化
	void Update(RendererEngine& renderer) override;// 更新
	void TakeDamage(const float _dmg);	// ダメージ処理
	void TakeDamage(const float _dmg, const CollisionInfo& _collInfo);	// ダメージ処理

	void RegisterOnDead(std::function<void()> _callback);	// 死んだときの処理の登録
	void RegisterOnDamage(std::function<void(float)> _callback);	// ダメージを受けた際の処理の登録

	/* HP */
	const float get_CrntHP()const { return m_CrntHP; }
	const float get_MaxHP()const { return m_MaxHP; }
	void set_MaxHP(const float _hp) { m_MaxHP = _hp; }
	void set_CrntHP(const float _hp);
	void set_RecoveryHP(const float _hp);
	const float get_DamageAmount()const { return m_DamageAmount; }


	const CollisionInfo& get_CollisionInfo()const { return m_CollisionInfo; }	// ダメージを受けた際の衝突情報の取得

	const bool get_IsDead()const { return m_IsDead; }			// 死亡フラグ取得
	//bool get_IsOnDamage()const { return m_IsOnDamage;}		// ダメージフラグを取得

	void Reset();
};

