#include "pch.h"
#include "GameScene_StateHeader.h"
#include "ResourceManager.h"
#include "SceneStateEnums.h"
#include "InputFactory.h"
#include "Component_SpriteRenderer.h"
#include "Component_ButtonUI.h"
#include "GameObjectManager.h"

using namespace SceneStateEnums;
using namespace UtilityData;
using namespace VECTOR2;
using namespace VECTOR3;
using namespace VECTOR4;

//*---------------------------------------------------------------------------------------
//* @:c_Game_Pause Class 
//*【?】開始
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
void c_Game_Pause::OnEnter(SceneManager* pOwner)
{
	// 最初は空
	this->SetInitChildState(pOwner, c_GAME::c_GAME_NONE);

	// 連続入力防止のため、30フレーム止める
	Master::m_pInputManager->StopInput(30);

	Master::m_pDataManager->set_IsPause(true);

	// UIの初期化
	BorrowFromUIPoolAndSetup(pOwner);

	m_CrntSelectPauseState = PAUSE_ITEM::NONE;

	m_NextState = c_GAME::c_GAME_PAUSE;

}


//*---------------------------------------------------------------------------------------
//* @:c_Game_Pause Class 
//*【?】終了
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
void c_Game_Pause::OnExit(SceneManager* pOwner)
{	
	Master::m_pDataManager->set_IsPause(false);

	// UIオブジェクトを開放
	ReleaseToUIPool();

	// 連続入力防止のため、30フレーム止める
	Master::m_pInputManager->StopInput(10);
}


//*---------------------------------------------------------------------------------------
//* @:c_Game_Pause Class 
//*【?】更新
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
int c_Game_Pause::Update(SceneManager* pOwner)
{
	float deltaTime = Master::m_pTimeManager->get_DeltaTime();


	//////////////////////////////////////////////////////////////////////////////////////////
	//
	//						子ステートの処理
	// 
	//////////////////////////////////////////////////////////////////////////////////////////
	if (m_CrntChildStateID != -1) {
		// 子ステートの更新
		int newState = m_pChildStateMap[m_CrntChildStateID]->Update(pOwner);

		// マニュアルのままなら、その後の処理を飛ばして返す
		if (newState == c_GAME::c_GAME_MANUAL || newState == c_GAME::c_GAME_CONFIG)
		{
			return c_GAME::c_GAME_PAUSE;
		}
		// ポーズのままであれば、子をNONEに戻す
		else if (newState == c_GAME::c_GAME_PAUSE)
		{
			m_pChildStateMap[m_CrntChildStateID]->OnExit(pOwner);	// 子ステートの終了
			this->SetInitChildState(pOwner, c_GAME::c_GAME_NONE);

			// UIを再度、セットアップする
			BorrowFromUIPoolAndSetup(pOwner);

			m_CrntSelectPauseState = PAUSE_ITEM::NONE;	// 選択状態をリセット
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	//
	//						現在のステートの処理
	// 
	//////////////////////////////////////////////////////////////////////////////////////////
	// 右クリックでメインメニューへ戻る
	if (GetMouseClickDown(MOUSE_BUTTON_STATE::RIGHT) || GetInputDown(GAME_CONFIG::PAUSE))
	{
		// ****************************************************
		//				戻る際のSE再生
		// ****************************************************
		Master::m_pSoundManager->Play(SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::SYSTEM_BACK01));

		return c_GAME::c_GAME_PLAY;
	}

	return  m_NextState;
}


//*---------------------------------------------------------------------------------------
//* @:c_Game_Pause Class 
//*【?】描画
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
void c_Game_Pause::Draw(SceneManager* pOwner)
{
	// 何も選択されて無いときに表示
	if (m_CrntSelectPauseState == PAUSE_ITEM::NONE)
	{
		float width = FLOAT_CAST(Master::m_pDataManager->get_ScreenWidth());
		float height = FLOAT_CAST(Master::m_pDataManager->get_ScreenHeight());

		Master::m_pDirectWriteManager->SetOutLine(3.0f, D2D1::ColorF(0.0f, 0.0f, 0.0f));
		Master::m_pDirectWriteManager->DrawStringToAligment("Ｐａｕｓｅ", VECTOR2::VEC2(0.0f, -250.0f), "White_40_STD", H_ALIGNMENT::CENTER, V_ALIGNMENT::CENTER, VEC2(width, height));
		Master::m_pDirectWriteManager->SetOutLine(0.0f);
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	//
	//						子ステートの描画処理
	// 
	//////////////////////////////////////////////////////////////////////////////////////////
	if (m_CrntChildStateID != -1)
	{
		m_pChildStateMap[m_CrntChildStateID]->Draw(pOwner);
	}
}


//*---------------------------------------------------------------------------------------
//*【?】使用するスプライトを初期化する
//*
//* [引数]
//* *pOwner : シーンの親
//*
//* [返値] なし
//*----------------------------------------------------------------------------------------
void c_Game_Pause::BorrowFromUIPoolAndSetup(SceneManager* pOwner)
{
	// メニュー項目のオブジェクトとコンポーネントの取得と設定
	for (int i = 0; i < static_cast<int>(PAUSE_ITEM::NUM); i++)
	{
		VEC2 pos = PAUSE_ITEM_START_POS;
		pos.y += i * PAUSE_ITEM_POS_Y_BETWEEN_DIST;		// 項目ごとにY座標ずらす

		UIData::RectTransformData rectTrans;
		UIData::ButtonUIData buttonData;
		buttonData._imagePath = "Resource/Texture/Title/Frame07.png";
		buttonData._text = g_PauseItemNames[i];
		buttonData._layerRank = 105;
		buttonData._inputValidationState = UIData::STATE::SELECTED;
		buttonData._onClicFunction = [this, pOwner, i]() { this->Button_OnClicFunction(pOwner, static_cast<PAUSE_ITEM>(i)); };	// クリック時の処理
		buttonData._textOffsetPos = VEC2(100.0f, 10.0f);
		rectTrans._size = PAUSE_ITEM_BUTTON_SIZE;
		rectTrans._pos = pos;
		m_pPauseItemBackButtonObjArray[i] = Master::m_pUIManager->GetButton(*m_pRenderer, rectTrans, buttonData);

		// ボタン
		auto button = m_pPauseItemBackButtonObjArray[i]->get_Component<ButtonUI>();
		button->set_Color(VEC4(10.0f, 10.0f, 0.0f, 1.0f), UIData::STATE::HIGH_LIGHTED);	// 選択されている状態で「黄色」に
		button->set_Color(VEC4(5.0f, 5.0f, 0.0f, 1.0f), UIData::STATE::PRESSED);
		button->set_Color(VEC4(10.0f, 10.0f, 0.0f, 1.0f), UIData::STATE::SELECTED);
		m_pPauseItemBackRectTransformArray[i] = m_pPauseItemBackButtonObjArray[i]->get_RectTransform();
	}

	//*****************************************************************************************
	//						最奥背景スプライト
	//*****************************************************************************************
	UIData::SpriteUIData backSpriteData;
	UIData::RectTransformData backRectTrans;
	backSpriteData._color = VEC4(0.0f, 0.0f, 0.0f, 0.85f);
	backSpriteData._shaderType = SHADER_TYPE::FORWARD_UNLIT_UI_NOTEXTURE_SPRITE;
	backSpriteData._layerRank = 100;
	backSpriteData._tag = "ConfigSceneBackSprite";
	backRectTrans._size = PAUSE_BACK_SPRITE_SIZE;
	backRectTrans._pos = PAUSE_BACK_SPRITE_POS;
	m_pPauseBackSpriteObj = Master::m_pUIManager->GetSprite(*m_pRenderer, backRectTrans, backSpriteData);
}


//*---------------------------------------------------------------------------------------
//*【?】スプライトを開放する（プールへ返す）
//*
//* [引数 なし
//* [返値] なし
//*----------------------------------------------------------------------------------------
void c_Game_Pause::ReleaseToUIPool()
{
	if (m_pPauseBackSpriteObj != nullptr) 
	{
		m_pPauseBackSpriteObj->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
	}

	for (int i = 0; i < static_cast<int>(PAUSE_ITEM::NUM); i++)
	{
		if (m_pPauseItemBackButtonObjArray[i] != nullptr)
		{
			m_pPauseItemBackButtonObjArray[i]->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
		}
	}
}

//*---------------------------------------------------------------------------------------
//*【?】ポーズ中のボタンが押された際の処理
//*
//* [引数]
//* *pOwner : シーンの親
//* _type : どの項目か 
//*
//* [返値] なし
//*----------------------------------------------------------------------------------------
void c_Game_Pause::Button_OnClicFunction(SceneManager* pOwner, PAUSE_ITEM _type)
{
	m_CrntSelectPauseState = _type;

	switch (_type)
	{
	case UtilityData::PAUSE_ITEM::NONE:
		break;

		/* ゲームに戻る */
	case UtilityData::PAUSE_ITEM::RETURN_GAME:
	{
		m_NextState = c_GAME::c_GAME_PLAY;
	}
		break;
		/* 再出撃 */
	case UtilityData::PAUSE_ITEM::RESTART:
	{
		m_NextState = c_GAME::c_GAME_LOAD;
	}
	break;
		/* 退却 */
	case UtilityData::PAUSE_ITEM::RETURN_TITLE:
	{
		m_NextState = c_GAME::c_GO_RESULT_SCENE;
	}
	break;
		/* 設定 */
	case UtilityData::PAUSE_ITEM::CONFIG:
	{
	// 子を設定に変更し、ステートはそのままポーズ状態にし、ボタンはすべてOFFに
		this->SetInitChildState(pOwner, c_GAME::c_GAME_CONFIG);
		m_NextState = c_GAME::c_GAME_PAUSE;

		// UIオブジェクトを開放
		ReleaseToUIPool();

	}
	break;
		/* 操作マニュアル */
	case UtilityData::PAUSE_ITEM::MANUAL:
	{
	// 子をマニュアルに設定し、ステートはそのままポーズ状態にし、ボタンはすべてOFFに
		this->SetInitChildState(pOwner, c_GAME::c_GAME_MANUAL);
		m_NextState = c_GAME::c_GAME_PAUSE;

		// UIオブジェクトを開放
		ReleaseToUIPool();
	}

		break;
	case UtilityData::PAUSE_ITEM::NUM:
		break;
	default:
		break;
	}
}