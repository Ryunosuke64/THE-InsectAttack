#pragma once
#include "IComponent.h"


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @: MissionLootUI Class --- */
//
//  ★継承：Component ★
//
// 【?】ミッション中に取得したアイテムなどのUIを表示
// 
// ***************************************************************************************
class  MissionLootUI : public IComponent
{
private:
	std::weak_ptr<class MissionInventory>m_pMissionInventory;

public:
    MissionLootUI(std::weak_ptr<GameObject> pOwner, int updateRank = 100);
    ~MissionLootUI();

	void Start(RendererEngine& renderer) override;		// 初期化
	void Update(RendererEngine& renderer) override;		// 更新処理
	void Draw(RendererEngine& renderer) override;		// 描画処理

	void set_MissionInventory(std::weak_ptr<MissionInventory> _pMissionInventory) { m_pMissionInventory = _pMissionInventory; }
};

