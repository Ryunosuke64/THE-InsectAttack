#pragma once
#include "IComponent.h"

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:Destructible Class --- */
//
//  ★継承：IComponent ★
//
// 【?】破壊可能オブジェクトに取り付けるコンポーネント
//		
// ***************************************************************************************
class Destructible : public IComponent
{
public:
	// 内部識別用の状態
	enum class STATE
	{
		ALIVE,			// 生存
		DESTROYSTART,	// 破壊開始
		DESTROYING,		// 破壊中
		DESTROYED		// 破壊完了
	};

private:
	std::unique_ptr<class IDestructionBehavior>m_pBehavior;
	std::weak_ptr<class Health> m_pHealthComp;
	std::weak_ptr<class MapObject> m_pMapObjectComp;
	STATE m_State;

public:
	Destructible(std::weak_ptr<GameObject> pOwner, int updateRank);
	~Destructible();

	void Start(RendererEngine& renderer) override;	// 初期化
	void Update(RendererEngine& renderer) override;	// 更新
	bool Setup(std::unique_ptr<IDestructionBehavior> behavior);

private:
	void StartDestruction(
		const VECTOR3::VEC3& hitPos,
		const VECTOR3::VEC3& hitDir);

};

