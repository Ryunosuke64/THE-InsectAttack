#include "pch.h"
#include "SceneManager.h"
#include "GameScene_StateHeader.h"
#include "ResourceManager.h"
#include "SceneStateEnums.h"
#include "InputFactory.h"
#include "Component_Health.h"
#include "RendererEngine.h"
#include "GameObject.h"
#include "Component_3DCamera.h"
#include "Component_WeaponController.h"
#include "Component_TrailRenderer.h"

using namespace UtilityData;
using namespace SceneStateEnums;
using namespace VECTOR4;
using namespace VECTOR3;
using namespace VECTOR2;


//*---------------------------------------------------------------------------------------
//* @:c_Game_Play Class 
//*【?】開始
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
void c_Game_Play::OnEnter(SceneManager* pOwner)
{
    // 最初は空
    this->SetInitChildState(pOwner, c_GAME::c_GAME_NONE);


    m_pPlayerObj = Master::m_pGameObjectManager->get_ObjectByTag("Player");

    m_EnemyNum = 0;

    // クリアフラグ初期化
    Master::m_pDataManager->set_IsMissionCleared(false);

    // ****************************************************
    //				ゲームBGMの再生
    // ****************************************************
    Master::m_pSoundManager->PlayBGM(BGM_ID::BGM_GAME_01);


    //*****************************************************************************************
    //						残り敵数の背景スプライト
    //*****************************************************************************************
    UIData::SpriteUIData spriteData;
    UIData::RectTransformData rectTrans;
    spriteData._tag = "EnemyNumBackSprite";
    spriteData._color = VEC4(1.0f, 0.0f, 0.0f, 0.5f);
    spriteData._shaderType = SHADER_TYPE::FORWARD_UNLIT_UI_NOTEXTURE_SPRITE;
    spriteData._layerRank = 110;
    rectTrans._size = ENEMY_NUM_BACK_SPRITE_SIZE;
    rectTrans._pos = ENEMY_NUM_BACK_SPRITE_POS;
    m_pEnemyNumBackSpriteObj = Master::m_pUIManager->GetSprite(*m_pRenderer, rectTrans, spriteData);
}


//*---------------------------------------------------------------------------------------
//* @:c_Game_Play Class 
//*【?】終了
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
void c_Game_Play::OnExit(SceneManager* pOwner)
{
    // ****************************************************
    //				ゲームBGMの停止
    // ****************************************************
    Master::m_pSoundManager->StopBGM(BGM_ID::BGM_GAME_01);

    // 連続入力防止のため、1フレーム止める
    Master::m_pInputManager->StopInput(1);

    // 敵数背景スプライトをプールへ返す
    if (m_pEnemyNumBackSpriteObj)
    {
        m_pEnemyNumBackSpriteObj->clear_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
    }
}


//*---------------------------------------------------------------------------------------
//* @:c_Game_Play Class 
//*【?】更新
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
int c_Game_Play::Update(SceneManager *pOwner)
{
    // プレイヤーが存在しなければ、終了
    if (m_pPlayerObj.expired())
    {
		MessageBeep(MB_ICONEXCLAMATION);
        MessageBoxA(NULL, "プレイヤーが存在しない", "c_Game_Play", MB_OK);
        pOwner->OnSceneClose();
    }

    // ミッションスクリプトの実行
    Master::m_pMissionDirector->ExcuteScriptMissionMain();

    //////////////////////////////////////////////////////////////////////////////////////////
    //
    //						子ステートの処理
    // 
    //////////////////////////////////////////////////////////////////////////////////////////
    if (m_CrntChildStateID != -1) 
    {
        //
        // 子ステートの更新
        //
        int newState = m_pChildStateMap[m_CrntChildStateID]->Update(pOwner);

        //******************************************************************
        // << ポーズ >>
        // ポーズのままなら、その後の処理を飛ばして返す
        //******************************************************************
        if (newState == c_GAME::c_GAME_PAUSE)
        {
            return c_GAME::c_GAME_PLAY;
        }
        //******************************************************************
        // << プレイ >>
        // プレイシーンのままであれば、子をNONEに戻す
        //******************************************************************
        else if (newState == c_GAME::c_GAME_PLAY)
        {
            m_pChildStateMap[m_CrntChildStateID]->OnExit(pOwner);	// 子ステートの終了
            this->SetInitChildState(pOwner, c_GAME::c_GAME_NONE);
        }
        //******************************************************************
        // << ゲームロード >>
        // 再出撃
        //******************************************************************
        else if (newState == c_GAME::c_GAME_LOAD)
        {
            m_pChildStateMap[m_CrntChildStateID]->OnExit(pOwner);	// 子ステートの終了
            this->SetInitChildState(pOwner, c_GAME::c_GAME_NONE);

            // ゲーム中に使用したリソースをクリアする
            Master::m_pDataManager->ClearGameSceneResource(*m_pRenderer);

            return c_GAME::c_GAME_LOAD;
        }
        //******************************************************************
        // << リザルト >>
        // 退却
        //******************************************************************
        else if (newState == c_GAME::c_GO_RESULT_SCENE)
        {
            m_pChildStateMap[m_CrntChildStateID]->OnExit(pOwner);	// 子ステートの終了
            return c_GAME::c_GO_RESULT_SCENE;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    //
    //						現在のステートの処理
    // 
    //////////////////////////////////////////////////////////////////////////////////////////
    // ボタンが押されたら、"ポーズ"へ遷移
    if (GetInputDown(GAME_CONFIG::PAUSE))
    {
        this->SetInitChildState(pOwner, c_GAME::c_GAME_PAUSE);
    }

    // プレイヤーが死んでいたら、"リザルト"へ
    if (Master::m_pDataManager->get_IsPlayerDead())
    {
        // スローモーション
        Master::m_pTimeManager->TriggerHitStop(3.0f, 0.5f);

        // ミッション失敗フラグを設定
        Master::m_pDataManager->set_IsMissionCleared(false);  

        return c_GAME::c_GO_RESULT_SCENE;
    }
    
    // 敵の数
    auto enemys = Master::m_pGameObjectManager->get_ObjectListByFactionAlive(FACTION::ENEMY);
    m_EnemyNum = INT_CAST(enemys.size());

    // 敵が居なくなったら"リザルト"へ
    if (m_EnemyNum == 0)
    {
        // スローモーション
        Master::m_pTimeManager->TriggerHitStop(3.0f, 0.5f);

        // ミッションクリアフラグを設定
        Master::m_pDataManager->set_IsMissionCleared(true);  

        return c_GAME::c_GO_RESULT_SCENE;
    }

    return c_GAME::c_GAME_PLAY;
}


//*---------------------------------------------------------------------------------------
//* @:c_Game_Play Class 
//*【?】描画
//* 引数：1.SceneManager
//* 返値：void
//*----------------------------------------------------------------------------------------
void c_Game_Play::Draw(SceneManager* pOwner)
{

    Master::m_pDirectWriteManager->SetOutLine(3.0f, D2D1::ColorF(0.0f, 0.0f, 0.0f));
    Master::m_pDirectWriteManager->DrawFormatString("残りの敵数：{:d}", VECTOR2::VEC2(0, 540), "White_40_STD", m_EnemyNum);
    Master::m_pDirectWriteManager->DrawFormatString("TABでポーズを開く", VECTOR2::VEC2(0, 620), "White_30_STD");
    Master::m_pDirectWriteManager->SetOutLine(0.0f);

    //////////////////////////////////////////////////////////////////////////////////////////
    //
    //						子ステートの描画処理
    // 
    //////////////////////////////////////////////////////////////////////////////////////////
    if (m_CrntChildStateID != -1)
    {
        m_pChildStateMap[m_CrntChildStateID]->Draw(pOwner);
    }

}


//*---------------------------------------------------------------------------------------
//* @:c_Game_Play Class 
//*【?】爆撃機動かす
//* 引数：なし
//* 返値：void
//*----------------------------------------------------------------------------------------
void c_Game_Play::BomberMove()
{
    m_pBomber[0] = Master::m_pGameObjectManager->get_ObjectByTag("B-2_1");
    m_pBomber[1] = Master::m_pGameObjectManager->get_ObjectByTag("B-2_2");
    m_pBomber[2] = Master::m_pGameObjectManager->get_ObjectByTag("B-2_3");
    static float counter = 0.0f;
    counter += 0.01f;

    VEC3 playerPos = m_pPlayerObj.lock()->get_Transform().lock()->get_VEC3ToPos();
    //-----------------------------------------------------------------------------
    // ■ 爆撃機の移動処理
    //-----------------------------------------------------------------------------
    static VEC3 B2PrevPos[3]{};
    VEC3 center = VEC3(0.0f, 0.0f, 0.0f);
    float radius = 700.0f;
    for (int i = 0; i < 3; i++)
    {
        auto B2Transform = m_pBomber[i].lock()->get_Transform().lock();
        if (B2Transform)
        {
            float intervalDist = 200.0f * i;

            VEC3 pos = VEC3(0.0f, 0.0f, 0.0f);
            pos.x = center.x + (radius - intervalDist) * cos(counter);
            pos.y = 300.0f + i * 100.0f;
            pos.z = center.z + (radius - intervalDist) * sin(counter);

            // 速度 = 今の座標 - 前の座標
            VEC3 velocity = pos - B2PrevPos[i];

            // 移動ベクトルから、Y軸周りの回転角度を求める
            float targetAngle = atan2(velocity.x, velocity.z);

            // 前の座標として保持
            B2PrevPos[i] = pos;

            B2Transform->set_Pos(pos);
            //B2Transform->SlerpLookAt(playerPos, 0.5f);
        }
    }
}
