#include "pch.h"
#include "TitleScene_StateHeader.h"
#include "GameObjectManager.h"
#include "ResourceManager.h"
#include "SceneStateEnums.h"
#include "Component_ButtonUI.h"
#include "InputFactory.h"
#include "GameObject.h"

using namespace Tool;
using namespace VECTOR2;
using namespace VECTOR3;
using namespace VECTOR4;
using namespace SceneStateEnums;
using namespace UtilityData;

//*---------------------------------------------------------------------------------------
//* @:c_Title_SoldierSelect Class 
//*【?】開始
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
void c_Title_SoldierSelect::OnEnter(SceneManager *pOwner)
{
	m_NextState = c_TITLE_SOLDIER_SELECT;

	// メニュー項目のボタンオブジェクトとコンポーネントの取得と設定
	for (int i = 0; i < INT_CAST(SOLDIER_TYPE::NUM); i++)
	{
		VEC2 pos = SOLDIER_ITEM_START_POS;
		pos.y += i * ITEM_POS_Y_BETWEEN_DIST;	// 項目ごとにY座標をずらす

		UIData::RectTransformData rectTrans;
		UIData::ButtonUIData buttonData;
		buttonData._imagePath = "Resource/Texture/Title/Frame07.png";
		buttonData._text = g_SoldierNames[i];
		buttonData._layerRank = 105;
		buttonData._inputValidationState = UIData::STATE::SELECTED;
		buttonData._textOffsetPos = VEC2(80.0f, 25.0f);
		buttonData._onClicFunction = // 武器の選択
			[this, i]() {
				Master::m_pDataManager->set_SelectWeaponID(m_CrntSelectItem * 2, 0);
				Master::m_pDataManager->set_SelectWeaponID(m_CrntSelectItem * 2 + 1, 1);
				m_DecisionTextDrawTimer = DECISON_TEXT_DRAW_DURATION;	// 装備決定のテキストを表示するためのカウンターをセット
				m_DecisionSoldierTypeIndex = m_CrntSelectItem;			// 決定した兵科のインデックスをセット
			};
		rectTrans._size = SOLDIER_ITEM_SIZE;
		rectTrans._pos = pos;

		// ボタン
		m_pButtonsObjArray[i] = Master::m_pUIManager->GetButton(*m_pRenderer, rectTrans, buttonData);		// ボタンオブジェクトをプールから取得
		m_pButtonArray[i] = m_pButtonsObjArray[i]->get_Component<ButtonUI>();								// ボタンコンポーネント取得
		m_pButtonArray[i].lock()->set_Color(VEC4(10.0f, 10.0f, 0.0f, 1.0f), UIData::STATE::HIGH_LIGHTED);	// 選択されている状態で「黄色」に
		m_pButtonArray[i].lock()->set_Color(VEC4(5.0f, 5.0f, 0.0f, 1.0f), UIData::STATE::PRESSED);
		m_pButtonArray[i].lock()->set_Color(VEC4(1.0f, 1.0f, 1.0f, 1.0f), UIData::STATE::DISABLED);			// 無効状態でも元の色のままにする
		m_pMenuItemRectTransformArray[i] = m_pButtonsObjArray[i]->get_RectTransform();						// ボタンのレクトトランスフォーム取得


		// Tweenでメニュー項目を下から上に登場させる
		VEC2* currentPos = m_pMenuItemRectTransformArray[i].lock()->get_RectPositionPtr();
		float* tweenPosY = &currentPos->y;
		Master::m_pTweenManager->AddTween(tweenPosY, currentPos->y, currentPos->y + 500, 0.25f, Tool::TweenType::LINEAR);


		m_ItemInfoArray[i]._pos = pos;
		m_ItemInfoArray[i]._name = g_SoldierNames[i];
	}


	// 武器説明背景のスプライトのオブジェクトとコンポーネントの取得と設定
	UIData::RectTransformData rectData;
	rectData._size = VEC2(1000.0f, 550.0f);
	rectData._pos = VEC2(820.0f, 480.0f);
	UIData::SpriteUIData spriteData;
	spriteData._tag = "WeaponDescriptionbackSprite";
	spriteData._color = VEC4(0.7f, 0.7f, 0.7f, 1.0f);
	//spriteData._shaderType = SHADER_TYPE::FORWARD_UNLIT_UI_NOTEXTURE_SPRITE;
	spriteData._imagePath = "Resource/Texture/Title/Frame10.png";
	spriteData._layerRank = 110;
	m_pWeaponDescriptionBackSpriteObj = Master::m_pUIManager->GetSprite(*m_pRenderer, rectData, spriteData);


	m_CrntSelectItem = 0;

	m_IsInit = true;
}


//*---------------------------------------------------------------------------------------
//* @:c_Title_SoldierSelect Class 
//*【?】終了
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
void c_Title_SoldierSelect::OnExit(SceneManager *pOwner)
{
	// *****************************************************************************************
	// 	// オブジェクトを非アクティブに（プールに返す）
	// *****************************************************************************************
	for (int i = 0; i < UINT_CAST(SOLDIER_TYPE::NUM); i++)
	{
		m_pButtonsObjArray[i]->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
	}

	m_pWeaponDescriptionBackSpriteObj->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);

	m_DecisionTextDrawTimer = 0.0f;
}


//*---------------------------------------------------------------------------------------
//* @:c_Title_SoldierSelect Class 
//*【?】更新
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
int c_Title_SoldierSelect::Update(SceneManager *pOwner)
{
	// 右クリックでメインメニューへ戻る
	if (GetMouseClickDown(MOUSE_BUTTON_STATE::RIGHT))
	{
		return c_TITLE::c_TITLE_MAIN_MENU;
	}

	POINT mousePos = Master::m_pInputManager->GetMousePos();	// マウス座標

	//m_NextState = c_TITLE_SOLDIER_SELECT;

	int i = 0;
	for (auto &button : m_pButtonArray)
	{
		UIData::STATE state = button.lock()->get_State();;
			

		m_ItemInfoArray[i]._isHovered = false;

		switch (state)
		{
		case UIData::STATE::NORMAL:
			break;
		case UIData::STATE::HIGH_LIGHTED:
		{
			if (m_CrntSelectItem != i) {
				// ****************************************************
				//				カーソルが載った時のSE再生
				// ****************************************************
				Master::m_pSoundManager->Play(SOUND_TYPE::SE, SOUND_ID_TO_INT(SOUND_ID::SYSTEM_MOVING_CURSOR01));
			}
			m_ItemInfoArray[i]._isHovered = true;	// マウスが乗ってる
			m_CrntSelectItem = i;					// なんの項目に乗ったか保持
		}
			break;
		case UIData::STATE::PRESSED:
			m_ItemInfoArray[i]._isHovered = true;	
			m_CrntSelectItem = i;

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
	//return c_TITLE::c_TITLE_SOLDIER_SELECT;
}


//*---------------------------------------------------------------------------------------
//* @:c_Title_SoldierSelect Class 
//*【?】描画
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
void c_Title_SoldierSelect::Draw(SceneManager *pOwner)
{
	int i = 0;

	float deltaTime = Master::m_pTimeManager->get_DeltaTime();

	for (auto& item : m_ItemInfoArray)
	{
		// 項目の位置
		VEC2 menuItemPos = item._pos;

		// スプライトの位置（文字の少し左側になるよう補正している）
		VEC2 spritePos;
		spritePos.x = menuItemPos.x - 100.0f;
		spritePos.y = menuItemPos.y;


		// マウスが乗っている項目は少しずらす
		if (m_CrntSelectItem == i)
		{
			// ずらす
			menuItemPos.x += MOUSE_HOVERTED_ITEM_SLIDEOFFSET;
			spritePos.x += MOUSE_HOVERTED_ITEM_SLIDEOFFSET;
		}

		//m_pMenuItemRectTransformArray[i].lock()->set_RectPosition(VEC2(spritePos.x, spritePos.y));
											  
		// 文字表示
		//Master::m_pDirectWriteManager->DrawString(item._name, menuItemPos, "White_40_STD");

		i++;
	}


	// 描画の実行（現在選択中の武器）
	auto weaponData_1 = static_cast<const WeaponData::GunWeaponData*>(Master::m_pWeaponDataManager->FindWeaponData(m_CrntSelectItem * 2));
	auto weaponData_2 = static_cast<const WeaponData::GunWeaponData*>(Master::m_pWeaponDataManager->FindWeaponData(m_CrntSelectItem * 2 + 1));

	// 兵装の説明
	Master::m_pDirectWriteManager->SetOutLine(2.0f, D2D1::ColorF(0.0f, 0.0f, 0.0f));
	Master::m_pDirectWriteManager->DrawString(g_WeaponDescriptions[m_CrntSelectItem], VEC2(950.0f, 550.0f), "White_20_STD");
	Master::m_pDirectWriteManager->SetOutLine(0.0f);

	DrawWeaponInfo(weaponData_1, 1,990.0f, 620.0f);
	DrawWeaponInfo(weaponData_2, 2,990.0f, 800.0f);

	Master::m_pDirectWriteManager->SetOutLine(2.0f, D2D1::ColorF(0.0f, 0.0f, 0.0f));
	// 装備を決定したときのテキスト表示 ************************************************************************
	if (m_DecisionTextDrawTimer > 0.0f) {
		std::string decisionText = g_SoldierNames[m_DecisionSoldierTypeIndex];
		float textY = m_pMenuItemRectTransformArray[m_DecisionSoldierTypeIndex].lock()->get_RectPosition().y - 40.0f;
		float textX = m_pMenuItemRectTransformArray[m_DecisionSoldierTypeIndex].lock()->get_RectPosition().x - 150.0f;

		Master::m_pDirectWriteManager->SetColor(D2D1::ColorF(D2D1::ColorF(0.0f, 1.0f, 1.0f)));	// 水色
		Master::m_pDirectWriteManager->DrawString("[" + decisionText + "]" + "を装備しました", VEC2(textX, textY), "White_30_STD");
		Master::m_pDirectWriteManager->SetColor(D2D1::ColorF(D2D1::ColorF::White));

		m_DecisionTextDrawTimer -= deltaTime;
	}
	Master::m_pDirectWriteManager->DrawString("☆兵装選択", VECTOR2::VEC2(40.0f, 500.0f), "White_40_STD");
	Master::m_pDirectWriteManager->SetOutLine(0.0f);

}



//*---------------------------------------------------------------------------------------
//*【?】武器パラメータの描画
//* TODO:ここら辺はAIに噛ませている部分も多いので、要確認
//*
//* [引数]
//* *weaponData : データ
//* weaponIndex : 武器インデックス
//* startX : 位置
//* startY : 位置
//*
//* [返値] なし
//*----------------------------------------------------------------------------------------
void c_Title_SoldierSelect::DrawWeaponInfo(const WeaponData::GunWeaponData* weaponData, int weaponIndex, float startX, float startY)
{
	if (!weaponData) return;

	// 基底データ（全弾種共通のパラメータ）の取得
	const auto& baseBulletData = weaponData->_bulletData;
	const auto& commonData = baseBulletData._commonData;

	float range = commonData._range;

	/* 各パラメータの変換 */
	std::wstring laserSightStr = weaponData->_isLaserSight ? L"装備" : L"----";																// レーザーサイト
	std::wstring zoomStr = weaponData->_zoomLength > 1.0f ? FormatFloat(weaponData->_zoomLength) + L"倍" : L"----";							// ズーム
	std::wstring fireRateStr = FormatFloat(weaponData->_fireRate) + L"発／秒";																// 連射速度
	std::wstring speedStr = L"秒速" + FormatFloat(commonData._speed) + L"m";																// 弾速
	std::wstring penetrationsStr = commonData._penetrationsCount > 0 ? std::to_wstring(commonData._penetrationsCount) + L"回" : L"なし";	// 貫通可能回数
	std::wstring reloadTimeStr = FormatFloat(weaponData->_reloadTime) + L"秒";																// リロード時間
	std::wstring rangeStr = FormatFloat(range) + L"m";																						// 射程距離
	std::wstring accuracyStr;																												// 精度

	if (weaponData->_accuracy <= 0.01f)accuracyStr = L"S";
	else if (weaponData->_accuracy <= 0.015f)accuracyStr = L"A+";
	else if (weaponData->_accuracy <= 0.02f)accuracyStr = L"A";
	else if (weaponData->_accuracy <= 0.025f)accuracyStr = L"B+";
	else if (weaponData->_accuracy <= 0.03f)accuracyStr = L"B";
	else if (weaponData->_accuracy <= 0.035f)accuracyStr = L"C";
	else if (weaponData->_accuracy <= 0.04f)accuracyStr = L"D";
	else if (weaponData->_accuracy <= 0.05f)accuracyStr = L"E";
	else accuracyStr = L"F";
	
	// 同時発射数が1より多い場合はダメージに「x N」を追加
	std::wstring damageStr = FormatFloat((commonData._damage));
	damageStr = (weaponData->_bulletSimultaneousNum > 1) ? damageStr + L" x " + std::to_wstring(weaponData->_bulletSimultaneousNum) : damageStr;

	// 弾種ごとの固有パラメータを取得して文字列化
	std::wstring extraLabelStr = L"";	// ラベル
	std::wstring extraValueStr = L"";	// 値
	std::visit([&extraLabelStr, &extraValueStr](const auto& arg) {
		using T = std::decay_t<decltype(arg)>; // 型を推論

		// 爆発弾の場合のみ、爆発半径の文字列を追加する
		if constexpr (std::is_same_v<T, BulletData::ExplosionHitConfig>) {
			extraLabelStr = L"爆発半径：";
			extraValueStr = FormatFloat(arg._explosionRadius, 1) + L"m";
		}
		// 将来的にショットガン（散弾）などの型が増えたら、ここに else if constexpr を足すだけで対応可能
		}, weaponData->_bulletData._hitData);


	float currentY = startY + 50.0f;
	float col1X = startX;				// 1列目のX
	float col2X = startX + 300.0f;		// 2列目のX
	//float col3X = startX + 600.0f;		// 3列目のX
	float lineHeight = 20.0f;
	VEC2 pos = VEC2();

	// 武器名の描画
	Master::m_pDirectWriteManager->SetOutLine(2.0f, D2D1::ColorF(0.0f, 0.0f, 0.0f));
	std::wstring weaponInfoStr = L"武器" + std::to_wstring(weaponIndex) + L":" + weaponData->_name;
	Master::m_pDirectWriteManager->DrawString(weaponInfoStr, VEC2(startX, startY), "White_30_STD");
	Master::m_pDirectWriteManager->SetOutLine(0.0f);

	// 左側の列
	pos.x = col1X;
	pos.y = currentY;
	this->DrawParam(L"弾数：", std::to_wstring(weaponData->_bulletMaxNum), pos);
	pos.y += lineHeight;
	this->DrawParam(L"連射速度：", fireRateStr , pos);
	pos.y += lineHeight;
	this->DrawParam(L"ダメージ：", damageStr, pos);
	pos.y += lineHeight;
	this->DrawParam(L"リロード時間：", reloadTimeStr, pos);
	pos.y += lineHeight;
	this->DrawParam(L"有効射程距離：", rangeStr, pos);


	// 右側の列
	pos.x = col2X;
	pos.y = currentY;
	this->DrawParam(L"弾速：", speedStr , pos);
	pos.y += lineHeight;
	this->DrawParam(L"精度：", accuracyStr, pos);
	pos.y += lineHeight;
	this->DrawParam(L"ズーム：", zoomStr, pos);
	pos.y += lineHeight;
	this->DrawParam(L"レーザーサイト：", laserSightStr, pos);
	pos.y += lineHeight;
	this->DrawParam(L"敵貫通可能回数：", penetrationsStr, pos);
	pos.y += lineHeight;
	this->DrawParam(extraLabelStr, extraValueStr, pos);

}

//*---------------------------------------------------------------------------------------
//*【?】パラメータの描画
//*
//* [引数]
//* &_label : ラベル名
//* &_value : 値
//* &_pos : 位置
//*
//* [返値] なし
//*----------------------------------------------------------------------------------------
void c_Title_SoldierSelect::DrawParam(const std::wstring& _label, const std::wstring& _value, const VECTOR2::VEC2& _pos)
{
	const float PARENT_BOX_SIZE_X = 300.0f;
	const float PARENT_BOX_SIZE_Y = 50.0f;

	// ラベル名
	Master::m_pDirectWriteManager->SetColor(D2D1::ColorF(1.0f, 1.0f, 1.0f));	// 白
	Master::m_pDirectWriteManager->SetOutLine(2.0f, D2D1::ColorF(0.0f, 0.0f, 0.0f));
	// 右揃えなので、X位置はボックスの半分を引いたもの
	Master::m_pDirectWriteManager->DrawStringToAligment(_label, VEC2(_pos.x - (PARENT_BOX_SIZE_X * 0.5f),_pos.y), "White_20_STD", H_ALIGNMENT::TRAILING, V_ALIGNMENT::TOP, VEC2(PARENT_BOX_SIZE_X, PARENT_BOX_SIZE_Y));

	// 値
	Master::m_pDirectWriteManager->SetColor(D2D1::ColorF(0.0f, 1.0f, 1.0f));	// 水色
	Master::m_pDirectWriteManager->DrawString(_value, VEC2(_pos.x + 150.0f, _pos.y), "White_20_STD", D2D1_DRAW_TEXT_OPTIONS_NONE, false);
	Master::m_pDirectWriteManager->SetColor(D2D1::ColorF(1.0f, 1.0f, 1.0f));	// 白

	Master::m_pDirectWriteManager->SetOutLine(0.0f);
}
