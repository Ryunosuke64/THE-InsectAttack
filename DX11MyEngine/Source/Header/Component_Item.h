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
	class RigidBody* m_pRigidBody;
	class BoxCollider* m_pBoxCollider;
	std::weak_ptr<GameObject> m_pPlayerObj;
	
	const float SUCTION_DURATION = 2.0f;	// 吸引時間
	const float SUCTION_DISTANCE = 10.0f;	// 吸引距離
	float m_SuctionElapsedTime = 0.0f;		// 吸引経過時間

	int m_EffectHandle;					// エフェクトハンドル
	int m_PointValue;					// ポイント値

public:
	Item(std::weak_ptr<GameObject> pOwner, int updateRank = 100);
	~Item();

	void Start(RendererEngine& renderer) override;		// 初期化
	void Update(RendererEngine& renderer) override;		// 更新処理
	void OnTriggerEnter(const PhysicsData::CollisionInfo& other)override;		// トリガー衝突処理

	void ResetParam();

	/* アイテムのタイプの設定 */
	const UtilityData::ITEM_TYPE get_ItemType()const { return m_ItemType; }
	void set_ItemType(const UtilityData::ITEM_TYPE &_type) { m_ItemType = _type; }

	/* ポイント値の設定 */
	void set_PointValue(int _value) { m_PointValue = _value; }

	void ApplyRecovery(class GameObject* _pPlayerObj, float _rate);
	void AddWeapon(class GameObject* _pPlayerObj);
	void AddArmor(class GameObject* _pPlayerObj);
};

