#include "pch.h"
#include "TitleScene_StateHeader.h"
#include "GameObjectManager.h"
#include "ResourceManager.h"
#include "Component_ButtonUI.h"
#include "InputFactory.h"
#include "SceneStateEnums.h"
#include "GameObject.h"

using namespace VECTOR2;
using namespace VECTOR3;
using namespace VECTOR4;

/// <summary>
/// ミッション名
/// </summary>
static constexpr const char* g_MissionNames[MISSION_NUM]=
{
	"0.デバッグステージ",
	"1.巨大生物襲来",
};

using namespace SceneStateEnums;
//*---------------------------------------------------------------------------------------
//* @:c_Title_MissionSelect Class 
//*【?】開始
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
void c_Title_MissionSelect::OnEnter(SceneManager *pOwner)
{
	// 最初は空
	this->SetInitChildState(pOwner, c_TITLE::c_TITLE_NONE);

	m_NextState = c_TITLE_MISSION_SELECT;

	// メニュー項目のオブジェクトとコンポーネントの取得と設定
	for (int i = 0; i < static_cast<int>(MISSION_NUM); i++)
	{
		VEC2 pos = MISSION_ITEM_START_POS;	
		pos.y += i * ITEM_POS_Y_BETWEEN_DIST;	// 項目ごとにY座標ずらす

		UIData::RectTransformData rectTrans;
		UIData::ButtonUIData buttonData;
		buttonData._imagePath = "Resource/Texture/Title/Frame08.png";
		buttonData._text = g_MissionNames[i];
		buttonData._layerRank = 105;
		buttonData._inputValidationState = UIData::STATE::PRESSED;
		buttonData._onClicFunction = [this, i, pOwner]() { 	this->MissionSelectButton_OnClicFunction(pOwner); };	// 難易度選択へ
		buttonData._textOffsetPos = VEC2(100.0f, 25.0f);
		rectTrans._size = MISSION_ITEM_SIZE;	// サイズ
		rectTrans._pos = pos;					// 位置
		m_pButtonsObjArray[i] = Master::m_pUIManager->GetButton(*m_pRenderer, rectTrans, buttonData);
		m_pButtonArray[i] = m_pButtonsObjArray[i]->get_Component<ButtonUI>();
		m_pButtonArray[i].lock()->set_Color(VEC4(10.0f, 10.0f, 0.0f, 1.0f), UIData::STATE::HIGH_LIGHTED);	// 選択されている状態で「黄色」に
		m_pButtonArray[i].lock()->set_Color(VEC4(5.0f, 5.0f, 0.0f, 1.0f), UIData::STATE::PRESSED);
		m_pMenuItemRectTransformArray[i] = m_pButtonsObjArray[i]->get_RectTransform();

		m_ItemInfoArray[i]._pos = pos;
		m_ItemInfoArray[i]._name = g_MissionNames[i];
	}


	m_CrntSelectItem = -1;

	m_IsInit = true;

}


//*---------------------------------------------------------------------------------------
//* @:c_Title_MissionSelect Class 
//*【?】終了
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
void c_Title_MissionSelect::OnExit(SceneManager *pOwner)
{
	// *****************************************************************************************
	// 	// オブジェクトを非アクティブに（プールに返す）
	// *****************************************************************************************
	for (int i = 0; i < MISSION_NUM; i++)
	{
		m_pButtonArray[i].lock()->set_IsInteractable(true);
		m_pButtonsObjArray[i]->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
	}
}


//*---------------------------------------------------------------------------------------
//* @:c_Title_MissionSelect Class 
//*【?】更新
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
int c_Title_MissionSelect::Update(SceneManager *pOwner)
{

	//////////////////////////////////////////////////////////////////////////////////////////
	//
	//						子ステートの処理
	// 
	//////////////////////////////////////////////////////////////////////////////////////////
	if (m_CrntChildStateID != -1) {
		// 子ステートの更新
		int newState = m_pChildStateMap[m_CrntChildStateID]->Update(pOwner);

		// ゲームシーンへ
		if (newState == c_TITLE::c_GO_GAME_SCENE)
		{
			m_pChildStateMap[m_CrntChildStateID]->OnExit(pOwner);	// 子ステートの終了
			return c_TITLE::c_GO_GAME_SCENE;
		}
		// ミッション選択のままであれば、子をNONEに戻す
		else if (newState == c_TITLE::c_TITLE_MISSION_SELECT)
		{
			m_pChildStateMap[m_CrntChildStateID]->OnExit(pOwner);	// 子ステートの終了
			this->SetInitChildState(pOwner, c_TITLE::c_TITLE_NONE);

			// ボタンも有効状態に
			for (int i = 0; i < static_cast<int>(MISSION_NUM); i++)
			{
				m_pButtonArray[i].lock()->set_IsInteractable(true);
			}
		}
	}



	//////////////////////////////////////////////////////////////////////////////////////////
	//
	//						ミッション選択ステートの処理
	// 
	//////////////////////////////////////////////////////////////////////////////////////////
	// 右クリックでメインメニューへ戻る
	if (GetMouseClickDown(MOUSE_BUTTON_STATE::RIGHT))
	{
		// ****************************************************
		//				戻る際のSE再生
		// ****************************************************
		Master::m_pSoundManager->Play(SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::SYSTEM_BACK01));

		return c_TITLE::c_TITLE_MAIN_MENU;
	}	

	//m_NextState = c_TITLE_MISSION_SELECT;

	int i = 0;
	for (auto &button : m_pButtonArray)
	{
		UIData::STATE state = button.lock()->get_State();;

		//m_MissionItems[i]._isHovered = false;

		switch (state)
		{
		case UIData::STATE::NORMAL:
			break;
		case UIData::STATE::HIGH_LIGHTED:	// マウスが乗ってる

			//// 前回と値が違うなら音を鳴らす
			//if (m_CrntSelectItem != i) {
			//	// ****************************************************
			//	//				カーソルが載った時のSE再生
			//	// ****************************************************
			//	Master::m_pSoundManager->Play(SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::SYSTEM_MOVING_CURSOR01));
			//}
			m_CrntSelectItem = i;
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



	return m_NextState;
}


//*---------------------------------------------------------------------------------------
//* @:c_Title_MissionSelect Class 
//*【?】描画
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
void c_Title_MissionSelect::Draw(SceneManager* pOwner)
{
	int i = 0;

	for (auto& item : m_ItemInfoArray)
	{
		// 項目の位置
		VEC2 menuItemPos = item._pos;

		// スプライトの位置（文字の少し左側になるよう補正している）
		VEC2 spritePos;
		spritePos.x = menuItemPos.x - 100.0f;
		spritePos.y = menuItemPos.y;


		// マウスが乗ってるならずらす
		if (m_CrntSelectItem == i)
		{
			spritePos.x += MOUSE_HOVERTED_ITEM_SLIDEOFFSET;
		}

		m_pMenuItemRectTransformArray[i].lock()->set_RectPosition(VEC2(spritePos.x, spritePos.y));
		m_pMenuItemRectTransformArray[i].lock()->set_Size(500.0f, 100.0f);

		i++;
	}

	Master::m_pDirectWriteManager->SetOutLine(3.0f, D2D1::ColorF(0.0f, 0.0f, 0.0f));
	Master::m_pDirectWriteManager->DrawString("☆ミッション選択", VECTOR2::VEC2(40.0f, 500.0f), "White_40_STD");
	Master::m_pDirectWriteManager->SetOutLine(0.0f);


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
//* @:c_Title_MissionSelect Class 
//*【?】ミッション選択ボタンが押された際の処理
//* 引数：pOwner
//* 返値：なし
//*----------------------------------------------------------------------------------------
void c_Title_MissionSelect::MissionSelectButton_OnClicFunction(SceneManager* pOwner)
{
	// 子ステートを難易度選択へ
	this->SetInitChildState(pOwner, c_TITLE::c_TITLE_DIFFICULT_SELECT);

	// ボタンを無効状態に
	for (int i = 0; i < static_cast<int>(MISSION_NUM); i++)
	{
		m_pButtonArray[i].lock()->set_IsInteractable(false);
	}
}