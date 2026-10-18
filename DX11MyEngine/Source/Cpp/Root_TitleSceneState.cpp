#include "pch.h"
#include "TitleScene_StateHeader.h"
#include "SceneManager.h"
#include "GameObjectManager.h"
#include "ResourceManager.h"
#include "SceneStateEnums.h"
#include "GameObject.h"
#include "Component_SpriteRenderer.h"
#include "InputFactory.h"

using namespace SceneStateEnums;
using namespace Tool::UV;
using namespace VECTOR2;
using namespace VECTOR3;
using namespace VECTOR4;


//*---------------------------------------------------------------------------------------
//* @:TitleSceneState Class 
//*【?】開始
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
void Root_TitleSceneState::OnEnter(SceneManager* pOwner)
{
	// 最初はロード
	this->SetInitChildState(pOwner, c_TITLE::c_TITLE_LOAD_PROCESS);

	float width = static_cast<float>(Master::m_pDataManager->get_ScreenWidth());
	float height = static_cast<float>(Master::m_pDataManager->get_ScreenHeight());

	//////////////////////////////////////////////////////////////////////////////////////////
	//
	//						タイトル画面UIスプライトの生成
	// 
	//////////////////////////////////////////////////////////////////////////////////////////
	
	UIData::SpriteUIData spriteData;
	UIData::RectTransformData spriteRect;
	// タイトルロゴのスプライト
	spriteData._imagePath = "Resource/Texture/Title/IA_TitleLogo.png";
	spriteData._tag = "TitleLogo_Sp";
	spriteData._layerRank = 100;
	spriteRect._size = VEC2(width, height);
	m_pTitleLogoObj = Master::m_pUIManager->GetSprite(*m_pRenderer, spriteRect, spriteData);

	// タイトル背景の緑のスプライト
	spriteData._imagePath = "Resource/Texture/Title/TitleBack.png";
	spriteData._tag = "TitleBack_Sp";
	spriteData._layerRank = 97;
	spriteRect._size = VEC2(width, height);
	m_pTitleBackObj = Master::m_pUIManager->GetSprite(*m_pRenderer, spriteRect, spriteData);

	// タイトル背景のグリッドスプライト 赤青
	spriteData._imagePath = "Resource/Texture/Title/2221833.png";
	spriteData._tag = "TitleBackGrid_Sp";
	spriteData._layerRank = 99;
	spriteRect._size = VEC2(0.0f, 0.0f);
	m_pTitleBackGridObj_Red = Master::m_pUIManager->GetSprite(*m_pRenderer, spriteRect, spriteData);
	m_pTitleBackGridObj_Blue = Master::m_pUIManager->GetSprite(*m_pRenderer, spriteRect, spriteData);


	// タイトル用のスプライトをオフに
	auto obj = Master::m_pGameObjectManager->get_ObjectByTag("MenuItem_Button_1");
	if (obj)obj->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
	obj = Master::m_pGameObjectManager->get_ObjectByTag("MenuItem_Button_2");
	if (obj)obj->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
	obj = Master::m_pGameObjectManager->get_ObjectByTag("MenuItem_Button_3");
	if (obj)obj->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
	obj = Master::m_pGameObjectManager->get_ObjectByTag("MenuItem_Button_4");
	if (obj)obj->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);


	Master::m_pDataManager->set_IsTitle(true);
}


//*---------------------------------------------------------------------------------------
//* @:TitleSceneState Class 
//*【?】終了
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
void Root_TitleSceneState::OnExit(SceneManager* pOwner)
{
    //Master::m_pGameObjectManager->clear_AllObject();
	// *****************************************************************************************
	// オブジェクトを非アクティブに（プールに返す）
	// *****************************************************************************************
	m_pTitleLogoObj->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
	m_pTitleBackObj->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
	m_pTitleBackGridObj_Red->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
	m_pTitleBackGridObj_Blue->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);

	m_pBackSprite.reset();
	m_pBackSprite2.reset();
	m_pBackSprite2_Rect.reset();
	m_pBackSprite3.reset();
	m_pBackSprite3_Rect.reset();

	// タイトル用のスプライトをオフに
	auto obj = Master::m_pGameObjectManager->get_ObjectByTag("MenuItem_Button_1");
	if (obj)obj->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
	obj = Master::m_pGameObjectManager->get_ObjectByTag("MenuItem_Button_2");
	if (obj)obj->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
	obj = Master::m_pGameObjectManager->get_ObjectByTag("MenuItem_Button_3");
	if (obj)obj->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
	obj = Master::m_pGameObjectManager->get_ObjectByTag("MenuItem_Button_4");
	if (obj)obj->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);

	// ****************************************************
	//				タイトルBGMの停止
	// ****************************************************
	Master::m_pSoundManager->StopBGM("bgm.hangeki");

	Master::m_pDataManager->set_IsTitle(false);
}


//*---------------------------------------------------------------------------------------
//* @:Root_TitleSceneState Class 
//*【?】更新
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
int Root_TitleSceneState::Update(SceneManager* pOwner)
{
	if (m_CrntChildStateID == -1)return -1;

	// 子ステートの更新
	int newState = m_pChildStateMap[m_CrntChildStateID]->Update(pOwner);
	
	// ゲームシーンへ
	if (newState == c_TITLE::c_GO_GAME_SCENE)
	{
		m_pChildStateMap[m_CrntChildStateID]->OnExit(pOwner);
		return SCENE_STATE::SCENE_STATE_GAME;
	}

	// 終了
	if (newState == c_TITLE::c_GO_EXIT)
	{
		m_pChildStateMap[m_CrntChildStateID]->OnExit(pOwner);
		pOwner->OnSceneClose();	// シーンを閉じる
		return SCENE_STATE::SCENE_STATE_TITLE;
	}

	// ステートが変更されていたら切り替え処理を行う
	// ※ c_GO_GAME_SCENE＆c_GO_EXIT以外
	if (newState != m_CrntChildStateID)
	{
		Master::m_pInputManager->StopInput(20);	// 入力を少しの間受け付けないようにする（シーン遷移の瞬間に入力されるのを防止）
		ChangeChildState(pOwner, newState);
	}

	return SCENE_STATE::SCENE_STATE_TITLE;
}


//*---------------------------------------------------------------------------------------
//* @:Root_TitleSceneState Class 
//*【?】描画
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
void Root_TitleSceneState::Draw(SceneManager* pOwner)
{
	if (m_CrntChildStateID == -1)return;

	float deltaTime = Master::m_pTimeManager->get_DeltaTime();

	//Master::m_pDirectWriteManager->DrawString("☆タイトルです",VECTOR2::VEC2(940, 540));
	constexpr float UV_SCROLL_SPEED = 0.001f;	// UVスクロール速度
	constexpr float COS_SPEED		= 1.0f;		// 揺れの速度
	constexpr float ROT_SIN_SPEED	= 5.0f;		// 回転用cosの揺れ速度
	constexpr float GRID_ROT		= 20.0f;	// グリッドの回転範囲（-20.0～20.0）
	constexpr float GRID_COLOR_A	= 0.4f;		// グリッドカラーのα

	// スプライト取得
	if (m_pBackSprite.expired())
	{
		if (m_pTitleBackObj && m_pTitleBackGridObj_Red && m_pTitleBackGridObj_Blue)
		{
			// 最奥背景（緑のやつ）のスプライト
			m_pBackSprite		= m_pTitleBackObj->get_Component<SpriteRenderer>();
			
			// 背景のグリッドスプライト（赤）
			m_pBackSprite2		= m_pTitleBackGridObj_Red->get_Component<SpriteRenderer>();
			m_pBackSprite2_Rect = m_pTitleBackGridObj_Red->get_RectTransform().lock();

			// 背景のグリッドスプライト（青）
			m_pBackSprite3		= m_pTitleBackGridObj_Blue->get_Component<SpriteRenderer>();
			m_pBackSprite3_Rect = m_pTitleBackGridObj_Blue->get_RectTransform().lock();
		}
	}
	else
	{
		auto backSprite = m_pBackSprite.lock();
		auto backSprite2 = m_pBackSprite2.lock();
		auto backSprite3 = m_pBackSprite3.lock();

		// 最奥背景（緑のやつ）のグリッドスプライトの挙動
		m_UVScroll.x += UV_SCROLL_SPEED;			// UVスクロール（左に動かす）
		backSprite->set_UVOffset(m_UVScroll);


		// 背景のグリッドスプライト（青＆赤）の挙動
		m_UVScroll2.x -= UV_SCROLL_SPEED;			// UVスクロール 最奥背景とは反対（右方向）に動かす 
		backSprite2->set_UVOffset(m_UVScroll2);	// 赤グリッド
		backSprite3->set_UVOffset(m_UVScroll);	// 青グリッド

		float cos_factor = cosf(m_UVScroll.x * COS_SPEED);	// 揺れ
		VEC2 rectPos = VEC2(1000.0f, 700.0f);				// 基準の位置

		float rot_sin_factor = (sinf(m_UVScroll.x * ROT_SIN_SPEED));	// 揺れ -1.0f～1.0f 
		VEC2 size = VEC2(4500.0f, 2700.0f);				// 基準サイズ
		//size.y += rot_cos_factor * 1000.0f;			// 縦幅を-1000.0f～1000.0fの幅で揺らす

		/* 赤グリッド背景 */
		auto backSprote2_Rect = m_pBackSprite2_Rect.lock();
		backSprote2_Rect->set_Pivot(VEC2(0.5f, 0.5f));	// 真ん中を中心に
		backSprote2_Rect->set_RotateToDeg(VEC3(0.0f, 0.0f, rot_sin_factor * GRID_ROT));
		backSprote2_Rect->set_Size(size.x, size.y);
		backSprote2_Rect->set_RectPosition(rectPos);

		/* 青グリッド背景 */
		auto backSprote3_Rect = m_pBackSprite3_Rect.lock();
		backSprote3_Rect->set_Pivot(VEC2(0.5f, 0.5f));	// 真ん中を中心に
		backSprote3_Rect->set_RotateToDeg(VEC3(0.0f, 0.0f, rot_sin_factor * GRID_ROT));
		backSprote3_Rect->set_Size(size.x, size.y);
		backSprote3_Rect->set_RectPosition(rectPos);

		float color = abs(cos_factor);
		backSprite2->set_Color(VEC4(color + 5.0f, color, color, GRID_COLOR_A));	// 色を赤っぽくする	αは最大0.3まで
		backSprite3->set_Color(VEC4(color, color, color, GRID_COLOR_A));			// 元の画像そのまま	αは最大0.3まで

	}

	// 子ステートの描画
	m_pChildStateMap[m_CrntChildStateID]->Draw(pOwner);


	Master::m_pDirectWriteManager->SetOutLine(3.0f, D2D1::ColorF(0.0f, 0.0f, 0.0f));
	Master::m_pDirectWriteManager->DrawString("マウス：選択　/　左クリック：決定　/　右クリック：戻る", VECTOR2::VEC2(700.0f, 1000.0f), "White_30_STD");
	Master::m_pDirectWriteManager->SetOutLine(0.0f);
}
