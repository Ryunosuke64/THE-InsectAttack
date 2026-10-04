#include "pch.h"
#include "TitleScene_StateHeader.h"
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
//* @:c_Title_Config Class 
//*【?】開始
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
void c_Title_Config::OnEnter(SceneManager *pOwner)
{

	// 設定画面用オブジェクトの生成
	auto configObj = GIGA_Engine::Instantiate2D(std::make_shared<GameObject>(), true);
	configObj->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
	configObj->set_Tag("ConfigSetting");
	m_pConfigSetting = configObj->add_Component<ConfigSetting>();
}


//*---------------------------------------------------------------------------------------
//* @:c_Title_Config Class 
//*【?】終了
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
void c_Title_Config::OnExit(SceneManager* pOwner)
{
	// 設定画面解放
	m_pConfigSetting.lock()->ReleaseToUIPool();
	m_pConfigSetting.lock()->get_OwnerObj().lock()->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_DELETE);	// 削除

}


//*---------------------------------------------------------------------------------------
//* @:c_Title_Config Class 
//*【?】更新
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
int c_Title_Config::Update(SceneManager* pOwner)
{
	// 右クリックでメインメニューへ戻る
	if (GetMouseClickDown(MOUSE_BUTTON_STATE::RIGHT))
	{
		// ****************************************************
		//				戻る際のSE再生
		// ****************************************************
		Master::m_pSoundManager->Play(SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::SYSTEM_BACK01));

		return c_TITLE::c_TITLE_MAIN_MENU;
	}

	return c_TITLE::c_TITLE_CONFIG;
}


//*---------------------------------------------------------------------------------------
//* @:c_Title_Config Class 
//*【?】描画
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
void c_Title_Config::Draw(SceneManager* pOwner)
{
	Master::m_pDirectWriteManager->SetOutLine(3.0f, D2D1::ColorF(0.0f, 0.0f, 0.0f));
	Master::m_pDirectWriteManager->DrawString("☆設定", VECTOR2::VEC2(40.0f, 500.0f), "White_40_STD");
	Master::m_pDirectWriteManager->SetOutLine(0.0f);

}
