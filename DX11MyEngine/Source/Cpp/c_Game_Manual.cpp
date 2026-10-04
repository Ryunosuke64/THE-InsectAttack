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
//* @:c_Game_Manual Class 
//*【?】開始
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
void c_Game_Manual::OnEnter(SceneManager* pOwner)
{
	// 連続入力防止のため、1フレーム止める
	Master::m_pInputManager->StopInput(1);

	Master::m_pDataManager->set_IsPause(true);

	//*****************************************************************************************
	//						最奥背景スプライト
	//*****************************************************************************************
	UIData::SpriteUIData spriteData;
	UIData::RectTransformData rectTrans;
	spriteData._imagePath = "Resource/Texture/UI/Manual.png";
	spriteData._layerRank = 100;
	spriteData._tag = "ManualSprite";
	rectTrans._size = VEC2(0.0f, MANUAL_SPRITE_SIZE.y);	// Xサイズを広げながら出現させたいので、yはそのまま
	rectTrans._pos = MANUAL_SPRITE_POS;
	// 画面の中央を中心に
	rectTrans._anchorMin = VEC2(0.5f);	
	rectTrans._anchorMax = VEC2(0.5f);
	rectTrans._pivot = VEC2(0.5f);
	m_pManualSpriteObj = Master::m_pUIManager->GetSprite(*m_pRenderer, rectTrans, spriteData);

	m_Timer = 0.0f;
}


//*---------------------------------------------------------------------------------------
//* @:c_Game_Manual Class 
//*【?】終了
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
void c_Game_Manual::OnExit(SceneManager* pOwner)
{
	m_pManualSpriteObj->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);

	// 連続入力防止のため、1フレーム止める
	Master::m_pInputManager->StopInput(1);
}


//*---------------------------------------------------------------------------------------
//* @:c_Game_Manual Class 
//*【?】更新
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
int c_Game_Manual::Update(SceneManager* pOwner)
{
	float deltaTime = Master::m_pTimeManager->get_DeltaTime();
	m_Timer += deltaTime;

	/* 左右に開くような感じに出現 */
	float t = m_Timer / MANUAL_SPRITE_SPWAN_DURATION;
	if (t <= 1.0f)
	{
		float ease = Tool::Easing::EaseOutSin(t);

		auto rect = m_pManualSpriteObj->get_RectTransform().lock();
		VEC2 crntSize = rect->get_SizeDelta();
		VEC2 newSize = VEC2::Lerp(crntSize, MANUAL_SPRITE_SIZE, ease);
		rect->set_Size(newSize);
	}

	// 右クリックでポーズへ戻る
	if (GetMouseClickDown(MOUSE_BUTTON_STATE::RIGHT) || GetInputDown(GAME_CONFIG::PAUSE))
	{
		// ****************************************************
		//				戻る際のSE再生
		// ****************************************************
		Master::m_pSoundManager->Play(SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::SYSTEM_BACK01));

		return c_GAME::c_GAME_PAUSE;
	}

	return c_GAME::c_GAME_MANUAL;
}


//*---------------------------------------------------------------------------------------
//* @:c_Game_Manual Class 
//*【?】描画
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
void c_Game_Manual::Draw(SceneManager* pOwner)
{
	float width = FLOAT_CAST(Master::m_pDataManager->get_ScreenWidth());
	float height = FLOAT_CAST(Master::m_pDataManager->get_ScreenHeight());

	Master::m_pDirectWriteManager->SetOutLine(3.0f, D2D1::ColorF(0.0f, 0.0f, 0.0f));
	Master::m_pDirectWriteManager->DrawStringToAligment("操作マニュアル", VECTOR2::VEC2(0.0f, -350.0f), "White_40_STD", H_ALIGNMENT::CENTER, V_ALIGNMENT::CENTER, VEC2(width, height));
	Master::m_pDirectWriteManager->SetOutLine(0.0f);
}
