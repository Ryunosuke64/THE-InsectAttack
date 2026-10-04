#include "pch.h"
#include "DamageTextManager.h"

using namespace VECTOR2;
using namespace VECTOR3;
using namespace Tool;

//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//*----------------------------------------------------------------------------------------
DamageTextManager::DamageTextManager():
	m_PositionType(DAMAGE_TEXT_POSITION_TYPE::FIXED)
{
}

//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
DamageTextManager::~DamageTextManager()
{

}

//*---------------------------------------------------------------------------------------
//*【?】初期化胥吏
//*
//* [引数] なし
//* [返値] なし
//*----------------------------------------------------------------------------------------
bool DamageTextManager::Init()
{
	m_DamageTexts.resize(256);

	return true;
}


//*---------------------------------------------------------------------------------------
//*【?】更新処理
//*
//* [引数]
//* deltaTime : デルタタイム
//*
//* [返値] なし
//*----------------------------------------------------------------------------------------
void DamageTextManager::Update(float deltaTime)
{
	for (auto& damageText : m_DamageTexts)
	{
		if (!damageText.isActive)
		{
			continue;
		}
		damageText.elapsedTime += deltaTime;


		switch (m_PositionType)
		{
		//*****************************************************************************************
		//						ヒット位置
		//*****************************************************************************************
		case DAMAGE_TEXT_POSITION_TYPE::HIT:
		{
			VEC2 screenPos = Tool::ConvertWorldToScreen(damageText.hitPoint);
			damageText.screenPos = screenPos;
			break;
		}

		//*****************************************************************************************
		//						固定位置
		//*****************************************************************************************
		case DAMAGE_TEXT_POSITION_TYPE::FIXED:
		{
			const VEC2 FixedPosition = VEC2(1220.0f, 850.0f);	// 固定位置のスクリーン座標
			const float RiseSpeed = 150.0f;						// 上昇速度

			damageText.screenPos = VEC2(FixedPosition.x, FixedPosition.y);

			// Y軸方向に上昇させる
			damageText.addOffsetY -= RiseSpeed * deltaTime;
			damageText.screenPos.y += damageText.addOffsetY;

			break;
		}
		default:
			break;
		}

		// 経過時間が表示時間を超えたら、削除する
		if (damageText.elapsedTime >= DAMAGE_TEXT_LIFETIME)
		{
			damageText.Reset();
		}
	}
}

//*---------------------------------------------------------------------------------------
//*【?】描画処理
//*
//* [引数] なし
//* [返値] なし
//*----------------------------------------------------------------------------------------
void DamageTextManager::Draw()
{
	// アウトライン設定
	Master::m_pDirectWriteManager->SetOutLine(1.0f, D2D1::ColorF(0.0f, 0.0f, 0.0f));

	for (auto& damageText : m_DamageTexts)
	{
		if (!damageText.isActive)
		{
			continue;
		}
		// 小数点を切り捨てる
		std::string adjustDamageValue = Tool::FormatFloat(damageText.damageValue);
		std::string damageTextStr = adjustDamageValue;

		VEC2 textPos = VEC2(damageText.screenPos.x, damageText.screenPos.y);;

		// ダメージテキストの描画
		Master::m_pDirectWriteManager->DrawString(
			damageTextStr,
			textPos,
			"White_20_STD",
			D2D1_DRAW_TEXT_OPTIONS_NONE,
			false
		);
	}

	// アウトライン解除
	Master::m_pDirectWriteManager->SetOutLine(0.0f);
}


//*---------------------------------------------------------------------------------------
//*【?】ダメージテキストの登録
//*
//* [引数]
//* position : 表示位置
//* damage : ダメージ量
//* positionType : 表示位置のタイプ
// 
//* [返値] なし
//*----------------------------------------------------------------------------------------
void DamageTextManager::
Register(
    const VECTOR3::VEC3& position,
    float damage,
    DAMAGE_TEXT_POSITION_TYPE positionType)
{
	// 使用されていないテキストを探す
    for (auto& text : m_DamageTexts)
    {
        if (text.isActive)
            continue;

        text.hitPoint = position;
		text.screenPos = VEC2(0.0f, 0.0f);
        text.damageValue = damage;
        text.elapsedTime = 0.0f;
        text.isActive = true;

        return;
    }
}


//*---------------------------------------------------------------------------------------
//*【?】テキスト用のスクリーン位置を求める [ヒット位置]
//*
//* [引数]
//* position : 現在の3D空間位置
// 
//* [返値] 
//* テキスト位置 
//*----------------------------------------------------------------------------------------
VEC2 DamageTextManager::ConvertTextPosition_Hit(const VEC3& position)
{
	// とりあえずそのまま返す
	return Tool::ConvertWorldToScreen(position);
}

//*---------------------------------------------------------------------------------------
//*【?】テキスト用のスクリーン位置を求める [固定位置]
//*
//* [引数]
//* position : 現在の3D空間位置
// 
//* [返値] 
//* テキスト位置 
//*----------------------------------------------------------------------------------------
VEC2 DamageTextManager::ConvertTextPosition_Fixed(const VEC3& position)
{
	const VEC2 FixedPosition = VEC2(1220.0f, 650.0f);
	VEC2 screenPos = Tool::ConvertWorldToScreen(position);


}