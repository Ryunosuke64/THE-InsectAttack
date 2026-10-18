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
	std::weak_ptr<class LineRenderer> m_pLineRendererComp;	// レーザー用のライン
	std::weak_ptr<class PointLight> m_pFlashPointLight;		// 発射時のフラッシュ用
	float m_Range;				// 射程距離
	float m_FireInterval;		// 発射間隔（1秒間あたり）
	float m_FireCooldownTimer;	// 発射タイマー
	bool m_IsBurstFire;			// バースト射撃か
	float m_BurstFireTimer;		// バースト射撃タイマー
	int m_BurstFireRemaining;	// バースト射撃回数
	int m_AmmoRemaining;		// 残弾数
	bool m_IsStopFire;			// 発射止める
	SoundID m_FireSoundID;		// 発射サウンドID
	VECTOR3::VEC3 m_LocalMuzzleOffset;
	VECTOR3::VEC3 m_TargetPos;
public:
	AI_GunWeapon(std::weak_ptr<GameObject> pOwner, int updateRank = 100);
	~AI_GunWeapon();

	void Start(RendererEngine& renderer) override;		// 初期化
	void Update(RendererEngine& renderer) override;		// 更新処理
	void LateUpdate(RendererEngine& renderer) override;		// 更新処理


	void BurstFire(RendererEngine& renderer);	// バースト発射
	void OnceFire(RendererEngine& renderer);	// 単発発射
	void Fire(RendererEngine& renderer);		// 弾の発射
	bool Setup(const WeaponData::BaseWeaponData* _pWeaponData)override;
	void set_AimPosition(const VECTOR3::VEC3& _targetPos) { m_TargetPos = _targetPos; };	// 射撃する目標位置の設定
	void set_AimDirection(const VECTOR3::VEC3& worldDirection);
	void UseOwnerForward();
	bool IsBurstFireEnd()const { return m_BurstFireTimer <= 0; };	// バースト射撃が終了しているか
	void set_LocalMuzzleOffset(const VECTOR3::VEC3& _offset) { m_LocalMuzzleOffset = _offset; }	// 発射位置のローカルオフセット設定

	const WeaponData::GunWeaponData* get_GunWeaponData()const;		// 武器のパラメータ取得
	std::weak_ptr < class GameObject >SelectHomingTarget(const VECTOR3::VEC3& _cameraPos, const VECTOR3::VEC3& _fw, float _rockOnRange, float _rockOnHalfAngleDeg);

	// 使用しない
	WeaponData::WeaponUIData get_WeaponUIData()const override { return{}; };
	void SwicthReset() override { ; };
};

