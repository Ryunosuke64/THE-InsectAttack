#include "pch.h"
#include "Component_MissionLootUI.h"
#include "Component_MissionInventory.h"

using namespace VECTOR2;

//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//*----------------------------------------------------------------------------------------
MissionLootUI::MissionLootUI(std::weak_ptr<GameObject> pOwner, int updateRank) :
	IComponent(pOwner, updateRank)
{

}

//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
MissionLootUI::~MissionLootUI()
{

}


//*---------------------------------------------------------------------------------------
//*【?】初期化
//*
//* [引数]
//* RendererEngine& : 描画エンジンの参照
//*
//* [返値] なし
//*----------------------------------------------------------------------------------------
void MissionLootUI::Start(RendererEngine& renderer)
{

}


//*---------------------------------------------------------------------------------------
//*【?】更新処理
//*
//* [引数]
//* RendererEngine& : 描画エンジンの参照
//*
//* [返値] なし
//*----------------------------------------------------------------------------------------
void MissionLootUI::Update(RendererEngine& renderer)
{
	
}


//*---------------------------------------------------------------------------------------
//*【?】描画処理
//*
//* [引数]
//* RendererEngine& : 描画エンジンの参照
//*
//* [返値] なし
//*----------------------------------------------------------------------------------------
void MissionLootUI::Draw(RendererEngine& renderer)
{
	if (m_pMissionInventory.expired())
	{
		return;
	}

	int crntPoint = m_pMissionInventory.lock()->get_CrntPoint();

	std::string crntPointStr = "功績値：" + std::to_string(crntPoint);
	float width = FLOAT_CAST(Master::m_pDataManager->get_ScreenWidth());
	float height = FLOAT_CAST(Master::m_pDataManager->get_ScreenHeight());

	Master::m_pDirectWriteManager->SetOutLine(1.0f, D2D1::ColorF(0.0f, 0.0f, 0.0f));
	Master::m_pDirectWriteManager->DrawStringToAligment(
		crntPointStr, 
		VECTOR2::VEC2(1650.0f, 300.0f), 
		"White_20_STD",
		H_ALIGNMENT::LEADING, 
		V_ALIGNMENT::TOP, 
		VEC2(width, height));
	Master::m_pDirectWriteManager->SetOutLine(0.0f);
}