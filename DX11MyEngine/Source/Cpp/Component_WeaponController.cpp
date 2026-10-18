#include "pch.h"
#include "Component_WeaponController.h"
#include "Component_GunWeapon.h"
#include "Component_WeaponBase.h"
#include "Component_SpriteRenderer.h"

using namespace VECTOR4;
using namespace VECTOR3;
using namespace VECTOR2;

constexpr int WEAPON_CHANGE_INTERVAL = 5;	// 武器の切り替え間隔


/* ----- 弾ゲージスプライト ----- */
// 前
const VEC2 g_BulletGageSpriteCenterPos  = VEC2(1420.0f, 800.0f);
const VEC2 g_BulletGageSpriteCenterSize = VEC2(420.0f, 200.0f);

// 後ろ
const VEC2 g_BulletGageSpriteBackPos    = VEC2(1620.0f, 700.0f);
const VEC2 g_BulletGageSpriteBackSize   = VEC2(210.0f, 100.0f);



/* ----- 武器スプライト ----- */
// 前
const VEC2 g_WeaponSpriteCenterPos  = VEC2(1220.0f, 830.0f);
const VEC2 g_WeaponSpriteCenterSize = VEC2(200.0f, 120.0f);

// 後ろs
const VEC2 g_WeaponSpriteBackPos    = VEC2(1520.0f, 715.0f);
const VEC2 g_WeaponSpriteBackSize   = VEC2(0.0f, 0.0f);		// 後ろの武器スプライトは表示しない


/* 武器のパラメータ文字の位置 */
const VEC2 g_WeaponParamStr_ParentBoxCenterSize = VEC2(800.0f, 50.0f);
const VEC2 g_WeaponParamStr_ParentBoxBackSize = VEC2(400.0f, 50.0f);

const VEC2 g_WeaponNameParamStrCenterPos = VEC2(970.0f, 810.0f);
const VEC2 g_WeaponAmmoParamStrCenterPos = VEC2(970.0f, 870.0f);
const VEC2 g_WeaponNameParamStrBackPos = VEC2(1400.0f, 690.0f);
const VEC2 g_WeaponAmmoParamStrBackPos = VEC2(1400.0f, 720.0f);


//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//*----------------------------------------------------------------------------------------
WeaponController::WeaponController(std::weak_ptr<GameObject> pOwner, int updateRank):
	IComponent(pOwner, updateRank),
	m_CrntWeaponSlotIndex(-1),
	m_MaxSlot(-1),
	m_WeaponChangeIntervalCounter(0)
{
	this->set_Tag("WeaponController");
}

//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
WeaponController::~WeaponController()
{
	m_WeaponArray.clear();
}


//*---------------------------------------------------------------------------------------
//*【?】初期化処理
//*
//* [引数]
//* renderer : 描画エンジンの参照
//*_maxSlot  : 最大スロット数
//* 
//* 
//* [返値] なし
//*----------------------------------------------------------------------------------------
bool WeaponController::Setup(RendererEngine& renderer, int _maxSlot)
{
	m_MaxSlot = _maxSlot;
	
	// 最大数分メモリ確保
	m_WeaponArray.reserve(m_MaxSlot);

	SetupWeaponSprite(renderer);	// 武器アイコン用スプライト生成

	SetupReloadSprite(renderer);	// リロード用スプライト生成

	return true;
}


//*---------------------------------------------------------------------------------------
//*【?】更新処理
//*
//* [引数]
//* renderer : 描画エンジンの参照
//*
//* [返値] なし
//*----------------------------------------------------------------------------------------
void WeaponController::Update(RendererEngine& renderer)
{
	// ポーズ中であれば、武器の操作を受け付けない
	if (Master::m_pDataManager->get_IsPause())
	{
		return;	// ポーズ中なら返す
	}


	// プレイヤーが死んでたら、武器の有効を解除し、操作不能に
	if (Master::m_pDataManager->get_IsPlayerDead())
	{
		for (int i = 0; i < m_MaxSlot; i++)
		{
			m_WeaponArray[i]->get_WeaponFlags().DisableFlag(WEAPON_STATUS::ENABLED);
		}

		return;
	}

	// 武器切り替えカウンター
	if (m_WeaponChangeIntervalCounter != 0)
	{
		m_WeaponChangeIntervalCounter--;
		return;
	}

	// 武器１
	if (GetInputDown(GAME_CONFIG::WEAPON_CHANGE1))
	{
		SwitchWeapon(0);
	}
	// 武器２
	if (GetInputDown(GAME_CONFIG::WEAPON_CHANGE2))
	{
		SwitchWeapon(1);
	}

	// マウスホイールで武器切り替え
	LONG scroll = Master::m_pInputManager->GetMousePosSlopeZ();
	if (scroll > 0)
	{
		NextWeapon();
	}
	else if (scroll < 0)
	{
		PrevWeapon();
	}

	// 現在装備している武器が、リロード中か
	if (get_IsCrntWeaponReloading())
	{
		/* 背景スプライトの位置と大きさ */
		auto backSpriteRect = m_pReloadBarBackSpriteObj->get_RectTransform().lock();
		backSpriteRect->set_Size(RELOAD_BAR_BACK_SIZE);
		backSpriteRect->set_RectPosition(RELOAD_BAR_BACK_POS);

		/* バー本体 */
		float reloatTime = m_WeaponArray[m_CrntWeaponSlotIndex]->get_WeaponUIData()._reloadTime;
		float crntReloatTime = m_WeaponArray[m_CrntWeaponSlotIndex]->get_WeaponUIData()._crntReloadTime;
		
		// リロードの進捗に合わせて、スプライトの横幅を変える
		float t = crntReloatTime / reloatTime;
		auto barRect = m_pReloadBarSpriteObj->get_RectTransform().lock();
		barRect->set_Size(RELOAD_BAR_SIZE.x * t, RELOAD_BAR_SIZE.y);	// 横幅を動かす


		/* 文字 */
		float width = FLOAT_CAST(Master::m_pDataManager->get_ScreenWidth());
		float height = FLOAT_CAST(Master::m_pDataManager->get_ScreenHeight());
		Master::m_pDirectWriteManager->SetOutLine(1.5f, D2D1::ColorF(0.0f, 0.0f, 0.0f));
		Master::m_pDirectWriteManager->DrawStringToAligment("リロード中", RELOAD_TEXT_POS, "White_20_STD", H_ALIGNMENT::CENTER, V_ALIGNMENT::CENTER, VEC2(width, height));
		Master::m_pDirectWriteManager->SetOutLine(0.0f);
	}
	// リロード中でない場合は、サイズを0.0にする
	else
	{
		auto backSpriteRect = m_pReloadBarBackSpriteObj->get_RectTransform().lock();
		backSpriteRect->set_Size(0.0f);

		auto barRect = m_pReloadBarSpriteObj->get_RectTransform().lock();
		barRect->set_Size(0.0f);
	}
}

//*---------------------------------------------------------------------------------------
//*【?】描画処理
//*
//* [引数]
//* renderer : 描画エンジンの参照
//*
//* [返値] なし
//*----------------------------------------------------------------------------------------
void WeaponController::Draw(RendererEngine& renderer)
{
	for (int i = 0; i < m_MaxSlot; i++)
	{
		VEC2 weaponPos = g_WeaponNameParamStrBackPos;
		VEC2 ammoPos = g_WeaponAmmoParamStrBackPos;
		VEC2 parentSize = g_WeaponParamStr_ParentBoxBackSize;
		std::string formatTag = "White_15_STD";

		// 現在、装備している武器は前の位置へ
		if (i == m_CrntWeaponSlotIndex)	{
			weaponPos = g_WeaponNameParamStrCenterPos;
			ammoPos = g_WeaponAmmoParamStrCenterPos;
			formatTag = "White_30_STD";
			parentSize = g_WeaponParamStr_ParentBoxCenterSize;
		}

		int maxAmmo = m_WeaponArray[i]->get_WeaponUIData()._ammoMaxNum;
		int remainingAmmo = m_WeaponArray[i]->get_WeaponUIData()._ammoRemaining;
		std::wstring weaponName = m_WeaponArray[i]->get_WeaponUIData()._name;
		std::string ammoStr = std::to_string(remainingAmmo) + " / " + std::to_string(maxAmmo);

		/* 武器名 */
		Master::m_pDirectWriteManager->DrawStringToAligment(weaponName, weaponPos, formatTag, H_ALIGNMENT::TRAILING, V_ALIGNMENT::CENTER, parentSize);

		/* 残弾数 */
		Master::m_pDirectWriteManager->DrawStringToAligment(ammoStr, ammoPos, formatTag, H_ALIGNMENT::TRAILING, V_ALIGNMENT::CENTER, parentSize);
	}
}


//*---------------------------------------------------------------------------------------
//*【?】 現在装備している武器がリロード中か
//*
//* [引数] なし
//* [返値] なし
//*----------------------------------------------------------------------------------------
bool WeaponController::get_IsCrntWeaponReloading()
{
	return m_WeaponArray[m_CrntWeaponSlotIndex]->get_WeaponFlags().GetFlag(WEAPON_STATUS::RELODING);
}

//*---------------------------------------------------------------------------------------
//*【?】武器の切り替え
//*
//* [引数]
//* _index : 切り替え先の武器のインデクス
//*
//* [返値] なし
//*----------------------------------------------------------------------------------------
void WeaponController::SwitchWeapon(int _index)
{
	if (_index < 0 || _index >= (int)m_WeaponArray.size() || _index == m_CrntWeaponSlotIndex) return;

	// 現在の武器がセットされていない
	if (m_CrntWeaponSlotIndex < 0)
	{
		return;
	}

	//*****************************************************************************************
	//						武器の切り替え音再生
	//*****************************************************************************************
	Master::m_pSoundManager->Play_RandPitch("weapon.change_01", 200);

	// 切り替え時のリセット
	m_WeaponArray[m_CrntWeaponSlotIndex]->SwicthReset(); 
	
	// 現在の武器を非表示に
	m_WeaponArray[m_CrntWeaponSlotIndex]->get_OwnerObj().lock()->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);

	/* --------------------------------------------------------------
	*  現在の武器背景のスプライトを後ろに
	*/
	auto gageRectTrans = m_pBulletGageSpriteObjArray[m_CrntWeaponSlotIndex]->get_RectTransform().lock();
	gageRectTrans->set_RectPosition(g_BulletGageSpriteBackPos);
	gageRectTrans->set_Size(g_BulletGageSpriteBackSize);

	auto weaponRectTrans = m_pWeaponSpriteObjArray[m_CrntWeaponSlotIndex]->get_RectTransform().lock();
	weaponRectTrans->set_RectPosition(g_WeaponSpriteBackPos);
	weaponRectTrans->set_Size(g_WeaponSpriteBackSize);


	// 新しい武器を表示
	m_CrntWeaponSlotIndex = _index;
	m_WeaponArray[m_CrntWeaponSlotIndex]->get_OwnerObj().lock()->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);


	/* --------------------------------------------------------------
	*  新しい武器背景のスプライトを前へ
	*/
	gageRectTrans = m_pBulletGageSpriteObjArray[m_CrntWeaponSlotIndex]->get_RectTransform().lock();
	gageRectTrans->set_RectPosition(g_BulletGageSpriteCenterPos);
	gageRectTrans->set_Size(g_BulletGageSpriteCenterSize);

	weaponRectTrans = m_pWeaponSpriteObjArray[m_CrntWeaponSlotIndex]->get_RectTransform().lock();
	weaponRectTrans->set_RectPosition(g_WeaponSpriteCenterPos);
	weaponRectTrans->set_Size(g_WeaponSpriteCenterSize);



	// 武器切り替えカウンタの設定
	m_WeaponChangeIntervalCounter = WEAPON_CHANGE_INTERVAL;
}

//*---------------------------------------------------------------------------------------
//*【?】次の武器へ切り替え
//*
//* [引数] なし
//* [返値] なし
//*----------------------------------------------------------------------------------------
void WeaponController::NextWeapon()
{
	int nextIdx = (m_CrntWeaponSlotIndex + 1) % (int)m_WeaponArray.size();
	SwitchWeapon(nextIdx);
}

//*---------------------------------------------------------------------------------------
//*【?】前の武器へ切り替え
//*
//* [引数] なし
//* [返値] なし
//*----------------------------------------------------------------------------------------
void WeaponController::PrevWeapon()
{
	int count = (int)m_WeaponArray.size();
	int prevIdx = (m_CrntWeaponSlotIndex - 1 + count) % count;
	SwitchWeapon(prevIdx);
}

//*---------------------------------------------------------------------------------------
//*【?】武器の登録
//*
//* [引数] 
//* _pWeapon : 武器コンポーネントの共有ポインタ
//* _slot : 武器をセットするスロット 
//* [返値] なし
//*----------------------------------------------------------------------------------------
void WeaponController::RegisterWeapon(std::shared_ptr<class GunWeapon> _pWeapon, int _slot)
{
	if (_slot >= m_MaxSlot)return;

	m_WeaponArray.emplace_back(_pWeapon);
}


//*----------------------------------------------------------------------------------------
//*【?】開始時に持っている武器の設定
//*
//* [引数] 
//* _slot : 武器スロット（ 0以上 m_MaxSlot以下） 
//* [返値] なし
//*----------------------------------------------------------------------------------------
void WeaponController::StartingWeapon(int _slot)
{
	m_CrntWeaponSlotIndex = _slot;

	// アクティブ状態に
	m_WeaponArray[m_CrntWeaponSlotIndex]->get_OwnerObj().lock()->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);

	// 装備武器以外は非アクティブに
	for (int i = 0; i < m_MaxSlot; i++)
	{
		if (m_CrntWeaponSlotIndex != i) {
			m_WeaponArray[i]->get_OwnerObj().lock()->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
		}
	}
}

//*----------------------------------------------------------------------------------------
//*【?】武器のクリア
//*
//* [引数] なし
//* [返値] なし
//*----------------------------------------------------------------------------------------
void WeaponController::ClearWeapon()
{
	for (int i = 0; i < m_MaxSlot; i++)
	{
		m_pBulletGageSpriteObjArray[i]->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
		m_pWeaponSpriteObjArray[i]->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
	}

	m_pBulletGageSpriteObjArray.clear();
	m_pWeaponSpriteObjArray.clear();
	m_WeaponArray.clear(); 

	// スプライトをプールへ返す
	m_pReloadBarSpriteObj->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
	m_pReloadBarBackSpriteObj->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);

	// ポーズ用の更新・描画フラグを戻す
	m_pReloadBarSpriteObj->set_IsUpdateAllowedDuringPause(true);
	m_pReloadBarSpriteObj->set_IsDrawAllowedDuringPause(true);
	m_pReloadBarBackSpriteObj->set_IsUpdateAllowedDuringPause(true);
	m_pReloadBarBackSpriteObj->set_IsDrawAllowedDuringPause(true);
}


//*----------------------------------------------------------------------------------------
//*【?】武器アイコンスプライトの生成
//*
//* [引数] 
//* &renderer : 描画エンジンの参照
//* [返値] なし
//*----------------------------------------------------------------------------------------
void WeaponController::SetupWeaponSprite(RendererEngine& renderer)
{

	UIData::RectTransformData rectData;
	UIData::SpriteUIData spriteData;

	// 弾のゲージ背景スプライトを作る **********************************************
	rectData._pos = g_BulletGageSpriteCenterPos;
	rectData._size = g_BulletGageSpriteCenterSize;
	rectData._anchorMax = VEC2(0.0f, 0.0f);
	rectData._anchorMin = VEC2(0.0f, 0.0f);
	spriteData._tag = "WeaponGage_1";
	spriteData._imagePath = "Resource/Texture/UI/BulletGage.png";
	spriteData._layerRank = 101;
	m_pBulletGageSpriteObjArray.push_back(Master::m_pUIManager->GetSprite(renderer, rectData, spriteData));

	// 後ろ
	spriteData._tag = "WeaponGage_2";
	rectData._pos = g_BulletGageSpriteBackPos;
	rectData._size = g_BulletGageSpriteBackSize;
	m_pBulletGageSpriteObjArray.push_back(Master::m_pUIManager->GetSprite(renderer, rectData, spriteData));


	// 武器のスプライトを作る **********************************************
	rectData._pos = g_WeaponSpriteCenterPos;
	rectData._size = g_WeaponSpriteCenterSize;
	spriteData._tag = "GunIcon_AR_01";
	spriteData._imagePath = "Resource/Texture/UI/GunIcon_AR_01.png";	// TODO:武器のパラメータから使用する画像を指定できるようにする
	spriteData._layerRank = 101;
	m_pWeaponSpriteObjArray.push_back(Master::m_pUIManager->GetSprite(renderer, rectData, spriteData));

	// 後ろ
	spriteData._imagePath = "Resource/Texture/UI/GunIcon_RL_01.png";
	spriteData._tag = "GunIcon_RL_01";
	rectData._pos = g_WeaponSpriteBackPos;
	rectData._size = g_WeaponSpriteBackSize;
	m_pWeaponSpriteObjArray.push_back(Master::m_pUIManager->GetSprite(renderer, rectData, spriteData));
}


//*----------------------------------------------------------------------------------------
//*【?】リロードスプライトの生成
//*
//* [引数] 
//* &renderer : 描画エンジンの参照
//* [返値] なし
//*----------------------------------------------------------------------------------------
void WeaponController::SetupReloadSprite(RendererEngine& renderer)
{
	// リロードバー用スプライトを作る **********************************************
	UIData::RectTransformData rectData;
	UIData::SpriteUIData spriteData;
	rectData._size = VEC2(0.0f, 0.0f);
	rectData._pos = RELOAD_BAR_POS;
	rectData._anchorMax = VEC2(0.5f, 0.5f);		// 中心を画面の真ん中に
	rectData._anchorMin = VEC2(0.5f, 0.5f);
	rectData._pivot = VEC2(0.0f, 0.0f);		// ピボットは左上
	spriteData._tag = "ReloadBar";
	spriteData._layerRank = 108;
	spriteData._color = VEC4(0.0f, 1.0f, 0.0f, 1.0f);
	spriteData._shaderType = SHADER_TYPE::FORWARD_UNLIT_UI_NOTEXTURE_SPRITE;
	m_pReloadBarSpriteObj = Master::m_pUIManager->GetSprite(renderer, rectData, spriteData);

	// リロードバー背景用スプライトを作る **********************************************
	rectData._size = RELOAD_BAR_BACK_SIZE;
	rectData._pos = RELOAD_BAR_BACK_POS;
	rectData._anchorMax = VEC2(0.5f, 0.5f);		// 中心を画面の真ん中に
	rectData._anchorMin = VEC2(0.5f, 0.5f);
	rectData._pivot = VEC2(0.5f, 0.5f);
	spriteData._tag = "ReloadBarBack";
	spriteData._layerRank = 107;
	spriteData._color = VEC4(0.05f, 0.05f, 0.05f, 0.8f);	// 黒背景
	spriteData._shaderType = SHADER_TYPE::FORWARD_UNLIT_UI_NOTEXTURE_SPRITE;
	m_pReloadBarBackSpriteObj = Master::m_pUIManager->GetSprite(renderer, rectData, spriteData);

	// ポーズ中は更新と描画を止める
	m_pReloadBarSpriteObj->set_IsUpdateAllowedDuringPause(false);	
	m_pReloadBarSpriteObj->set_IsDrawAllowedDuringPause(false);	
	m_pReloadBarBackSpriteObj->set_IsUpdateAllowedDuringPause(false);	
	m_pReloadBarBackSpriteObj->set_IsDrawAllowedDuringPause(false);
}
