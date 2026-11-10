#include "pch.h"
#include "Component_Player_HPBar.h"
#include "Component_Health.h"
#include "Component_SpriteRenderer.h"
#include "RendererEngine.h"
#include "GameObject.h"

using namespace VECTOR4;
using namespace VECTOR3;
using namespace VECTOR2;


//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//*----------------------------------------------------------------------------------------
Player_HPBar::Player_HPBar(std::weak_ptr<GameObject> pOwner, int updateRank)
    :IComponent(pOwner, updateRank),
    m_pPlayerHPBarBackSpriteObj(nullptr),
    m_pPlayerHPBarSpriteObj(nullptr),
    m_PrevHP(0.0f),
    m_Timer(0.0f)
{
    this->set_Tag("Player_HPBar");
}


//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
Player_HPBar::~Player_HPBar()
{
    // ---------------------------------------
    // プールへ返す
    // ---------------------------------------
    if (m_pPlayerHPBarBackSpriteObj != nullptr) {
        m_pPlayerHPBarBackSpriteObj->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
    }
    if (m_pPlayerHPBarSpriteObj != nullptr) {
        m_pPlayerHPBarSpriteObj->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
    }
}

//*---------------------------------------------------------------------------------------
//*【?】初期化
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void Player_HPBar::Start(RendererEngine& renderer)
{
    m_pPlayerObj = Master::m_pGameObjectManager->get_ObjectByTag("Player");

    UIData::RectTransformData rectData;
    UIData::SpriteUIData spriteData;


    // プレイヤーHPバー用スプライトを作る **********************************************
    rectData._size = HPBAR_BACK_SIZE;
    rectData._pos = HPBAR_BACK_POS;
    rectData._anchorMax = VEC2(0.0f, 0.0f);
    rectData._anchorMin = VEC2(0.0f, 0.0f);
    spriteData._tag = "HPBar_Back";
    spriteData._imagePath = "Resource/Texture/UI/HPBar_Ver2_ai.png";
    spriteData._layerRank = 101;
    spriteData._color = VEC4(1.0f, 1.0f, 1.0f, 0.5f);
    m_pPlayerHPBarBackSpriteObj = Master::m_pUIManager->GetSprite(renderer, rectData, spriteData);
    m_pPlayerHPBarBackSpriteRenderer = m_pPlayerHPBarBackSpriteObj->get_Component<SpriteRenderer>();


    // プレイヤーHPバー本体 **********************************************
    // HPバー本体はテクスチャなしで、カラーのみで表示する。
    rectData._size = HPBAR_SIZE;
    rectData._pos = HPBAR_POS;
    spriteData._tag = "HPBar";
    spriteData._layerRank = 102;
    spriteData._shaderType = SHADER_TYPE::FORWARD_UNLIT_UI_NOTEXTURE_SPRITE;    // テクスチャなし
    spriteData._color = VECTOR4::VEC4(0.3f, 1.0f, 0.5f, 1.0f);
    m_pPlayerHPBarSpriteObj = Master::m_pUIManager->GetSprite(renderer, rectData, spriteData);
    m_pPlayerHPBarSpriteRenderer = m_pPlayerHPBarSpriteObj->get_Component<SpriteRenderer>();
}

//*---------------------------------------------------------------------------------------
//*【?】更新
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void Player_HPBar::Update(RendererEngine& renderer)
{
    // HPバーの更新
    if (auto player = m_pPlayerObj.lock())
    {
        float deltaTime = Master::m_pTimeManager->get_DeltaTime();
        m_Timer += deltaTime;

        float crntHP = player->get_Component<Health>()->get_CrntHP();
        float maxHP = player->get_Component<Health>()->get_MaxHP();
        VEC4 color = VEC4(1.0f, 1.0f, 1.0f, 0.5f);
        VEC4 bodyColor = VEC4(0.3f, 1.0f, 0.5f, 1.0f);  // 水色
        VEC2 backSpritePos;
        VEC2 bodySpritePos;      
        VEC2 crntHPTextPos;
        VEC2 maxHPTextPos;

        // ダメージを受けたら、揺らす
        if (m_PrevHP > crntHP)
        {
            m_Shaker.Start(SHAKE_HPBAR_DURATION, SHAKE_LENGTH);
            color = VEC4(1.0f, 0.0f, 0.0f, 1.0f);
        }

        m_Shaker.Update(deltaTime); // シェイク更新

        /* 背景部分 */
        backSpritePos = m_Shaker.Apply(HPBAR_BACK_POS); // シェイク適用
        auto backRect = m_pPlayerHPBarBackSpriteObj->get_RectTransform().lock();
        auto backSpriteComp = m_pPlayerHPBarBackSpriteRenderer.lock();
        backRect->set_RectPosition(backSpritePos);  // 位置
        backSpriteComp->set_Color(color);           // カラー

        /* 本体部分 */
        backSpritePos = m_Shaker.Apply(HPBAR_POS); // シェイク適用
        float hpBarScale = (crntHP / maxHP) * SIZE_X;    // HPバーの最大幅が520なので、520倍してスケールに変換
        auto bodyRect = m_pPlayerHPBarSpriteObj->get_RectTransform().lock();
        auto bodySpriteComp = m_pPlayerHPBarSpriteRenderer.lock();
        bodyRect->set_RectPosition(backSpritePos);  // 位置
        bodyRect->set_Size(hpBarScale, SIZE_Y);     // サイズ
        
        // HPが3割り未満になったら赤く点滅させる
        if (crntHP < maxHP * 0.3f){
            float red = sinf(m_Timer * DANGER_COLOR_FADE_SPEED) * 0.5f + 0.5f;  // 0～1
            bodyColor = VEC4(red, 0.0f, 0.0f, 0.5f);
        }
        bodySpriteComp->set_Color(bodyColor);

        /* 数値表示 */
        crntHPTextPos = m_Shaker.Apply(CRNTHP_TEXT_POS); // シェイク適用
        maxHPTextPos = m_Shaker.Apply(MAXHP_TEXT_POS); // シェイク適用
        Master::m_pDirectWriteManager->DrawFormatString("{:d}", crntHPTextPos, "White_20_STD", INT_CAST(crntHP));
        Master::m_pDirectWriteManager->DrawFormatString("{:d}", maxHPTextPos, "White_20_STD", INT_CAST(maxHP));

        m_PrevHP = crntHP;
    }
}
