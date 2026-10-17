#pragma once
#include "Component_WeaponBase.h"
#include "ConstantWeaponData.h"

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:GunWeapon Class --- */
//
//  ★継承：IComponent ★
//
// 【?】アサルトライフル
//		
// ***************************************************************************************
class GunWeapon : public WeaponBase
{
private:
	std::weak_ptr<class LineRenderer> m_pLineRendererComp;	// レーザー用のライン
	std::weak_ptr<class PointLight> m_pFlashPointLight;		// 発射時のフラッシュ用
	//WeaponData::GunWeaponData m_WeaponParameter;
	float m_Range;				// 射程距離
	float m_CrntReloadTime;		// 現在のリロード時間
	float m_FireInterval;		// 発射間隔（1秒間あたり）
	float m_FireCooldownTimer;	// 発射タイマー
	bool m_IsBurstFire;			// バースト射撃か
	bool m_IsNowBurstFire;		// バースト射撃中か
	float m_BurstFireTimer;		// バースト射撃タイマー
	int m_BurstFireRemaining;	// バースト射撃回数
	int m_AmmoRemaining;		// 残弾数
	bool m_IsNowZoom;			// ズーム中かどうか
	bool m_IsStopFire;			// 発射止める
	StateMachine<GunWeapon>m_StateMachine;

public:
	GunWeapon( std::weak_ptr<GameObject> pOwner, int updateRank = 100);
	~GunWeapon();

	void Start(RendererEngine& renderer) override;		// 初期化
	void Update(RendererEngine& renderer) override;		// 更新処理
	void LateUpdate(RendererEngine& renderer) override;		// 更新処理
	void Draw(RendererEngine& renderer)override;		// 描画処理

	void BurstFire(RendererEngine& renderer);	// バースト発射
	void OnceFire(RendererEngine& renderer);	// 単発発射
	void Fire(RendererEngine& renderer);	// 弾の発射
	bool Setup(const WeaponData::BaseWeaponData* _pWeaponData)override;
	void SwicthReset() override;	// 武器切り替え時のリセット

	std::weak_ptr<class PointLight> get_FlashPointLight() { return m_pFlashPointLight; }	// フラッシュライト取得
	const WeaponData::GunWeaponData* get_GunWeaponData()const;		// 武器のパラメータ取得
	std::weak_ptr < class GameObject >SelectHomingTarget(const VECTOR3::VEC3& _cameraPos, const VECTOR3::VEC3& _fw, float _rockOnRange, float _rockOnHalfAngleDeg);

	bool get_IsNowZoom()const { return m_IsNowZoom; }								// ズームフラグの取得
	bool get_IsStopFire()const { return m_IsStopFire; }								// 攻撃中止フラグの取得
	void set_IsStopFire(bool _flag) { m_IsStopFire = _flag; }						// 攻撃中止フラグの設定
	int get_AmmoRemaining()const { return m_AmmoRemaining; }						// 残弾数の取得
	void set_AmmoRemaining(int _ammo) { m_AmmoRemaining = _ammo; }					// 残弾数の設定
	void set_CrntReloadTime(float _time) { m_CrntReloadTime = _time; }				// 現在のリロード時間の設定

	WeaponData::WeaponUIData get_WeaponUIData()const override;
};



