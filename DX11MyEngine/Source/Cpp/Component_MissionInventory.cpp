#include "pch.h"
#include "Component_MissionInventory.h"
#include "ConstantGameData.h"


//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//*----------------------------------------------------------------------------------------
MissionInventory::MissionInventory(std::weak_ptr<GameObject> pOwner, int updateRank) :
	IComponent(pOwner, updateRank),
	m_CrntPoint(0),
	m_CrntGetArmorNum(0),
	m_CrntGetWeaponNum(0)
{

}

//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
MissionInventory::~MissionInventory()
{

}


//*---------------------------------------------------------------------------------------
//*【?】インベントリのリセット
//*
//* [引数] なし
//* [返値] なし 
//*----------------------------------------------------------------------------------------
void MissionInventory::ResetInventory()
{
	m_CrntPoint = 0;
	m_CrntGetArmorNum = 0;
	m_CrntGetWeaponNum = 0;
}

//*---------------------------------------------------------------------------------------
//*【?】スコアを増やす
//*
//* [引数] 
//* value : 増やす値 
//* [返値] なし 
//*----------------------------------------------------------------------------------------
void MissionInventory::AddPoint(int _value)
{
	m_CrntPoint += _value;

	m_CrntPoint = std::clamp(
		m_CrntPoint,
		GameData::MIN_POINT,
		GameData::MAX_POINT
	);
}


//*---------------------------------------------------------------------------------------
//*【?】スコアを減らす
//*
//* [引数] 
//* value : 減らす値 
//* [返値] なし 
//*----------------------------------------------------------------------------------------
void MissionInventory::MinusPoint(int value)
{
	m_CrntPoint -= value;


	m_CrntPoint = std::clamp(
		m_CrntPoint, 
		GameData::MIN_POINT, 
		GameData::MAX_POINT
	);
}