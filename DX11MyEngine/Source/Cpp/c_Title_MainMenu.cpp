#include "pch.h"
#include "TitleScene_StateHeader.h"
#include "ResourceManager.h"
#include "SceneStateEnums.h"
#include "Component_SpriteRenderer.h"
#include "Component_ButtonUI.h"
#include "GameObject.h"
#include "MeshFactory.h"
#include "InputFactory.h"

using namespace SceneStateEnums;
using namespace VECTOR2;
using namespace VECTOR3;
using namespace VECTOR4;
using namespace UtilityData;


//*---------------------------------------------------------------------------------------
//* @:c_Title_MainMenu Class 
//*【?】開始
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
void c_Title_MainMenu::OnEnter(SceneManager* pOwner)
{
	// 遷移先シーンの設定
	m_MenuItemInfoArray[0]._nextState = c_TITLE::c_TITLE_MISSION_SELECT;
	m_MenuItemInfoArray[1]._nextState = c_TITLE::c_TITLE_SOLDIER_SELECT;
	m_MenuItemInfoArray[2]._nextState = c_TITLE::c_TITLE_CONFIG;
	m_MenuItemInfoArray[3]._nextState = c_TITLE::c_GO_EXIT;

	ButtonUI* prevButton = nullptr;

	// メニュー項目のオブジェクトとコンポーネントの取得と設定
	for (int i = 0; i < static_cast<int>(TITLEMENU_ITEM::NUM); i++)
	{
		VEC2 pos = MENU_ITEM_START_POS;
		pos.y += i * ITEM_POS_Y_BETWEEN_DIST;	// Y座標を項目ごとにずらす

		UIData::RectTransformData rectTrans;
		UIData::ButtonUIData buttonData;
		buttonData._imagePath = "Resource/Texture/Title/Frame08.png";
		buttonData._text = g_TitleMenuItemNames[i];
		buttonData._layerRank = 105;
		buttonData._inputValidationState = UIData::STATE::PRESSED;
		buttonData._onClicFunction = [this, i, pOwner]() { 
			Button_OnClicFunction(pOwner, static_cast<TITLEMENU_ITEM>(i));
			};

		buttonData._textOffsetPos = VEC2(100.0f, 25.0f);
		rectTrans._size = MENU_ITEM_SIZE;
		rectTrans._pos = pos;
		m_pButtonsObjArray[i] = Master::m_pUIManager->GetButton(*m_pRenderer, rectTrans, buttonData);
		m_pButtonArray[i] = m_pButtonsObjArray[i]->get_Component<ButtonUI>();
		m_pButtonArray[i].lock()->set_Color(VEC4(10.0f, 10.0f, 0.0f, 1.0f), UIData::STATE::HIGH_LIGHTED);	// 選択されている状態で「黄色」に
		m_pButtonArray[i].lock()->set_Color(VEC4(5.0f, 5.0f, 0.0f, 1.0f), UIData::STATE::PRESSED);	
		m_pButtonArray[i].lock()->set_Color(VEC4(1.0f, 1.0f, 1.0f, 1.0f), UIData::STATE::DISABLED);			// 無効状態でも元の色のままにする	
		m_pMenuItemRectTransformArray[i] = m_pButtonsObjArray[i]->get_RectTransform();

		// Tweenでメニュー項目を下から上に登場させる
		VEC2 *currentPos = m_pMenuItemRectTransformArray[i].lock()->get_RectPositionPtr();
		float* tweenPosY = &currentPos->y;
		Master::m_pTweenManager->AddTween(tweenPosY, currentPos->y, currentPos->y - 650, 0.25f, Tool::TweenType::LINEAR);

		m_MenuItemInfoArray[i]._pos = pos;
		m_MenuItemInfoArray[i]._name = g_TitleMenuItemNames[i];
		m_MenuItemInfoArray[i]._type = static_cast<TITLEMENU_ITEM>(i);
	}

	{
		//for (int i = 0; i < static_cast<int>(TITLEMENU_ITEM::NUM); i++)
		//{
		//	auto currentBtn = m_pButtonArray[i].lock();
		//	if (!currentBtn) continue;

		//	ButtonUI* upBtn = nullptr;
		//	ButtonUI* downBtn = nullptr;
		//	ButtonUI* leftBtn = nullptr;  // 縦並びなので今回は空
		//	ButtonUI* rightBtn = nullptr; // 縦並びなので今回は空

		//	// --- 上のボタンの設定 ---
		//	if (i > 0) {
		//		upBtn = m_pButtonArray[i - 1].lock().get(); // 1つ上のボタン
		//	}
		//	else {
		//		// 一番上のボタンの場合、一番下へループさせる
		//		upBtn = m_pButtonArray[static_cast<int>(TITLEMENU_ITEM::NUM) - 1].lock().get();

		//		// ※ループさせずに行き止まりにする場合は何も代入しない（デフォルトの空ポインタのまま）
		//	}

		//	// --- 下のボタンの設定 ---
		//	if (i < static_cast<int>(TITLEMENU_ITEM::NUM) - 1) {
		//		downBtn = m_pButtonArray[i + 1].lock().get(); // 1つ下のボタン
		//	}
		//	else {
		//		// 一番下のボタンの場合、一番上へループさせる
		//		downBtn = m_pButtonArray[0].lock().get();
		//	}

		//	// コンポーネントにナビゲーション情報をセット
		//	currentBtn->set_Navigation(upBtn, downBtn, leftBtn, rightBtn);
		//}

		//// 画面を開いた直後は、一番上のボタン（i = 0）にフォーカスを当てる
		//if (!m_pButtonArray[0].expired())
		//{
		//	// UIManager側に実装したフォーカスセット関数を呼ぶ想定
		//	Master::m_pUIManager->SetFirstFocus(m_pButtonArray[0].lock().get());
		//}
	}

	// ****************************************************
	//				タイトルBGMの再生
	// ****************************************************
	Master::m_pSoundManager->PlayBGM(BGM_ID::BGM_HANGEKI);

	m_NextState = c_TITLE_MAIN_MENU;
	m_IsInit = true;

	m_MenuState = MENU_STATE::FADE_IN;

	m_CrntSelectItem = TITLEMENU_ITEM::MISSION_SELECT;
}


//*---------------------------------------------------------------------------------------
//* @:c_Title_MainMenu Class 
//*【?】終了
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
void c_Title_MainMenu::OnExit(SceneManager* pOwner)
{


}


//*---------------------------------------------------------------------------------------
//* @:c_Title_MainMenu Class 
//*【?】更新
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
int c_Title_MainMenu::Update(SceneManager* pOwner)
{
	POINT mousePos = Master::m_pInputManager->GetMousePos();	// マウス座標
	int i = 0;
	float deltaTime = Master::m_pTimeManager->get_DeltaTime();

	//
	// フェード状態を確認
	//
	switch (m_MenuState)
	{
		/* フェードなし */
	case MENU_STATE::NONE:
		break;

		/* フェードイン処理 */
	case c_Title_MainMenu::MENU_STATE::FADE_IN:
		FadeInMenuItems(deltaTime);
		//return c_TITLE_MAIN_MENU;
		break;

		/* フェードアウト処理 */
	case c_Title_MainMenu::MENU_STATE::FADE_OUT:
		FadeOutMenuItems(deltaTime);
		//return c_TITLE_MAIN_MENU;
		break;
	default:
		break;
	}

	//
	// ボタンの状態を確認
	//
	for (auto &button : m_pButtonArray)
	{
		UIData::STATE state = button.lock()->get_State();
		
		m_MenuItemInfoArray[i]._isHovered = false;

		switch (state)
		{
		case UIData::STATE::NORMAL:
			break;
			// ハイライト状態
		case UIData::STATE::HIGH_LIGHTED:
			if (m_CrntSelectItem != static_cast<TITLEMENU_ITEM>(i))
			{
			//	// ****************************************************
			//	//				カーソルが載った時のSE再生
			//	// ****************************************************
			//	Master::m_pSoundManager->Play(SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::SYSTEM_MOVING_CURSOR01));
			}
			m_CrntSelectItem = static_cast<TITLEMENU_ITEM>(i);
			m_MenuItemInfoArray[i]._isHovered = true;	// マウスが乗ってる
			break;
		case UIData::STATE::PRESSED:
			break;
		case UIData::STATE::SELECTED:
			break;
		case UIData::STATE::DISABLED:
			break;
		default:
			break;
		}
		
		i++;
	}

	for (auto& item : m_MenuItemInfoArray)
	{
		// スプライトの位置（文字の少し左側になるよう補正している）
		VEC2 spritePos;
		spritePos.x = item._pos.x - 100.0f;
		spritePos.y = item._pos.y;

		// マウスが乗ってるならずらす
		if (m_CrntSelectItem == item._type)
		{
			spritePos.x += MOUSE_HOVERTED_ITEM_SLIDEOFFSET;
		}

		//m_pMenuItemRectTransformArray[static_cast<int>(item._type)].lock()->set_RectPosition(VEC2(spritePos.x, spritePos.y));
		m_pMenuItemRectTransformArray[static_cast<int>(item._type)].lock()->set_Size(MENU_ITEM_SIZE.x, MENU_ITEM_SIZE.y);
	}

    return m_NextState;
}


//*---------------------------------------------------------------------------------------
//* @:c_Title_MainMenu Class 
//*【?】描画
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
void c_Title_MainMenu::Draw(SceneManager* pOwner)
{
	Master::m_pDirectWriteManager->SetOutLine(3.0f, D2D1::ColorF(0.0f, 0.0f, 0.0f));
	Master::m_pDirectWriteManager->DrawString("☆作戦司令本部", VECTOR2::VEC2(40.0f, 500.0f), "White_40_STD");
	Master::m_pDirectWriteManager->SetOutLine(0.0f);
}

//*---------------------------------------------------------------------------------------
//*【?】ボタンが押された際の処理
//*
//* [引数] 
//* *_pOwner : オーナー
//* _type    : 押されたボタンの種類
//* 
//* [返値] なし
//*----------------------------------------------------------------------------------------
void c_Title_MainMenu::Button_OnClicFunction(SceneManager* pOwner, UtilityData::TITLEMENU_ITEM _type)
{
	// 遷移先を設定
	m_NextState = m_MenuItemInfoArray[UINT_CAST(_type)]._nextState;

	// フェードアウト開始
	m_MenuState = MENU_STATE::FADE_OUT;	

	// Tweenでメニュー項目を下にスライドさせる
	for (int i = 0; i < static_cast<int>(TITLEMENU_ITEM::NUM); i++)
	{
		VEC2* currentPos = m_pMenuItemRectTransformArray[i].lock()->get_RectPositionPtr();
		float* tweenPosY = &currentPos->y;
		Master::m_pTweenManager->AddTween(tweenPosY, currentPos->y, currentPos->y + 650, 0.25f, Tool::TweenType::LINEAR);

		if (auto button = m_pButtonArray[i].lock())
		{
			button->set_IsInteractable(false);	// フェードアウト中はボタンを押せないようにする
		}
	}


	// *****************************************************************************************
	// 	フェードアウト終了後、オブジェクトを非アクティブに（プールに返す）
	// *****************************************************************************************
	Master::m_pTimeManager->AddTimer(FADE_OUT_DURATION,
		[this]()
		{
			for (int i = 0; i < static_cast<int>(TITLEMENU_ITEM::NUM); i++)
			{
				m_pButtonsObjArray[i]->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
			}
		}
	);
}



//*---------------------------------------------------------------------------------------
//*【?】フェードイン処理
//*
//* [引数] なし
//* [返値] なし
//*----------------------------------------------------------------------------------------
void c_Title_MainMenu::FadeInMenuItems(float _deltaTime)
{
	m_FadeTimer += _deltaTime;
	if(m_FadeTimer >= FADE_IN_DURATION)
	{
		m_FadeTimer = 0.0f;
		m_MenuState = MENU_STATE::NONE;
	}
}

//*---------------------------------------------------------------------------------------
//*【?】フェードアウト処理
//*
//* [引数] なし
//* [返値] なし
//*----------------------------------------------------------------------------------------
void c_Title_MainMenu::FadeOutMenuItems(float _deltaTime)
{
	m_FadeTimer += _deltaTime;

	if (m_FadeTimer >= FADE_OUT_DURATION)
	{
		m_FadeTimer = 0.0f;
		m_MenuState = MENU_STATE::NONE;
	}
}