#pragma once
#include "Component_WeaponBase.h"
#include "ConstantWeaponData.h"

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:AI_GunWeapon Class --- */
//
//  ★継承：WeaponBase ★
//
// 【?】AI用銃火器コンポーネント
//		
// ***************************************************************************************
class AI_GunWeapon : public WeaponBase
{
private:

public:
	AI_GunWeapon(std::weak_ptr<GameObject> pOwner, int updateRank = 100);
	~AI_GunWeapon();

	void Start(RendererEngine& renderer) override;		// 初期化
	void Update(RendererEngine& renderer) override;		// 更新処理
	void LateUpdate(RendererEngine& renderer) override;		// 更新処理


	void BurstFire(RendererEngine& renderer);	// バースト発射
	void OnceFire(RendererEngine& renderer);	// 単発発射
	void Fire(RendererEngine& renderer);	// 弾の発射
	bool Setup(const WeaponData::BaseWeaponData* _pWeaponData)override;

	const WeaponData::GunWeaponData* get_GunWeaponData()const;		// 武器のパラメータ取得
	std::weak_ptr < class GameObject >SelectHomingTarget(const VECTOR3::VEC3& _cameraPos, const VECTOR3::VEC3& _fw, float _rockOnRange, float _rockOnHalfAngleDeg);

	WeaponData::WeaponUIData get_WeaponUIData()const override { ; };
};

