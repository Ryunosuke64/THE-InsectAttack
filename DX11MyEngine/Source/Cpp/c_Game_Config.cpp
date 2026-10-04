#include "pch.h"
#include "GameScene_StateHeader.h"
#include "GameObjectManager.h"
#include "ResourceManager.h"
#include "SceneStateEnums.h"
#include "InputFactory.h"
#include "Component_SpriteRenderer.h"
#include "Component_ConfigSetting.h"
#include "Component_ButtonUI.h"

using namespace SceneStateEnums;
using namespace UtilityData;
using namespace VECTOR2;
using namespace VECTOR3;
using namespace VECTOR4;


//*---------------------------------------------------------------------------------------
//* @:c_Game_Config Class 
//*【?】開始
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
void c_Game_Config::OnEnter(SceneManager* pOwner)
{
	// 設定画面用オブジェクトの生成
	auto configObj = GIGA_Engine::Instantiate2D(std::make_shared<GameObject>(), true);
	configObj->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
	configObj->set_Tag("ConfigSetting");
	m_pConfigSetting = configObj->add_Component<ConfigSetting>();
	m_pConfigSetting.lock()->set_ItemStartPos(VEC2(700.0f,350.0f));
}


//*---------------------------------------------------------------------------------------
//* @:c_Game_Config Class 
//*【?】終了
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
void c_Game_Config::OnExit(SceneManager* pOwner)
{
	// 設定画面解放
	m_pConfigSetting.lock()->ReleaseToUIPool();
	m_pConfigSetting.lock()->get_OwnerObj().lock()->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_DELETE);	// 削除

	// 連続入力防止のため、1フレーム止める
	Master::m_pInputManager->StopInput(1);
}


//*---------------------------------------------------------------------------------------
//* @:c_Game_Config Class 
//*【?】更新
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
int c_Game_Config::Update(SceneManager* pOwner)
{
	// 右クリックでメインメニューへ戻る
	if (GetMouseClickDown(MOUSE_BUTTON_STATE::RIGHT))
	{
		// ****************************************************
		//				戻る際のSE再生
		// ****************************************************
		Master::m_pSoundManager->Play(SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::SYSTEM_BACK01));

		return c_GAME::c_GAME_PAUSE;
	}

	return c_GAME::c_GAME_CONFIG;
}


//*---------------------------------------------------------------------------------------
//* @:c_Game_Config Class 
//*【?】描画
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
void c_Game_Config::Draw(SceneManager* pOwner)
{
	float width = FLOAT_CAST(Master::m_pDataManager->get_ScreenWidth());
	float height = FLOAT_CAST(Master::m_pDataManager->get_ScreenHeight());
	Master::m_pDirectWriteManager->SetOutLine(3.0f, D2D1::ColorF(0.0f, 0.0f, 0.0f));
	Master::m_pDirectWriteManager->DrawStringToAligment("設定", VECTOR2::VEC2(0.0f, -250.0f), "White_40_STD", H_ALIGNMENT::CENTER, V_ALIGNMENT::CENTER, VEC2(width, height));
	Master::m_pDirectWriteManager->SetOutLine(0.0f);

}
