#pragma once
#include "IComponent.h"


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:MissionInventory Class --- */
//
//  ★継承：IComponent ★
//
// 【?】ミッション中の所持品全般
//		入手したアイテムの管理
//		武器の管理、アーマーの管理、ポイントの管理
//		
// ***************************************************************************************
class MissionInventory : public IComponent
{
private:
	int m_CrntPoint;		// 現在のポイント
	int m_CrntGetArmorNum;	// 現在の入手アーマー数
	int m_CrntGetWeaponNum;	// 現在の入手武器数


public:
	MissionInventory(std::weak_ptr<GameObject> pOwner, int updateRank);
	~MissionInventory();

	int get_CrntPoint()const { return m_CrntPoint; }
	int get_CrntGetArmorNum()const { return m_CrntGetArmorNum; }
	int get_CrntGetWeaponNum()const { return m_CrntGetWeaponNum; }

	void AddPoint(int _value);
	void AddArmor() { m_CrntGetArmorNum++; }
	void AddWeapon() { m_CrntGetWeaponNum++; }
	
	void MinusPoint(int value);

	void ResetInventory();
};

