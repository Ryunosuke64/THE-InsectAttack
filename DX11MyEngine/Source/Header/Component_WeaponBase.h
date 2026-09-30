#pragma once
#include "IComponent.h"
#include "ConstantWeaponData.h"

enum class WEAPON_STATUS : unsigned
{
	ENABLED = 1 << 0,
	IDLE = 1 << 1,
	FIRE = 1 << 2,
	RELODING = 1 << 3,
};

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:WeaponBase Class --- */
//
//  ★継承：IComponent ★
//
// 【?】武器の基底コンポーネント
//		
// ***************************************************************************************
class WeaponBase : public IComponent
{
protected:
	GIGA_Engine::BitFlag::BIT_FLAG<unsigned int> m_WeaponFlags;
	const WeaponData::BaseWeaponData *m_pWeaponData;

public:
	WeaponBase(std::weak_ptr<GameObject> pOwner, int updateRank = 100);
	virtual ~WeaponBase() = default;

	GIGA_Engine::BitFlag::BIT_FLAG<unsigned int>& get_WeaponFlags() { return m_WeaponFlags; }
	const GIGA_Engine::BitFlag::BIT_FLAG<unsigned int>& get_WeaponFlags() const { return m_WeaponFlags; }
		
	virtual bool Setup(const WeaponData::BaseWeaponData* _pWeaponData) = 0;
	virtual void SwicthReset() = 0;	// 武器切り替え時のリセット

	virtual WeaponData::WeaponUIData get_WeaponUIData() const = 0;	// UIの表示に必要な武器データ（派生先で具体的に記述）
};