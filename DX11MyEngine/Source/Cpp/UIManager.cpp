#include "pch.h"
#include "UIManager.h"
#include "Component_SpriteRenderer.h"
#include "Component_ButtonUI.h"
#include "Component_RectTransform.h"
#include "ObjectPool.h"
#include "MeshFactory.h"

using namespace GIGA_Engine;
using namespace VECTOR2;
using namespace VECTOR3;
using namespace VECTOR4;

constexpr int NUM_DEFAULT__SPRITE   = 32;       // デフォルトスプライト数
constexpr int NUM_MAX__SPRITE       = 4096;     // 最大スプライト数
constexpr int NUM_DEFAULT__BUTTON   = 16;       // デフォルトボタン数
constexpr int NUM_MAX__BUTTON       = 128;      // 最大ボタン数

//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//*----------------------------------------------------------------------------------------
UIManager::UIManager()
{
}

//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
UIManager::~UIManager()
{
}

//*---------------------------------------------------------------------------------------
//*【?】初期化
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//*
//* [返値]なし
//*----------------------------------------------------------------------------------------
bool UIManager::Init(RendererEngine &renderer)
{
    // 既に作成されているなら返す
    if (!m_UIObjectPoolMap.empty())
    {
        MessageBoxA(NULL, "プールはすでに生成済み", "UIManager", MB_OK);
        return true;
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    //
    //
    //						スプライト用ライトのプール
    // 
    //
    //////////////////////////////////////////////////////////////////////////////////////////
    m_UIObjectPoolMap.emplace(UIData::UI_TYPE::SPRITE, ObjectPool<GameObject>(
        // 取得時に実行 ******************************************************************************************
        [&renderer](GameObject *obj)
        {
            // アクティブに
            obj->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
        },
        // 返却時に実行 ******************************************************************************************
        [](GameObject *obj)
        {
            auto sprite = obj->get_Component<SpriteRenderer>();
            sprite->clear_Texture();       // テクスチャのクリア
            sprite->set_Color(VEC4(1.0f)); 
            sprite->set_UVOffset(VEC2());
        },
        // 生成時に実行 ******************************************************************************************
        [&renderer]()->GameObject *
        {
            CreateSpriteInfo sprite;
            sprite.pTextureMap[0] = Master::m_pResourceManager->LoadWIC_Texture(L"Resource/Texture/Title/GIGACON_Logo.png");  
            sprite.ObjTag = "Sprite";
            sprite.pRenderer = &renderer;
            sprite.ShaderType = SHADER_TYPE::FORWARD_UNLIT_UI_SPRITE;
            sprite.Type = SPRITE_USAGE_TYPE::NORMAL;
            sprite.Width = 200.0f;
            sprite.Height = 200.0f;
            sprite.IsActive = true;
            sprite.IsTransparent = true;
            auto obj = MeshFactory::CreateSprite(sprite);
            obj->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_DONT_DESTROY);    // ノンデストロイ
            obj->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);        // ノンアクティブ

            //*****************************************************************************************
            //						コンポーネントの追加
            //*****************************************************************************************


            return obj.get();
        },
        NUM_DEFAULT__SPRITE,  // デフォルト数
        NUM_MAX__SPRITE       // 最大数
    ));

    //////////////////////////////////////////////////////////////////////////////////////////
    //
    //
    //						ボタン用ライトのプール
    // 
    //
    //////////////////////////////////////////////////////////////////////////////////////////
    m_UIObjectPoolMap.emplace(UIData::UI_TYPE::BUTTON, ObjectPool<GameObject>(
        // 取得時に実行 ******************************************************************************************
        [&renderer](GameObject *obj)
        {
            // アクティブに
            obj->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
        },
        // 返却時に実行 ******************************************************************************************
        [](GameObject *obj)
        {            
            /* スプライト */
            auto sprite = obj->get_Component<SpriteRenderer>();
            sprite->clear_Texture();       // テクスチャのクリア
            sprite->set_Color(VEC4(1.0f)); 
            sprite->set_UVOffset(VEC2());

            /* ボタン */
            auto button = obj->get_Component<ButtonUI>();
            button->set_TextOffsetPos(VEC2());
            button->OnClickFunc(nullptr);
            button->set_Text("Button");
            button->set_IsInteractable(true);
            button->ParamReset();
        },
        // 生成時に実行 ******************************************************************************************
        [&renderer]()->GameObject *
        {
            CreateSpriteInfo sprite;
            sprite.pTextureMap[0] = Master::m_pResourceManager->LoadWIC_Texture(L"Resource/Texture/Title/GIGACON_Logo.png");
            sprite.ObjTag = "Sprite";
            sprite.pRenderer = &renderer;
            sprite.ShaderType = SHADER_TYPE::FORWARD_UNLIT_UI_SPRITE;
            sprite.Type = SPRITE_USAGE_TYPE::NORMAL;
            sprite.Width = 200.0f;
            sprite.Height = 200.0f;
            sprite.IsActive = true;
            sprite.IsTransparent = true;
            auto obj = MeshFactory::CreateSprite(sprite);
            obj->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_DONT_DESTROY);    // ノンデストロイ
            obj->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);        // ノンアクティブ

            //*****************************************************************************************
            //						コンポーネントの追加
            //*****************************************************************************************
            auto button = obj->add_Component<ButtonUI>();
            button->Start(renderer);

            return obj.get();
        },
        NUM_DEFAULT__BUTTON,  // デフォルト数
        NUM_MAX__BUTTON       // 最大数
    ));

	return true;
}

//*---------------------------------------------------------------------------------------
//*【?】更新
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//*
//* [返値]なし
//*----------------------------------------------------------------------------------------
void UIManager::Update(RendererEngine &renderer)
{
    //=========================================================================================
    //
    //						取り出されたオブジェクトのみ更新
    //
    //=========================================================================================
    
    //*****************************************************************************************
    //						UI
    //*****************************************************************************************
    for (auto mapIt = m_ExtractedUIMap.begin(); mapIt != m_ExtractedUIMap.end(); )
    {
        // プールが存在するかどうかの確認
        auto poolIt = m_UIObjectPoolMap.find(mapIt->first);
        if (poolIt == m_UIObjectPoolMap.end())
        {
            OutputDebugStringA("指定されたプールが存在しません");
            continue;
        }

        auto &pool = poolIt->second;    // プールの取り出し
        auto &uiArray = mapIt->second;  // 配列の取り出し


        for (auto bulletIt = uiArray.begin(); bulletIt != uiArray.end(); )
        {
            auto bullet = *bulletIt;

            // アクティブフラグが降りていれば、プールへ返却
            if (bullet->get_IsStatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE) == false)
            {
                // 返却
                pool.release(bullet);

                // 次の要素へ
                bulletIt = uiArray.erase(bulletIt);
            }
            else
            {
                ++bulletIt;
            }
        }

        ++mapIt;
    }


    // ボタン入力の更新
    //ButtonInputProcess(renderer);

    //////////////////////////////////////////////////////////////////////////////////////////
    //						デバッグ用
    //              ※ デバッグモードが有効の際に表示
    //////////////////////////////////////////////////////////////////////////////////////////
    if (Master::m_pDataManager->get_IsDebugMode() == false)return;


    Master::m_pDebugger->BeginDebugWindow(Tool::U8ToChar(u8"UIプールの確認"), 0);

    for (int i = 0; i < static_cast<int>(UIData::UI_TYPE::NUM); i++)
    {
        if (m_UIObjectPoolMap.empty())break;

        if (Master::m_pDebugger->DG_TreeNode(std::to_string(i)))
        {
            // プール本体の情報 **********************************************************
            // プールが存在するかどうかチェック
            auto it = m_UIObjectPoolMap.find(static_cast<UIData::UI_TYPE>(i));
            if (it != m_UIObjectPoolMap.end())
            {
                Master::m_pDebugger->DG_BulletText(Tool::U8ToChar(u8"プール最大数：%d"), it->second.get_MaxNum());
                Master::m_pDebugger->DG_BulletText(Tool::U8ToChar(u8"プールの現在の生成数：%d"), it->second.get_CrntCreateNum());


                // プールから取り出して使用しているオブジェクトの情報 ********************************************************
                auto &extractedIt = m_ExtractedUIMap[static_cast<UIData::UI_TYPE>(i)];
                Master::m_pDebugger->DG_BulletText(Tool::U8ToChar(u8"使用しているオブジェクト数：%d"), extractedIt.size());

            }
            else
            {
                Master::m_pDebugger->DG_BulletText(Tool::U8ToChar(u8"プールが存在しません。"));
            }

            // ツリー終了
            Master::m_pDebugger->DG_TreePop();
        }
    }
    Master::m_pDebugger->EndDebugWindow();

}

//*---------------------------------------------------------------------------------------
//*【?】更新
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//*
//* [返値]なし
//*----------------------------------------------------------------------------------------
void UIManager::Draw(RendererEngine &renderer)
{
}

//*---------------------------------------------------------------------------------------
//*【?】ボタンの入力処理
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//*
//* [返値]なし
//*----------------------------------------------------------------------------------------
void UIManager::ButtonInputProcess(RendererEngine& renderer)
{
    if (m_pCrntFocusedButton == nullptr) {
        return;
    }
    ButtonUI *pNextBtn = nullptr;

    // 入力に応じて次の移動先を取得
    if (GetInputDown(GAME_CONFIG::VIEW_UP))
    {
        pNextBtn = m_pCrntFocusedButton->m_pNavUp;
    }
    else if (GetInputDown(GAME_CONFIG::VIEW_DOWN))
    {
        pNextBtn = m_pCrntFocusedButton->m_pNavDown;
    }
    else if (GetInputDown(GAME_CONFIG::VIEW_LEFT))
    {
        pNextBtn = m_pCrntFocusedButton->m_pNavLeft;
    }
    else if (GetInputDown(GAME_CONFIG::VIEW_RIGHT))
    {
        pNextBtn = m_pCrntFocusedButton->m_pNavRight;
    }

    // 移動先が存在すればフォーカスを移す
    if (pNextBtn != nullptr) {
        m_pCrntFocusedButton->m_IsFocused = false;  // 今のボタンからフォーカスを外す
        pNextBtn->m_IsFocused = true;               // 次のボタンにフォーカスを当てる
        m_pCrntFocusedButton = pNextBtn;

        // カーソル移動音を鳴らす
        //Master::m_pSoundManager->Play(SOUND_TYPE::SE, m_pCrntFocusedButton->m_InputSoundID);
    }
}

//*---------------------------------------------------------------------------------------
//*【?】最初に選択状態にするボタンをセットする
//*
//* [引数]
//* *_firstButton         : 選択状態にするボタン
//*
//* [返値]なし
//*----------------------------------------------------------------------------------------
void UIManager::SetFirstFocus(class ButtonUI* _firstButton)
{
    // 以前のフォーカスを外す
    if (m_pCrntFocusedButton != nullptr) {
        m_pCrntFocusedButton->set_IsFocused(false);
    }

    // 新しいボタンにフォーカスを当てる
    if (_firstButton != nullptr) {
        _firstButton->set_IsFocused(true);
        m_pCrntFocusedButton = _firstButton;
    }
}

//*---------------------------------------------------------------------------------------
//*【?】スプライトの取り出し
//*
//* [引数]
//* &renderer         : 描画エンジンの参照
//* &_transformData   : トランスフォームパラメータ
//* &_param           : パラメータ
//*
//* [返値]なし
//*----------------------------------------------------------------------------------------
GameObject *UIManager::GetSprite(RendererEngine &renderer, const UIData::RectTransformData &_transformData, const UIData::SpriteUIData &_param)
{
    auto &pool = m_UIObjectPoolMap.find(UIData::UI_TYPE::SPRITE)->second;
    auto obj = pool.get();
    if (obj == nullptr)
    {
        OutputDebugStringA("プールに空きがありません");
        return nullptr;
    }

    // トランスフォームの設定
    auto transform = obj->get_RectTransform().lock();
    if (transform == nullptr)
    {
        MessageBoxA(NULL, "UIトランスフォームが見つかりません", "UIManager", MB_OK);
        return nullptr;
    }

    // トランスフォームの設定 *********************************************************************
    UIData::RectTransformData::SetRectTransformData(*transform.get(), _transformData);

    obj->set_LayerRank(_param._layerRank);  // 描画ランクの設定
	obj->set_Tag(_param._tag);              // タグの設定


    // スプライトの初期化 *********************************************************************
    auto sprite = obj->get_Component<SpriteRenderer>();
    if (sprite == nullptr) {
        MessageBoxA(NULL, "スプライトコンポーネントが見つかりません", "UIManager", MB_OK);
        return nullptr;
    }
    UIData::ButtonUIData::SetSpriteData(*sprite.get(), _param); // データのセット

    // 更新リストに登録
    m_ExtractedUIMap[UIData::UI_TYPE::SPRITE].push_back(obj);

    return obj;
}

//*---------------------------------------------------------------------------------------
//*【?】ボタンの取り出し
//*
//* [引数]
//* &renderer         : 描画エンジンの参照
//* &_transformData   : トランスフォームパラメータ
//* &_param           : パラメータ
//*
//* [返値]なし
//*----------------------------------------------------------------------------------------
GameObject *UIManager::GetButton(RendererEngine &renderer, const UIData::RectTransformData &_transformData, const UIData::ButtonUIData &_param)
{
    auto &pool = m_UIObjectPoolMap.find(UIData::UI_TYPE::BUTTON)->second;
    auto obj = pool.get();
    if (obj == nullptr)
    {
        OutputDebugStringA("プールに空きがありません");
        return nullptr;
    }

    // トランスフォームの設定 *********************************************************************
    auto transform = obj->get_RectTransform().lock();
    if (transform == nullptr){
        MessageBoxA(NULL, "UIトランスフォームが見つかりません", "UIManager", MB_OK);
        return nullptr;
    }
    UIData::RectTransformData::SetRectTransformData(*transform.get(), _transformData); // データのセット

    obj->set_LayerRank(_param._layerRank);  // 描画ランクの設定
    obj->set_Tag(_param._tag);              // タグの設定

    // スプライトの初期化 *********************************************************************
    auto sprite = obj->get_Component<SpriteRenderer>();
    if (sprite == nullptr) {
        MessageBoxA(NULL, "スプライトコンポーネントが見つかりません", "UIManager", MB_OK);
        return nullptr;
    }
    UIData::ButtonUIData::SetSpriteData(*sprite.get(), _param); // データのセット


    //ボタンの初期化 *********************************************************************
    auto button = obj->get_Component<ButtonUI>();
    if (button == nullptr) {
        MessageBoxA(NULL, "ボタンコンポーネントが見つかりません", "UIManager", MB_OK);
        return nullptr;
    }
    UIData::ButtonUIData::SetButtonData(*button.get(), _param); // データのセット
    button->set_Sprite(sprite);

    // 更新リストに登録
    m_ExtractedUIMap[UIData::UI_TYPE::BUTTON].push_back(obj);

    return obj;
}

