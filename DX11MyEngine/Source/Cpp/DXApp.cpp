#include "pch.h"
#include "DXApp.h"
#include "Window.h"
#include "Path.h"
#include "InputManager.h"
#include "DirectWriteManager.h"
#include "FontConfig.h"
#include "InputFactory.h"
#include "GameManager.h"
#include "RendererEngine.h"
#include "ResourceManager.h"
#include "MeshFactory.h"
#include "../../resource.h"   

#include <string>
#include <cassert>

using namespace Input;
using namespace VERTEX;
using namespace Tool;


Debugger                *Master::m_pDebugger            = nullptr;  // ImGui機能ラップ
ShaderManager           *Master::m_pShaderManager       = nullptr;  // シェーダ管理
LightManager            *Master::m_pLightManager        = nullptr;  // ライト管理
BlendManager            *Master::m_pBlendManager        = nullptr;  // ブレンド管理
DirectWriteManager      *Master::m_pDirectWriteManager  = nullptr;  // 文字管理 
GameObjectManager       *Master::m_pGameObjectManager   = nullptr;  // オブジェクト管理
ResourceManager         *Master::m_pResourceManager     = nullptr;  // リソース管理
EditorManager           *Master::m_pEditorManager       = nullptr;  // エディタ管理
InputManager            *Master::m_pInputManager        = nullptr;   // 入力管理
CollisionManager        *Master::m_pCollisionManager    = nullptr;   // 衝突管理
EffectManager           *Master::m_pEffectManager       = nullptr;   // エフェクト管理
SoundManager            *Master::m_pSoundManager        = nullptr;   // サウンド管理
TimeManager             *Master::m_pTimeManager         = nullptr;   // 時間管理
DataManager             *Master::m_pDataManager         = nullptr;   // データ管理
BulletManager           *Master::m_pBulletManager       = nullptr;   // 弾管理
UIManager               *Master::m_pUIManager           = nullptr;   // UI管理
RandomManager           *Master::m_pRandomManager       = nullptr;   // 乱数管理
WeaponDataManager       *Master::m_pWeaponDataManager   = nullptr;   // 武器データ管理
ItemManager             *Master::m_pItemManager         = nullptr;   // アイテム管理
TweenManager            *Master::m_pTweenManager        = nullptr;   // Tween管理
BuildingManager         *Master::m_pBuildingManager     = nullptr;   // 建物管理
ScriptManager           *Master::m_pScriptManager       = nullptr;   // AngelScript管理
EnemyManager            *Master::m_pEnemyManager        = nullptr;   // エネミー管理
MissionDirector         *Master::m_pMissionDirector     = nullptr;   // ミッション管理

//*---------------------------------------------------------------------------------------
//* @:DXApp Class 
//*【?】コンストラクタ
//* 引数：1.ウインドウの横幅
//* 引数：2.ウインドウの縦幅
//*----------------------------------------------------------------------------------------
DXApp::DXApp(uint32_t width, uint32_t height) :
    m_hInst(nullptr),
    m_hWnd(nullptr),
    m_Width(width),
    m_Height(height),
    m_pRenderer(),
    m_pGameManager(),
    m_IsClose(false),
    m_IsEditMode(false),
    m_FpsFrameCount(0),
    m_FpsElapsedSeconds(0.0f),
    m_CurrentFps(0.0f)
{

}


//*---------------------------------------------------------------------------------------
//* @:DXApp Class 
//*【?】デストラクタ
//* 引数：なし
//*----------------------------------------------------------------------------------------
DXApp::~DXApp()
{

}


//*---------------------------------------------------------------------------------------
//* @:DXApp Class 
//*【?】初期化
//* 引数：1.ウインドウのインスタンスハンドル
//* 引数：2.今は使用していない
//* 引数：3.ウインドウの表示状態
//* 返値：bool
//*----------------------------------------------------------------------------------------
bool DXApp::Init(HINSTANCE hInstance,LPSTR lpCmdLine, int nCmdShow)
{
    m_hInst = NULL;
    m_hWnd  = NULL;

    /** ゲーム・描画の管理クラス **/
    /**     インスタンス化       **/
    m_pGameManager = new GameManager();
    m_pRenderer = std::make_shared<RendererEngine>();

    Master::m_pDebugger             = new Debugger();               // ImGui機能ラップ 
    Master::m_pShaderManager        = new ShaderManager();          // シェーダ管理
    Master::m_pLightManager         = new LightManager();           // ライト管理
    Master::m_pBlendManager         = new BlendManager();           // ブレンド管理
    Master::m_pDirectWriteManager   = new DirectWriteManager();     // 文字管理 
    Master::m_pGameObjectManager    = new GameObjectManager();      // オブジェクト管理
    Master::m_pResourceManager      = new ResourceManager();        // リソース管理
    Master::m_pEditorManager        = new EditorManager();          // エディタ管理
    Master::m_pInputManager         = new InputManager();           // 入力管理
    Master::m_pCollisionManager     = new CollisionManager();       // 衝突管理
    Master::m_pEffectManager        = new EffectManager();          // エフェクト管理
    Master::m_pSoundManager         = new SoundManager();           // サウンド管理
    Master::m_pTimeManager          = new TimeManager();            // 時間管理
    Master::m_pDataManager          = new DataManager();            // データ管理
    Master::m_pBulletManager        = new BulletManager();          // 弾管理 
    Master::m_pUIManager            = new UIManager();              // UI管理
    Master::m_pRandomManager        = new RandomManager();          // 乱数管理
    Master::m_pWeaponDataManager    = new WeaponDataManager();      // 武器管理
    Master::m_pItemManager          = new ItemManager();            // アイテム管理
    Master::m_pTweenManager         = new TweenManager();           // Tween管理
    Master::m_pBuildingManager      = new BuildingManager();        // 建物管理
    Master::m_pScriptManager        = new ScriptManager();          // AngelScript管理
    Master::m_pEnemyManager         = new EnemyManager();           // エネミー管理
    Master::m_pMissionDirector      = new MissionDirector();        // ミッション管理


    // *************************************************************************************************
    /**  ウインドウの初期化 **/
    // *************************************************************************************************
    if (FAILED(this->InitWindow(hInstance, nCmdShow))) 
    {
        assert(false);
        return false;
    }

    // *************************************************************************************************
    /**  ランダムマネージャー初期化 **/
    // *************************************************************************************************
    if (!Master::m_pRandomManager->Init())
    {
        assert(false);
        return false;
    }

    // *************************************************************************************************
    /**  データマネージャの初期化 **/
    // *************************************************************************************************
    if (!Master::m_pDataManager->Init(m_pRenderer.get()))
    {
        assert(false);
        return false;
    }

    // *************************************************************************************************
    /**  描画(directX)初期化 **/
    // *************************************************************************************************
    if (!m_pRenderer->Init(m_hWnd))
    {
        assert(false);
        return false;
    }

    // *************************************************************************************************
    //** ブレンドステートの初期化 **/
    // *************************************************************************************************
    if (!Master::m_pBlendManager->Init(m_pRenderer))
    {
        assert(false);
        return false;
    }

    // *************************************************************************************************
    //** シェーダ管理の初期化 **/
    //** 描画クラスの弱参照を入れる   **/
    // *************************************************************************************************
    if (!Master::m_pShaderManager->Init(m_pRenderer))
    {
        assert(false);
        return false;
    }

    ///////////////////////////////////////////
    // シェーダ作成
    ///////////////////////////////////////////
    if (!Master::m_pShaderManager->CreateShader(SHADER_TYPE::DEFERRED_STD_STATIC,               SHADER_CREATE_TYPE::CSO))return false;
    if (!Master::m_pShaderManager->CreateShader(SHADER_TYPE::DEFERRED_STD_STATIC_N,             SHADER_CREATE_TYPE::CSO))return false;
    if (!Master::m_pShaderManager->CreateShader(SHADER_TYPE::DEFERRED_STD_SKINNED_N,            SHADER_CREATE_TYPE::CSO))return false;
    if (!Master::m_pShaderManager->CreateShader(SHADER_TYPE::FORWARD_UNLIT_UI_SPRITE,           SHADER_CREATE_TYPE::CSO))return false;
    if (!Master::m_pShaderManager->CreateShader(SHADER_TYPE::DEFERRED_STD_RT_SPRITE,            SHADER_CREATE_TYPE::CSO))return false;
    if (!Master::m_pShaderManager->CreateShader(SHADER_TYPE::FORWARD_UNLIT_STATIC,              SHADER_CREATE_TYPE::CSO))return false;
    if (!Master::m_pShaderManager->CreateShader(SHADER_TYPE::FORWARD_UNLIT_UI_NOTEXTURE_SPRITE, SHADER_CREATE_TYPE::CSO))return false;
    if (!Master::m_pShaderManager->CreateShader(SHADER_TYPE::FORWARD_UNLIT_TRAIL,               SHADER_CREATE_TYPE::RUNTIME))return false;
    if (!Master::m_pShaderManager->CreateShader(SHADER_TYPE::POST_GAUSSIAN_BLUR_HORIZONTAL,     SHADER_CREATE_TYPE::CSO))return false;
    if (!Master::m_pShaderManager->CreateShader(SHADER_TYPE::POST_GAUSSIAN_BLUR_VERTICAL,       SHADER_CREATE_TYPE::CSO))return false;
    if (!Master::m_pShaderManager->CreateShader(SHADER_TYPE::POST_SKYBOX,                       SHADER_CREATE_TYPE::CSO))return false;
    if (!Master::m_pShaderManager->CreateShader(SHADER_TYPE::POST_LUMINANCE_FILTER,             SHADER_CREATE_TYPE::CSO))return false;
    if (!Master::m_pShaderManager->CreateShader(SHADER_TYPE::POST_KAWASE_FILTER,                SHADER_CREATE_TYPE::CSO))return false;
    if (!Master::m_pShaderManager->CreateShader(SHADER_TYPE::POST_SHADOWMAP,                    SHADER_CREATE_TYPE::RUNTIME))return false;
    if (!Master::m_pShaderManager->CreateShader(SHADER_TYPE::POST_SHADOWMAP_SKINNED,            SHADER_CREATE_TYPE::RUNTIME))return false;
    if (!Master::m_pShaderManager->CreateShader(SHADER_TYPE::POST_DEPTH_OF_FILED,               SHADER_CREATE_TYPE::CSO))return false;
    if (!Master::m_pShaderManager->CreateShader(SHADER_TYPE::POST_TONEMAPPING,                  SHADER_CREATE_TYPE::CSO))return false;
    if (!Master::m_pShaderManager->CreateShader(SHADER_TYPE::DEFERRED_STD_TRAIL,                SHADER_CREATE_TYPE::RUNTIME))return false;
    if (!Master::m_pShaderManager->CreateShader(SHADER_TYPE::DEFERRED_STD_DECAL,                SHADER_CREATE_TYPE::CSO))return false;
    if (!Master::m_pShaderManager->CreateShader(SHADER_TYPE::POST_DISTORTION,                   SHADER_CREATE_TYPE::RUNTIME))return false;


    // データ管理にスクリーンの大きさを設定する（定数バッファも更新）
    Master::m_pDataManager->set_ScreenSize(m_pRenderer->get_ScreenWidth(), m_pRenderer->get_ScreenHeight());


    // *************************************************************************************************
    //** リソース管理の初期化 **/
    //** 描画クラスの弱参照を入れる   **/
    // *************************************************************************************************
    if (!Master::m_pResourceManager->Init(m_pRenderer))
    {
        assert(false);
        return false;
    }



    // *************************************************************************************************
    /**  サウンドマネージャの初期化 **/
    // *************************************************************************************************
    if (!Master::m_pSoundManager->InitXA2Sound())
    {
        assert(false);
        return false;
    }

    // *************************************************************************************************
    /**  directWrire 初期化 **/
    // *************************************************************************************************
    if (FAILED(Master::m_pDirectWriteManager->Init(*m_pRenderer)))
    {
        assert(false);
        return false;
    }

    // *************************************************************************************************
    /**  スクリプトマネージャー（AngelScript）初期化 **/
    // *************************************************************************************************
    if (!Master::m_pScriptManager->Init())
    {
        assert(false);
        return false;
    }

    // *************************************************************************************************
    /**  ゲームマネージャー初期化 **/
    // *************************************************************************************************
    if (!m_pGameManager->Init(*m_pRenderer))
    {
        assert(false);
        return false;
    }

    // *************************************************************************************************
    /**  武器データマネージャー初期化 **/
    // *************************************************************************************************
    if (!Master::m_pWeaponDataManager->Init())
    {
        assert(false);
        return false;
    }

    // *************************************************************************************************
    /**  ライトマネージャー初期化 **/
    // *************************************************************************************************
    if (!Master::m_pLightManager->Init(m_pRenderer))
    {
        assert(false);
        return false;
    }

    // *************************************************************************************************
    /** デバッガー初期化 **/
    // *************************************************************************************************
    if (!Master::m_pDebugger->Init(m_hWnd,m_pRenderer))
    {
        assert(false);
        return false;
    }

    // *************************************************************************************************
    /**  入力マネージャー初期化 **/
    // *************************************************************************************************
    if (!Master::m_pInputManager->Init(m_hWnd))
    {
        assert(false);
        return false;
    }
    
    // *************************************************************************************************
    /**  エディタマネージャー初期化 **/
    // *************************************************************************************************
    if (!Master::m_pEditorManager->Init(*m_pRenderer))
    {
        assert(false);
        return false;
    }   
    
    // *************************************************************************************************
    /**  エフェクトマネージャの初期化 **/
    // *************************************************************************************************
    if (!Master::m_pEffectManager->Setup(*m_pRenderer))
    {
        assert(false);
        return false;
    }

    // *************************************************************************************************
    /**  タイムマネージャの初期化 **/
    // *************************************************************************************************
    if (!Master::m_pTimeManager->Init())
    {
        assert(false);
        return false;
    }


    /** フォントデータ作成 **/
    // 白 50サイズ 標準 ----------------------------------------------
    FONT_DATA *pFontData = new FONT_DATA("White_50_STD"); 
    pFontData->fontSize = 50.0f;
    pFontData->fontWeight = DWRITE_FONT_WEIGHT_BOLD;
    pFontData->color = D2D1::ColorF(D2D1::ColorF::White);

    // 登録
    Master::m_pDirectWriteManager->SetFontData(pFontData);
    SAFE_DELETE(pFontData);


    // 白 40サイズ 標準 ----------------------------------------------
    pFontData = new FONT_DATA("White_40_STD");
    pFontData->fontSize = 40.0f;
    pFontData->fontWeight = DWRITE_FONT_WEIGHT_BOLD;
    pFontData->color = D2D1::ColorF(D2D1::ColorF::White);

    // 登録
    Master::m_pDirectWriteManager->SetFontData(pFontData);
    SAFE_DELETE(pFontData);

    // 白 30サイズ 標準 ----------------------------------------------
    pFontData = new FONT_DATA("White_30_STD");  
    pFontData->fontSize = 30.0f;
    pFontData->fontWeight = DWRITE_FONT_WEIGHT_BOLD;
    pFontData->color = D2D1::ColorF(D2D1::ColorF::White);

    // 登録
    Master::m_pDirectWriteManager->SetFontData(pFontData);
    SAFE_DELETE(pFontData);

    // 白 20サイズ 標準 ----------------------------------------------
    pFontData = new FONT_DATA("White_20_STD");  
    pFontData->fontSize = 20.0f;
    pFontData->fontWeight = DWRITE_FONT_WEIGHT_BOLD;
    pFontData->color = D2D1::ColorF(D2D1::ColorF::White);

    // 登録
    Master::m_pDirectWriteManager->SetFontData(pFontData);
    SAFE_DELETE(pFontData);

    // 白 15サイズ 標準 ----------------------------------------------
    pFontData = new FONT_DATA("White_15_STD");  
    pFontData->fontSize = 15.0f;
    pFontData->fontWeight = DWRITE_FONT_WEIGHT_BOLD;
    pFontData->color = D2D1::ColorF(D2D1::ColorF::White);

    // 登録
    Master::m_pDirectWriteManager->SetFontData(pFontData);
    SAFE_DELETE(pFontData);

    // 白 10サイズ 標準 ----------------------------------------------
    pFontData = new FONT_DATA("White_10_STD");  
    pFontData->fontSize = 10.0f;
    pFontData->fontWeight = DWRITE_FONT_WEIGHT_BOLD;
    pFontData->color = D2D1::ColorF(D2D1::ColorF::White);

    // 登録
    Master::m_pDirectWriteManager->SetFontData(pFontData);
    SAFE_DELETE(pFontData);



    // 正常終了
    return true;
}


//*---------------------------------------------------------------------------------------
//* @:DXApp Class 
//*【?】終了処理
//* 引数：なし
//* 返値：void
//*----------------------------------------------------------------------------------------
void DXApp::Term()
{

    m_pGameManager->Term(*m_pRenderer);

    Master::m_pShaderManager->Term();
    Master::m_pLightManager->Term();
    Master::m_pDirectWriteManager->Term();
    Master::m_pBlendManager->Term();
    Master::m_pGameObjectManager->Term(*m_pRenderer);
    Master::m_pResourceManager->Term();
    Master::m_pInputManager->Term();
    Master::m_pSoundManager->UninitXA2Sound();
    m_pRenderer->Term();
}


//*---------------------------------------------------------------------------------------
//* @:DXApp Class 
//*【?】更新処理
//* 引数：なし
//* 返値：int
//*----------------------------------------------------------------------------------------
int DXApp::MainLoop()
{
    timeBeginPeriod(1); // タイマーの分解機能設定を１ms(１ミリ秒 = 1/1000秒)にする
    DWORD crntTime = timeGetTime(); // 現在時刻
    DWORD lastTime = crntTime;      // 前回更新時刻

    MSG msg = { 0 };
    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            crntTime = timeGetTime();   // 現在時刻を更新
            DWORD difference = (crntTime - lastTime);

            // 現在時刻と前回更新時刻の差分が１６.６６msより大きければゲーム処理を実行する
            if (difference > 1000.0f / g_Fps)
            {
                // 終了ボタンが押されるか、ゲームの終了フラグが立っているか
                if (GetInput(GAME_CONFIG::EXIT) || m_pGameManager->get_IsGameClose()) break;   

                // 前回時刻を更新
                lastTime = crntTime;

                float win_width = static_cast<float>(m_pRenderer->get_ScreenWidth());
                float win_height = static_cast<float>(m_pRenderer->get_ScreenHeight());

                Master::m_pTimeManager->Update();

                Master::m_pDebugger->BeginFrame(
                    win_width,
                    win_height,
                    Master::m_pDataManager->get_IsDebugMode());

                // MainLoop先頭で求めたフレーム開始間隔を秒へ変換してFPS計測に渡す。
                // TimeManagerのデルタタイムはタイムスケールやヒットストップの影響を受けるため、
                // 画面が実際に更新された頻度を示すFPSには、未加工の実経過時間を使用する。
                UpdateFps(static_cast<float>(difference) / 1000.0f);

                float deltatime = Master::m_pTimeManager->get_DeltaTime();

				// デバッグキー：F1でデバッグモードの切り替え
                if (GetInputDown(GAME_CONFIG::F1))
                {
                    m_IsEditMode ? m_IsEditMode = false :
                        m_IsEditMode = true;

                    Master::m_pDataManager->set_IdDebugMode(m_IsEditMode);
                }
				// デバッグキー：F1でカメラコントロールモードのオンオフ切り替え
                if (GetInputDown(GAME_CONFIG::F2))
                {
					bool isCameraControlMode = Master::m_pDataManager->get_IsCameraControl();

                    isCameraControlMode ? Master::m_pDataManager->set_IsCameraControl(false) :
						Master::m_pDataManager->set_IsCameraControl(true);
                }
				// デバッグキー：F2で武器使用のオンオフ切り替え
                if (GetInputDown(GAME_CONFIG::F3))
                {
					bool isUseWeapon = Master::m_pDataManager->get_IsUseWeapon();

                    isUseWeapon ? Master::m_pDataManager->set_IsUseWeapon(false) :
						Master::m_pDataManager->set_IsUseWeapon(true);
                }

                // ImGui関連
                AppEditDrawImGui();

                // DirectWrite描画開始
                Master::m_pDirectWriteManager->BeginDraw();

                // 入力更新
                Master::m_pInputManager->Update();
                
                // 描画の開始
                m_pRenderer->BeginRender();

                // ゲーム更新
                m_pGameManager->Update(*m_pRenderer);

                // エディタの更新
                Master::m_pEditorManager->Update(*m_pRenderer);

                // ポーズ中はエフェクト停止
                if (Master::m_pDataManager->get_IsPause() == false)
                {
                    // エフェクト更新（描画はパイプラインクラスのフォワードと同じ位置で行っている）
                    Master::m_pEffectManager->UpdateEffect(*m_pRenderer);
                }


                // ゲーム描画
                m_pGameManager->Draw(*m_pRenderer);

                // サウンドの更新
                Master::m_pSoundManager->Update(*m_pRenderer);

                // DirectWrite描画終了
                Master::m_pDirectWriteManager->EndDraw();

                // DirectWriteまで含む完成フレームを、ImGui描画前にGameウィンドウ用へ保存する。
                // 順番を逆にすると、エディタ自身までキャプチャされて再帰的な表示になる。
                if (Master::m_pDataManager->get_IsDebugMode())
                {
                    m_pRenderer->CaptureFrameBufferForEditor();
                }

                // ImGUI描画終了
                Master::m_pDebugger->EndFrame();

                // 描画の終了
                m_pRenderer->EndRender();

                // 画面更新
                m_pRenderer->Swap();
            }
            //else
            //{
            //    Sleep(1);
            //}
        }
    }

    m_pRenderer->Term();  // リソース解放

    timeEndPeriod(1); // タイマーの分解機能設定をデフォルトに戻す

    return (int)msg.wParam;
}

//*---------------------------------------------------------------------------------------
//* @:DXApp Class 
//*【?】ImGui関連の記述
//* 引数：なし
//* 返値：void
//*----------------------------------------------------------------------------------------
void DXApp::AppEditDrawImGui()
{
    //=========================================================================================
    //
    //						操作ガイド
    //
    //=========================================================================================
    //Master::m_pDebugger->BeginDebugWindow(U8ToChar(u8"説明"));
    //Master::m_pDebugger->DG_Text(U8ToChar(u8"DirectX11を使用した自作3Dグラフィックフレームワークです。"));
    //Master::m_pDebugger->DG_Text(U8ToChar(u8"レンダリングパイプラインから自作し、ディファードシェーディングによる\n基本的なライティング、ポストエフェクト、シャドウ等を実装しました。"));
    //Master::m_pDebugger->DG_Text(U8ToChar(u8"プログラム側はコンポーネント指向で設計し、\n柔軟にオブジェクトを作成できるようにしています。"));
    //Master::m_pDebugger->DG_Text(U8ToChar(u8"※ 現在は敵を一定数倒すだけのシンプルなものになっていますが、\n今後ゲーム部分の拡張を続けていきます。"));
    //Master::m_pDebugger->DG_Separator();    // 区切り
    //Master::m_pDebugger->DG_BulletText(U8ToChar(u8"☆操作方法"));
    //Master::m_pDebugger->DG_Text(U8ToChar(u8"[キーボード]"));
    //Master::m_pDebugger->DG_Text(U8ToChar(u8"WASD：移動"));
    //Master::m_pDebugger->DG_Text(U8ToChar(u8"マウスまたは矢印キー：視点操作"));
    //Master::m_pDebugger->DG_Text(U8ToChar(u8"Space：ジャンプ"));
    //Master::m_pDebugger->DG_Text(U8ToChar(u8"AまたはDを押しながらSpace：緊急回避"));
    //Master::m_pDebugger->DG_Text(U8ToChar(u8"左Shift：押している間、弾が発射されなくなります。"));
    //Master::m_pDebugger->DG_Text(U8ToChar(u8"1または2：武器切り替え"));
    //Master::m_pDebugger->DG_Text(U8ToChar(u8"ESC：終了"));
    //Master::m_pDebugger->DG_Text(U8ToChar(u8"[マウス]"));
    //Master::m_pDebugger->DG_Text(U8ToChar(u8"マウスホイール：武器切り替え"));
    //Master::m_pDebugger->DG_Text(U8ToChar(u8"左クリック：射撃"));
    //Master::m_pDebugger->DG_Text(U8ToChar(u8"右クリック：ズーム"));
    //Master::m_pDebugger->DG_Separator();    // 区切り
    //Master::m_pDebugger->DG_BulletText(
    //    U8ToChar(
    //        u8"オブジェクトリストからオブジェクトを選択し、インスペクタでコンポーネントの\n一部パラメータ等が見れたりいじれるようになっています。"));


    //Master::m_pDebugger->DG_Separator();    // 区切り
    //Master::m_pDebugger->DG_BulletText(U8ToChar(u8"☆実装コンポーネント"));

    //Master::m_pDebugger->DG_BulletText(U8ToChar(u8"ユーティリティ"));
    //Master::m_pDebugger->DG_Text(U8ToChar(u8"・Transform\n・Camera3D\n"));
    //Master::m_pDebugger->DG_Separator();    // 区切り


    //Master::m_pDebugger->DG_BulletText(U8ToChar(u8"メッシュ"));
    //Master::m_pDebugger->DG_Text(U8ToChar(u8"・ModelMeshResource\n・ModelMeshRenderer\n・SkinnedMeshAnimator\n・SkyRenderer\n・SpriteRenderer\n・MeshResource\n・MeshRenderer\n・LineRenderer\n・TrailRenderer\n・BillboardResource\n・BillboardRenderer\n"));
    //Master::m_pDebugger->DG_Separator();    // 区切り


    //Master::m_pDebugger->DG_BulletText(U8ToChar(u8"ライト"));
    //Master::m_pDebugger->DG_Text(U8ToChar(u8"・DirectionLight\n・PointLight\n"));
    //Master::m_pDebugger->DG_Separator();    // 区切り


    //Master::m_pDebugger->DG_BulletText(U8ToChar(u8"コライダー"));
    //Master::m_pDebugger->DG_Text(U8ToChar(u8"・BoxCollider\n・SphereCollider\n"));
    //Master::m_pDebugger->DG_Separator();    // 区切り


    //Master::m_pDebugger->DG_BulletText(U8ToChar(u8"ゲームプレイ用"));
    //Master::m_pDebugger->DG_Text(U8ToChar(u8"・PlayerController\n・EnemyController\n・Health...など\n"));
    //Master::m_pDebugger->DG_Separator();    // 区切り

    //Master::m_pDebugger->EndDebugWindow();


    //=========================================================================================
    //
    //					    アプリケーション情報
    //
    //=========================================================================================
    Master::m_pDebugger->BeginDebugWindow(U8ToChar(u8"アプリケーション設定"),0);

    // FPSの計測処理とは分離し、この関数では表示だけを行う。
    // 表示先を既存のアプリケーション設定ウィンドウにまとめることで、
    // FPS専用ウィンドウが他のデバッグウィンドウと重なることを防いでいる。
    DrawFps();
    Master::m_pDebugger->DG_Separator();

    if (Master::m_pDebugger->DG_CheckBox(U8ToChar(u8"デバッグ用エディタの表示"), &m_IsEditMode))
    {
        Master::m_pDataManager->set_IdDebugMode(m_IsEditMode);
    }

	bool isCameraControl = Master::m_pDataManager->get_IsCameraControl();
    if (Master::m_pDebugger->DG_CheckBox(U8ToChar(u8"視点操作の有効/無効（F1キーで切り替え）"), &isCameraControl))
    {
        Master::m_pDataManager->set_IsCameraControl(isCameraControl);
    }

	bool isUseWeapon = Master::m_pDataManager->get_IsUseWeapon();
    if (Master::m_pDebugger->DG_CheckBox(U8ToChar(u8"武器使用の有効/無効（F2キーで切り替え）"), &isUseWeapon))
    {
        Master::m_pDataManager->set_IsUseWeapon(isUseWeapon);
    }
    Master::m_pDebugger->EndDebugWindow();
}


//*---------------------------------------------------------------------------------------
//* @:DXApp Class
//*【?】FPSの更新
//* 引数：1.前回の描画開始から今回の描画開始までの実経過時間（秒）
//* 返値：void
//*----------------------------------------------------------------------------------------
void DXApp::UpdateFps(float frameElapsedSeconds)
{
    // 0秒以下の値ではFPSを正しく求められないため、異常値は計測対象から除外する。
    if (frameElapsedSeconds <= 0.0f)
    {
        return;
    }

    // 毎フレームの経過時間とフレーム数を蓄積し、計測区間全体の平均FPSを求める。
    // 「1 / 今回の経過時間」を直接表示する方式より、一時的な処理時間の揺れに強くなる。
    m_FpsElapsedSeconds += frameElapsedSeconds;
    ++m_FpsFrameCount;

    // 表示値の更新間隔。短すぎると数値が読みにくく、長すぎると負荷変動への反応が
    // 遅くなるため、ここでは視認性と追従性のバランスがよい0.5秒としている。
    constexpr float FPS_UPDATE_INTERVAL_SECONDS = 0.5f;
    if (m_FpsElapsedSeconds < FPS_UPDATE_INTERVAL_SECONDS)
    {
        return;
    }

    // FPS = 計測区間内の描画フレーム数 / 計測区間の実経過秒数
    m_CurrentFps = static_cast<float>(m_FpsFrameCount) / m_FpsElapsedSeconds;

    // 次の計測区間を開始できるよう、蓄積値だけをリセットする。
    // m_CurrentFpsは次の計測が完了するまで、直近の結果として保持する。
    m_FpsFrameCount = 0;
    m_FpsElapsedSeconds = 0.0f;
}


//*---------------------------------------------------------------------------------------
//* @:DXApp Class
//*【?】FPSのデバッグ表示
//* 引数：なし
//* 返値：void
//*----------------------------------------------------------------------------------------
void DXApp::DrawFps()
{
    // UpdateFpsで更新された平均FPSを小数第1位まで表示する。
    // 初回の0.5秒間は計測区間が完成していないため、初期値の0.0が表示される。
    Master::m_pDebugger->DG_TextValue("FPS : %.1f", m_CurrentFps);
}


//*---------------------------------------------------------------------------------------
//* @:DXApp Class 
//*【?】DXAppの実行（エントリーポイント的な）
//* 引数：1.ウインドウのインスタンスハンドル
//* 引数：2.コマンドライン（今は使用していない）
//* 引数：3.ウインドウを表示するためのもの
//* 返値：void
//*----------------------------------------------------------------------------------------
void DXApp::Run(HINSTANCE hInstance,  LPSTR lpCmdLine, int nCmdShow)
{
    // 初期化が成功したらメインループに入る
    if (Init(hInstance,lpCmdLine,nCmdShow))
    {
        MainLoop();
    }

    // 終了
    Term();
}



// ======================================================================================================
//
/*                      ウインドウ制御関連                 */
//
// ======================================================================================================

//*---------------------------------------------------------------------------------------
//* @:DXApp Class 
//*【?】ウインドウの初期化
//* 引数：1.ウインドウのインスタンスハンドル
//* 引数：2.ウインドウを表示するためのもの
//* 返値：HRESULT
//*----------------------------------------------------------------------------------------
HRESULT DXApp::InitWindow(HINSTANCE hInstance, int nCmdShow)
{
    HICON icon = (HICON)LoadImage(
        hInstance,
        MAKEINTRESOURCE(IDI_ICON1),
        IMAGE_ICON, 
        GetSystemMetrics(SM_CXICON), 
        GetSystemMetrics(SM_CYICON), 
        0
    );

    // ウインドウにデータを渡す
    // Register class
    WNDCLASSEX wcex;
    ZeroMemory(&wcex, sizeof(wcex));
    wcex.cbSize        = sizeof(WNDCLASSEX);    // 構造体のサイズ
    wcex.style         = CS_CLASSDC;            // ウインドウスタイル
    wcex.lpfnWndProc   = WndProc;               // コールバック関数(アドレス)渡す
    wcex.cbClsExtra    = 0;                     // ウインドウクラス構造体の後ろに割り当てる補足バイト数
    wcex.cbWndExtra    = 0;                     // ウインドウインスタンスの後ろに割り当てる補足バイト数
    wcex.hInstance     = hInstance;             // インスタンスハンドル
    wcex.hIcon         = icon/*LoadIcon(NULL, IDI_APPLICATION)*/;   // アイコンのハンドル
    wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);       // マウスカーソルのハンドル
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);        // ウインドウ背景色
    wcex.lpszMenuName  = NULL;                  // デフォルトメニュー名
    wcex.lpszClassName = g_WindowClassNameA;    // オリジナルの名前付ける
    wcex.hIconSm       = NULL;                  // 左上やタスクバーの小さいアイコン
    
    if (!RegisterClassEx(&wcex))                // この設定でウインドウズにお願いする
        return E_FAIL;

    m_WindowStyle = WS_OVERLAPPED | /*WS_CAPTION | */ WS_SYSMENU | /*WS_MAXIMIZEBOX |*/ WS_MINIMIZEBOX;

    // windowの作成
    // Create window
    m_hInst = hInstance;
    RECT rc = { WND_RECT_LEFT, WND_RECT_TOP,  static_cast<LONG>(m_Width), static_cast<LONG>(m_Height) };      // 矩形を設定する
    int res = AdjustWindowRect(&rc, m_WindowStyle, FALSE);   // ウインドウの形を整える

    m_hWnd = CreateWindowW(
        g_WindowClassNameW,                              // 上で設定した名前と合わせる
        g_WindowTitle,                                   // ウインドウタイトル
        m_WindowStyle,                                   // ウインドウスタイル
        /*WS_OVERLAPPEDWINDOW | WS_POPUP,*/              // 基本ウインドウ、タイトルバー表示、Xボタンなど、サイズ変更、全画面
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        rc.right - rc.left,
        rc.bottom - rc.top,
        NULL,
        NULL,
        hInstance,
        NULL);
    
    if (!m_hWnd)
        return E_FAIL;

    // 消すと出ないよ
    ShowWindow(m_hWnd, nCmdShow);


    // ボーダーレスフルスクリーンに設定
    SetBorderlessFullscreen(m_hWnd);

    return S_OK;
}


//*---------------------------------------------------------------------------------------
//* @:DXApp Class 
//*【?】ウインドウの終了
//* 引数：なし
//* 返値：void
//*----------------------------------------------------------------------------------------
void DXApp::TermWindow()
{

}

// Imguiウインドウを動かすためのもの
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
//*---------------------------------------------------------------------------------------
//* @:DXApp Class 
//*【?】ウインドウプロシージャ
//      ※コールバック関数
//* 引数：1.メッセージを受け取ったウインドウのハンドル（どのウインドウに表示するかはこの受け取ったウインドウによって決まっている？）
//* 引数：2.メッセージの種別
//* 引数：3.補助パラメータ的な奴っぽい
//* 引数：4.・・・
//* 返値：LRESULT
//*----------------------------------------------------------------------------------------
LRESULT CALLBACK DXApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

    // Imguiウインドウを動かすためのもの
    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
        return true;

    switch (message)
    {
    case WM_PAINT:      // 再描画命令
        hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        break;

    case WM_DESTROY:    // 終了時に呼ばれる
        PostQuitMessage(0);
        break;


    default:            // デフォルトウインドウプロシージャ(いい感じにやってくれる)
        return DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }

    return 0;
}

//*---------------------------------------------------------------------------------------
//*【?】ボーダーレスウインドウに設定
//*
//* [引数]
//* hwnd : ウインドウハンドル
//*
//* [返値] なし
//*----------------------------------------------------------------------------------------
void DXApp::SetBorderlessFullscreen(HWND hwnd)
{
    // ウィンドウの枠を消す
    SetWindowLong(
        hwnd,
        GWL_STYLE,
        WS_POPUP | WS_VISIBLE
    );

    // モニターサイズ取得
    HMONITOR monitor = MonitorFromWindow(
        hwnd,
        MONITOR_DEFAULTTONEAREST
    );

    MONITORINFO monitorInfo{};
    monitorInfo.cbSize = sizeof(MONITORINFO);

    GetMonitorInfo(monitor, &monitorInfo);

    int width =
        monitorInfo.rcMonitor.right -
        monitorInfo.rcMonitor.left;

    int height =
        monitorInfo.rcMonitor.bottom -
        monitorInfo.rcMonitor.top;

    // モニターいっぱいに広げる
    SetWindowPos(
        hwnd,
        HWND_TOP,
        monitorInfo.rcMonitor.left,
        monitorInfo.rcMonitor.top,
        width,
        height,
        SWP_FRAMECHANGED
    );
}

//*---------------------------------------------------------------------------------------
//*【?】ウインドウモードに設定
//*
//* [引数]
//* hwnd : ウインドウハンドル
//*
//* [返値] なし
//*----------------------------------------------------------------------------------------
void DXApp::SetWindowMode(HWND hwnd)
{
    SetWindowLong(
        hwnd,
        GWL_STYLE,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE
    );

    SetWindowPos(
        hwnd,
        nullptr,
        0,
        0,
        m_Width,
        m_Height,
        SWP_FRAMECHANGED | SWP_NOZORDER
    );
}
