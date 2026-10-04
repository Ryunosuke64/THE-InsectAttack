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
using namespace UtilityData;
using namespace SceneStateEnums;



//*---------------------------------------------------------------------------------------
//* @:c_MissionSelect_DifficultSelect Class 
//*【?】開始
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
void c_MissionSelect_DifficultSelect::OnEnter(SceneManager* pOwner)
{
	// メニュー項目のオブジェクトとコンポーネントの取得と設定
	for (int i = 0; i < UINT_CAST(DIFFICULTY_LEVEL::NUM); i++)
	{
		VEC2 pos = ITEM_START_POS;
		pos.y += i * ITEM_POS_Y_BETWEEN_DIST;	// Yを項目ごとにずらす

		UIData::RectTransformData rectTrans;
		UIData::ButtonUIData buttonData;
		buttonData._imagePath = "Resource/Texture/Title/Frame08.png";
		buttonData._text = g_DifficultyNames[i];
		buttonData._layerRank = 105;
		buttonData._inputValidationState = UIData::STATE::PRESSED;
		buttonData._onClicFunction = [this, i, pOwner]() {this->DifficultSelectButton_OnClicFunction(pOwner, i); };
		buttonData._textOffsetPos = VEC2(100.0f, 10.0f);
		rectTrans._size = ITEM_SIZE;
		rectTrans._pos = pos;
		// ボタン
		m_pButtonsObjArray[i] = Master::m_pUIManager->GetButton(*m_pRenderer, rectTrans, buttonData);
		m_pButtonArray[i] = m_pButtonsObjArray[i]->get_Component<ButtonUI>();
		m_pButtonArray[i].lock()->set_Color(VEC4(10.0f, 10.0f, 0.0f, 1.0f), UIData::STATE::HIGH_LIGHTED);	// 選択されている状態で「黄色」に
		m_pButtonArray[i].lock()->set_Color(VEC4(5.0f, 5.0f, 0.0f, 1.0f), UIData::STATE::PRESSED);
		m_pItemRectTransformArray[i] = m_pButtonsObjArray[i]->get_RectTransform();
	}


	// 難易度背景のスプライトのオブジェクトとコンポーネントの取得と設定
	UIData::RectTransformData rectData;
	rectData._size = VEC2(1000.0f, 550.0f);
	rectData._pos = VEC2(820.0f, 480.0f);
	UIData::SpriteUIData spriteData;
	spriteData._tag = "DifficultSelectBackSprite";
	spriteData._color = VEC4(0.7f, 0.7f, 0.7f, 1.0f);
	//spriteData._shaderType = SHADER_TYPE::FORWARD_UNLIT_UI_NOTEXTURE_SPRITE;
	spriteData._imagePath = "Resource/Texture/Title/Frame10.png";
	spriteData._layerRank = 99;
	m_pBackSpriteObj = Master::m_pUIManager->GetSprite(*m_pRenderer, rectData, spriteData);

	m_NextState = c_TITLE_DIFFICULT_SELECT;
}


//*---------------------------------------------------------------------------------------
//* @:c_MissionSelect_DifficultSelect Class 
//*【?】終了
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
void c_MissionSelect_DifficultSelect::OnExit(SceneManager* pOwner)
{
	// *****************************************************************************************
	//  オブジェクトを非アクティブに（プールに返す）
	// *****************************************************************************************
	m_pBackSpriteObj->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);

	for (int i = 0; i < UINT_CAST(DIFFICULTY_LEVEL::NUM); i++)
	{
		m_pButtonsObjArray[i]->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
	}

	// 入力止める
	Master::m_pInputManager->StopInput(5);
}


//*---------------------------------------------------------------------------------------
//* @:c_MissionSelect_DifficultSelect Class 
//*【?】更新
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
int c_MissionSelect_DifficultSelect::Update(SceneManager* pOwner)
{
	// 右クリックでミッション選択へ戻る
	if (GetMouseClickDown(MOUSE_BUTTON_STATE::RIGHT))
	{
		// ****************************************************
		//				戻る際のSE再生
		// ****************************************************
		Master::m_pSoundManager->Play(SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::SYSTEM_BACK01));


		return c_TITLE::c_TITLE_MISSION_SELECT;
	}
	int i = 0;
	for (auto& button : m_pButtonArray)
	{
		UIData::STATE state = button.lock()->get_State();
		switch (state)
		{
		case UIData::STATE::NORMAL:
			break;
		case UIData::STATE::HIGH_LIGHTED:
			if (m_CrntSelectItem != static_cast<DIFFICULTY_LEVEL>(i))
			{
				// ****************************************************
				//				カーソルが載った時のSE再生
				// ****************************************************
				Master::m_pSoundManager->Play(SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::SYSTEM_MOVING_CURSOR01));
			}
			m_CrntSelectItem = static_cast<DIFFICULTY_LEVEL>(i);
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
//* @:c_MissionSelect_DifficultSelect Class 
//*【?】描画
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
void c_MissionSelect_DifficultSelect::Draw(SceneManager* pOwner)
{
	Master::m_pDirectWriteManager->SetOutLine(3.0f, D2D1::ColorF(0.0f, 0.0f, 0.0f));
	Master::m_pDirectWriteManager->DrawString("☆難易度選択", VECTOR2::VEC2(600.0f, 500.0f), "White_40_STD");
	Master::m_pDirectWriteManager->SetOutLine(0.0f);


	// 難易度説明
	VEC2 textPos = VEC2(1350.0f, 550.0f);
	textPos.y = ITEM_START_POS.y + ITEM_POS_Y_BETWEEN_DIST * UINT_CAST(m_CrntSelectItem);	// 項目のY座標に合わせる
	Master::m_pDirectWriteManager->SetOutLine(2.0f, D2D1::ColorF(0.0f, 0.0f, 0.0f));
	Master::m_pDirectWriteManager->DrawString(g_DifficultyDescriptions[UINT_CAST(m_CrntSelectItem)], textPos, "White_20_STD");
	Master::m_pDirectWriteManager->SetOutLine(0.0f);
}

//*---------------------------------------------------------------------------------------
//* @:c_MissionSelect_DifficultSelect Class 
//*【?】難易度選択ボタンが押された際の処理
//* 引数：pOwner
//* 引数：_difficultNumber
//* 返値：なし
//*----------------------------------------------------------------------------------------
void c_MissionSelect_DifficultSelect::DifficultSelectButton_OnClicFunction(SceneManager* _pOwner, int _difficultNumber)
{
	m_NextState = c_GO_GAME_SCENE;	// ゲームシーンへ
	Master::m_pDataManager->set_SelectDifficultyLevel(static_cast<DIFFICULTY_LEVEL>(_difficultNumber));
}
