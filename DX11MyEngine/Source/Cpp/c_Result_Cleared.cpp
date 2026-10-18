#include "pch.h"
#include "ResultScene_StateHeader.h"
#include "ResourceManager.h"
#include "SceneStateEnums.h"
#include "InputFactory.h"
#include "Component_SpriteRenderer.h"

using namespace SceneStateEnums;
using namespace VECTOR2;
using namespace VECTOR3;
using namespace VECTOR4;
//*---------------------------------------------------------------------------------------
//* @:c_Result_Cleared Class 
//*【?】開始
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
void c_Result_Cleared::OnEnter(SceneManager* pOwner)
{
	m_ElapsedTime = 0.0f;

	// ****************************************************
	//				クリアBGMの再生
	// ****************************************************
	Master::m_pSoundManager->PlayBGM("bgm.victory_01", false);


	// クリアテキスト用スプライトを作る **********************************************
	UIData::RectTransformData rectData;
	UIData::SpriteUIData spriteData;
	rectData._size = VEC2(6000.0f, 6000.0f);	//縮小フェードなので、初期は大きく
	rectData._pivot = VEC2(0.5f, 0.5f);
	rectData._anchorMax = VEC2(0.5f, 0.5f);
	rectData._anchorMin = VEC2(0.5f, 0.5f);
	rectData._pos = VEC2(0.0f, 0.0f);
	spriteData._tag = "ClearedTextSprite";
	spriteData._imagePath = "Resource/Texture/UI/MissionCleared.png";
	spriteData._layerRank = 999;
	spriteData._color = VEC4(1.0f, 1.0f, 1.0f, 0.0f);
	m_pClearedTextSpriteObj = Master::m_pUIManager->GetSprite(*m_pRenderer, rectData, spriteData);
	m_pClearedTextSpriteRenderer = m_pClearedTextSpriteObj->get_Component<SpriteRenderer>();


	// 白ワイプ用スプライトを作る **********************************************
	rectData._size = VEC2(1920.0f, 1080.0f);
	rectData._pivot = VEC2(0.5f, 0.5f);
	rectData._anchorMax = VEC2(0.5f, 0.5f);
	rectData._anchorMin = VEC2(0.5f, 0.5f);
	spriteData._tag = "WipeSprite";
	spriteData._layerRank = 998;
	spriteData._imagePath = "";
	spriteData._color = VEC4(1.0f, 1.0f, 1.0f, 0.0f);
	spriteData._shaderType = SHADER_TYPE::FORWARD_UNLIT_UI_NOTEXTURE_SPRITE;
	m_pWhiteWipeSpriteObj = Master::m_pUIManager->GetSprite(*m_pRenderer, rectData, spriteData);
	m_pWhiteWipeSpriteRenderer = m_pWhiteWipeSpriteObj->get_Component<SpriteRenderer>();

	// 黒ワイプ用スプライトを作る **********************************************
	rectData._size = VEC2(1920.0f, 1080.0f);
	rectData._pivot = VEC2(0.5f, 0.5f);
	rectData._anchorMax = VEC2(0.5f, 0.5f);
	rectData._anchorMin = VEC2(0.5f, 0.5f);
	spriteData._tag = "WipeSprite";
	spriteData._layerRank = 1000;
	spriteData._color = VEC4(0.1f, 0.1f, 0.1f, 0.0f);
	spriteData._shaderType = SHADER_TYPE::FORWARD_UNLIT_UI_NOTEXTURE_SPRITE;
	m_pBlackWipeSpriteObj = Master::m_pUIManager->GetSprite(*m_pRenderer, rectData, spriteData);
	m_pBlackWipeSpriteRenderer = m_pBlackWipeSpriteObj->get_Component<SpriteRenderer>();

}


//*---------------------------------------------------------------------------------------
//* @:c_Result_Cleared Class 
//*【?】終了
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
void c_Result_Cleared::OnExit(SceneManager* pOwner)
{
	// スプライトをプールへ返す
	m_pWhiteWipeSpriteObj->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
	m_pClearedTextSpriteObj->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
	m_pBlackWipeSpriteObj->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
}


//*---------------------------------------------------------------------------------------
//* @:c_Result_Cleared Class 
//*【?】更新
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
int c_Result_Cleared::Update(SceneManager* pOwner)
{
	float deltaTime = Master::m_pTimeManager->get_DeltaTime();
	m_ElapsedTime += deltaTime;

	// テキストのフェードイン
	float text_t = m_ElapsedTime / TEXT_FADE_IN_DURATION;
	if (text_t <= 1.0f) {
		auto rect = m_pClearedTextSpriteObj->get_RectTransform().lock();
		auto textSprite = m_pClearedTextSpriteRenderer.lock();
		textSprite->set_Color(VEC4(1.0f, 1.0f, 1.0f, 1.0));

		VEC2 crntSize = rect->get_SizeDelta();
		rect->set_Size(VEC2::Lerp(crntSize, VEC2(1280.0f, 256.0f), text_t));
	}
	else
	{
		// ワイプのフェードイン
		if (FADE_OUT_START_DELAY < (m_ElapsedTime - TEXT_FADE_IN_DURATION))
		{
			// テキストとワイプのフェードに入るまでの時間の差分を打ち消す
			float whiteWipeElapsed = m_ElapsedTime - FADE_OUT_START_DELAY - TEXT_FADE_IN_DURATION;

			// フェードイン
			float wipe_t = whiteWipeElapsed / WHITE_FADE_DURATION;
			if (wipe_t <= 1.0f)
			{
				float ease = Tool::Easing::EaseInCubic(wipe_t);
				auto wipeSprite = m_pWhiteWipeSpriteRenderer.lock();
				wipeSprite->set_Color(VEC4(1.0f, 1.0f, 1.0f, ease));
			}
			//=========================================================================================
			//
			//						ワイプフェードが終了したら、
			//				画面をブラックアウトさせる用のスプライトを
			//					フェードインさせて、タイトルへ戻る
			//
			//=========================================================================================
			else
			{
				float blackWipeElapsed = m_ElapsedTime - FADE_OUT_START_DELAY - TEXT_FADE_IN_DURATION - WHITE_FADE_DURATION;

				// フェードイン
				wipe_t = blackWipeElapsed / BLACK_FADE_DURATION;

				if (wipe_t <= 1.0f)
				{
					float ease = Tool::Easing::EaseInSine(wipe_t);
					auto wipeSprite = m_pBlackWipeSpriteRenderer.lock();
					wipeSprite->set_Color(VEC4(0.0f, 0.0f, 0.0f, ease));
				}
				else
				{
					return c_RESULT::c_GO_TITLE_SCENE;
				}
			}
		}
	}

	return c_RESULT::c_CLEARED;
}


//*---------------------------------------------------------------------------------------
//* @:c_Result_Cleared Class 
//*【?】描画
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
void c_Result_Cleared::Draw(SceneManager* pOwner)
{
	//Master::m_pDirectWriteManager->DrawString("クリアおめでとう！", VECTOR2::VEC2(740, 540), "White_50_STD");
	//Master::m_pDirectWriteManager->DrawString("Spaceを1秒以上長押しでタイトルに戻ります", VECTOR2::VEC2(540, 840), "White_40_STD");
}
