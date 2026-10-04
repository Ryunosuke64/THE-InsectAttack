#pragma once
#include "IState.h"
#include "CompositeState.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//						ゲームシーンのステートのヘッダクラス定義をまとめたヘッダ
// 
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:Root_GameSceneState Class --- */
//
// 【?】ゲームシーンの親ステート
//
// ***************************************************************************************
class Root_GameSceneState : public CompositeState<SceneManager>
{
private:

public:
	void OnEnter(SceneManager* pOwner) override;
	void OnExit(SceneManager* pOwner)override;
	int Update(SceneManager* pOwner)override;
	void Draw(SceneManager* pOwner)override;
};


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:c_Game_LoadProcess Class --- */
//
// 【?】ゲームシーンの子ステート
//		ゲームに必要なもののロード処理
//
// ***************************************************************************************
class c_Game_LoadProcess : public IState<SceneManager>
{
private:
	bool m_IsLoad = false;
	class GameObject* m_pLoadBackObj = nullptr;

public:
	void OnEnter(SceneManager* pOwner) override;
	void OnExit(SceneManager* pOwner)override;
	int Update(SceneManager* pOwner)override;
	void Draw(SceneManager* pOwner)override;
};



// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:c_Game_Play Class --- */
//
// 【?】ゲームシーンの子ステート
//		ゲームプレイ
//	
//		ポーズ画面の子ステートを持つ
//
// ***************************************************************************************
class c_Game_Play : public CompositeState<SceneManager>
{
private:
	int m_EnemyNum;	// 残り敵数

	/* オブジェクト */
	std::weak_ptr<class GameObject>m_pBomber[3];
	std::weak_ptr<class GameObject>m_pPlayerObj;
	class GameObject* m_pEnemyNumBackSpriteObj;	// 残り敵数の背景に使用するスプライト

public:
	void OnEnter(SceneManager* pOwner) override;
	void OnExit(SceneManager* pOwner)override;
	int Update(SceneManager* pOwner)override;
	void Draw(SceneManager* pOwner)override;

	void BomberMove();
};


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:c_Game_Pause Class --- */
//
// 【?】ゲームプレイ中のポーズ画面
//		
//		必要なものは子として持つ
//
// ***************************************************************************************
class c_Game_Pause : public CompositeState<SceneManager>
{
private:
	class GameObject* m_pPauseBackSpriteObj;	// ポーズ背景スプライト
	std::array<class GameObject*, UINT_CAST(UtilityData::PAUSE_ITEM::NUM)> m_pPauseItemBackButtonObjArray;								// 項目背景のボタンオブジェクト
	std::array<std::weak_ptr<class RectTransform>, UINT_CAST(UtilityData::PAUSE_ITEM::NUM)> m_pPauseItemBackRectTransformArray;			// 項目背景のRectTransform
	SceneStateEnums::c_GAME m_NextState;
	UtilityData::PAUSE_ITEM m_CrntSelectPauseState;	// 現在の選択中のポーズ項目

	/* 定数 */
	const VECTOR2::VEC2 PAUSE_BACK_SPRITE_SIZE = VECTOR2::VEC2(550.0f, 550.0f);	// ポーズの背景スプライトをのサイズ
	const VECTOR2::VEC2 PAUSE_BACK_SPRITE_POS = VECTOR2::VEC2(660.0f, 300.0f);	// ポーズの背景スプライトをの位置
	const VECTOR2::VEC2 PAUSE_ITEM_BUTTON_SIZE = VECTOR2::VEC2(500.0f, 80.0f);	// ポーズ項目のスプライトのサイズ
	const VECTOR2::VEC2 PAUSE_ITEM_START_POS = VECTOR2::VEC2(670.0f, 340.0f);	// 項目の開始位置（画面の中央を中心）
	const float PAUSE_ITEM_POS_Y_BETWEEN_DIST = 100.0f;							// 項目同士のY距離



public:
	void OnEnter(SceneManager* pOwner) override;
	void OnExit(SceneManager* pOwner)override;
	int Update(SceneManager* pOwner)override;
	void Draw(SceneManager* pOwner)override;

private:

	void ChangeConfigValue(UtilityData::CONFIG_ITEM _item, bool _isLeftIndex);	// 設定値を変更する
	void Button_OnClicFunction(SceneManager* pOwner, UtilityData::PAUSE_ITEM _type);		// ボタンが押された際の処理
	void BorrowFromUIPoolAndSetup(SceneManager* pOwner);		// プールからスプライトを取り出し、初期化
	void ReleaseToUIPool();						// プールへ返す
};

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:c_Game_Config Class --- */
//
// 【?】ゲームシーンの子ステート
//		設定画面
//
// ***************************************************************************************
class c_Game_Config : public IState<SceneManager>
{
private:
	std::weak_ptr<class ConfigSetting> m_pConfigSetting;	// 設定画面用コンポーネントを持つ

public:
	void OnEnter(SceneManager* pOwner) override;
	void OnExit(SceneManager* pOwner)override;
	int Update(SceneManager* pOwner)override;
	void Draw(SceneManager* pOwner)override;

};


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:c_Game_Manual Class --- */
//
// 【?】操作説明（マニュアル）の表示
//
// ***************************************************************************************
class c_Game_Manual : public IState<SceneManager>
{
private:
	class GameObject* m_pManualSpriteObj;	// マニュアルスプライト
	float m_Timer;

	/* 定数 */
	const VECTOR2::VEC2 MANUAL_SPRITE_SIZE = VECTOR2::VEC2(960.0f, 540.0f);	// マニュアルスプライトをのサイズ
	const VECTOR2::VEC2 MANUAL_SPRITE_POS = VECTOR2::VEC2(0.0f, 0.0f);		// マニュアルスプライトの位置
	const float MANUAL_SPRITE_SPWAN_DURATION = 1.0f;						// マニュアルスプライトの表示完了までの時間

public:
	void OnEnter(SceneManager* pOwner) override;
	void OnExit(SceneManager* pOwner)override;
	int Update(SceneManager* pOwner)override;
	void Draw(SceneManager* pOwner)override;
};
