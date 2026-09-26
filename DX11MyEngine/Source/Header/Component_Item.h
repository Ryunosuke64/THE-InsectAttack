#pragma once
#include "IComponent.h"
#include "ConstantUtilityData.h"

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:Item Class --- */
//
//  ★継承：IComponent ★
//
// 【?】アイテム
//		回復小、回復大、アーマー、武器
//		
// ***************************************************************************************
class Item : public IComponent
{
private:
	UtilityData::ITEM_TYPE m_ItemType;
	class MyTransform *m_pTransform;
	class Physics* m_pPhysics;
	class BoxCollider* m_pBoxCollider;
	float m_Timer;


public:
	Item(std::weak_ptr<GameObject> pOwner, int updateRank = 100);
	~Item();

	void Start(RendererEngine& renderer) override;		// 初期化
	void Update(RendererEngine& renderer) override;		// 更新処理
	void OnTriggerEnter(const PhysicsData::CollisionInfo& other)override;		// トリガー衝突処理
	void OnCollisionEnter(const PhysicsData::CollisionInfo& other)override;	// 物理衝突処理

	/* アイテムのタイプの設定 */
	const UtilityData::ITEM_TYPE get_ItemType()const { return m_ItemType; }
	void set_ItemType(const UtilityData::ITEM_TYPE &_type) { m_ItemType = _type; }

	void ApplyRecovery(class GameObject* _pPlayerObj, float _rate);
	void AddWeapon(class GameObject* _pPlayerObj);
	void AddArmor(class GameObject* _pPlayerObj);
};

