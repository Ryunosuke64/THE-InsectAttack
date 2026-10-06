//--------------------------------------------------------------------------------------
//      * Includes *
//--------------------------------------------------------------------------------------
#include "pch.h"
#include "RendererEngine.h"
#include "Path.h"
#include "DX_RenderTarget.h"
#include "BlendManager.h"
#include "RenderPipeline.h"
#include "Component_3DCamera.h"
#include "GameObject.h"
#include "ConstantBuffer.h"
#include "ConstantRenderData.h"
#include <string>

using namespace  DirectX;
using namespace  RenderData;
using namespace VECTOR2;
using namespace VECTOR3;
using namespace VECTOR4;

//*---------------------------------------------------------------------------------------
//* @:RendererEngine Class 
//*【?】コンストラクタ
//* 引数：なし
//*----------------------------------------------------------------------------------------
RendererEngine::RendererEngine() :
    m_driverType(D3D_DRIVER_TYPE_UNKNOWN),
    m_featureLevel(D3D_FEATURE_LEVEL_11_0),
    m_pd3dDevice(nullptr),
    m_pImmediateContext(nullptr),
    m_pSwapChain(nullptr),
    m_pRenderTargetView(nullptr),
    m_pDepthStencil(nullptr),
    m_pDepthStencilView(nullptr),
    m_pVertexBuffer(nullptr),
    m_pIndexBuffer(nullptr),
    m_pSamplerLinear(nullptr),
    m_pSamplerShadow(nullptr),
    m_pRasterState_NoneCull(nullptr),
    m_pRasterState_FrontCull(nullptr),
    m_pRasterState_BackCull(nullptr),
    m_pDepthStencilState(nullptr),
    m_pDepthWriteDisabled_DSS(nullptr),
    m_pRendererPipeline(nullptr),
    m_pClampShadow(nullptr),
    m_CrntRenderPass(RENDER_PASS::MAIN),
    m_ScreenWidth(0),
    m_ScreenHeight(0),
    m_hWnd(0),
    m_StartTime(0ul),
    m_CrntViewPort(),
    m_Proj(XMMatrixIdentity()),
    m_View(XMMatrixIdentity())
{
}


//*---------------------------------------------------------------------------------------
//* @:RendererEngine Class 
//*【?】デストラクタ
//* 引数：なし
//*----------------------------------------------------------------------------------------
RendererEngine::~RendererEngine()
{

}


//*---------------------------------------------------------------------------------------
//* @:RendererEngine Class 
//*【?】初期化
//* 引数：1.ウインドウハンドル
//* 戻値：成功したか
//*----------------------------------------------------------------------------------------
bool RendererEngine::Init(HWND hWnd)
{
    m_driverType            = D3D_DRIVER_TYPE_NULL;
    m_featureLevel          = D3D_FEATURE_LEVEL_11_0;    // DirectXのバージョン
    m_pd3dDevice            = NULL;        // GPUを抽象化してくれる
    m_pImmediateContext     = NULL;        // 絵を描く部分を抽象化したもの
    m_pSwapChain            = NULL;        // バッファの入れ替え(バッファ数増やすと入力遅延が起きる)
    m_pRenderTargetView     = NULL;        // 絵を描く領域を切り替える(メモリ領域を覗く、眼鏡みたいなもの)
    m_pDepthStencil         = NULL;        // 画像を読み込んで使えるようにするもの
    m_pDepthStencilView     = NULL;        // 深度バッファ(ZBuffer)奥行き
    m_pVertexBuffer         = NULL;        // 頂点バッファ(実際の頂点のデータが詰まっている)
    m_pSamplerLinear        = NULL;        // テクスチャからどうピクセルをもらうか、サンプルをどうするか
    m_pSamplerShadow        = NULL;        // シャドウマップ用サンプラー
    m_pRasterState_NoneCull = NULL;        // ラスタライザ
    m_pRasterState_FrontCull = NULL;
    m_pRasterState_BackCull = NULL;
    m_pDepthStencilState    = NULL;        // Z比較をするための設定
    //m_pBlendStateAlpha = NULL;           // αブレンド用
    //m_pBlendStateAdd   = NULL;           // 加算合成用
    //m_pBlendStateSub   = NULL;           // 減算合成用

    m_hWnd = hWnd;  // ウインドウハンドル受け取る

    RECT client_rc;
    // ウインドウの黒い部分の大きさを返す
    if (GetClientRect(m_hWnd, &client_rc) == false)
    {
        return false;
    }
    m_ScreenWidth = client_rc.right - client_rc.left;
    m_ScreenHeight = client_rc.bottom - client_rc.top;

    // データ管理にスクリーンの大きさを設定する
    //Master::m_pDataManager->set_ScreenSize(m_ScreenWidth, m_ScreenHeight);

    // 正常に初期化されたか
    if (!InitDx11()) {
        CleanupDX11();  // されて無ければ解放
        return false;
    }

    // 初期化
    //m_RenderParam.Init(*this);

    m_StartTime = timeGetTime();    // 開始時間取得

    // 投影変換行列の設定
    //SetupProjectionTransform(static_cast<float>(m_ScreenWidth), static_cast<float>(m_ScreenHeight), 100.0f, 1.0f, 10000.0f);


    return true;
}


//*---------------------------------------------------------------------------------------
//* @:RendererEngine Class 
//*【?】描画の開始
//* 引数：なし
//* 戻値：void
//*----------------------------------------------------------------------------------------
void RendererEngine::BeginRender()
{
    // フレームバッファのレンダリングターゲットとデプスステンシルのクリア
    FLOAT clearColor[] = { 1.0f,0.0f,1.0f,1.0f };
    m_pImmediateContext->ClearRenderTargetView(m_pRenderTargetView, clearColor);
    m_pImmediateContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0);


    //// OMにブレンドステートオブジェクトを設定
    //FLOAT BlendFactor[4] = { 0.f, 0.f, 0.f, 0.f };
    //m_pImmediateContext->OMSetBlendState(m_pBlendStateAlpha, nullptr, 1);   
    
    // 深度ステンシルステート設定
    m_pImmediateContext->OMSetDepthStencilState(m_pDepthStencilState, 0);   

    // ラスタライザ設定
    m_pImmediateContext->RSSetState(m_pRasterState_BackCull);

    ID3D11SamplerState *samplers[] =  {
        m_pSamplerLinear,
        m_pSamplerShadow,
        m_pClampShadow,
    };

    // サンプラー設定
    m_pImmediateContext->PSSetSamplers(0, ARRAYSIZE(samplers), samplers);


    //*****************************************************************************************
    //						フレーム定数バッファを設定する
    //*****************************************************************************************
    CB_FRAME cbFrame{};
    float gameElapsedTime =  Master::m_pTimeManager->get_GameElapsedTime();
    float deltaTime =  Master::m_pTimeManager->get_DeltaTime();
    cbFrame.time = gameElapsedTime;
    cbFrame.deltaTime = deltaTime;
    Master::m_pShaderManager->BindConstantBuffer(CONSTANT_BUFFER_TYPE::FRAME, (void*)&cbFrame, sizeof(CB_FRAME));
}


//*---------------------------------------------------------------------------------------
//* @:RendererEngine Class 
//*【?】描画の終了
//* 引数：なし
//* 戻値：void
//*----------------------------------------------------------------------------------------
void RendererEngine::EndRender()
{
    // なにかあれば

    ReleaseRenderTargetSetNull();

}

//*---------------------------------------------------------------------------------------
//* 完成したゲーム画面をエディタ表示用に保存し、ImGui描画用の画面を準備する
//*----------------------------------------------------------------------------------------
void RendererEngine::CaptureFrameBufferForEditor()
{
    // 初期化に失敗してコピー先がない場合は、通常のバックバッファ表示を壊さず終了する。
    if (!m_pFrameBufferTexture || !m_pSwapChain)
    {
        return;
    }

    ID3D11Texture2D* pBackBuffer = nullptr;
    const HRESULT hr = m_pSwapChain->GetBuffer(
        0,
        __uuidof(ID3D11Texture2D),
        reinterpret_cast<void**>(&pBackBuffer));
    if (FAILED(hr))
    {
        return;
    }

    // CopyResourceのコピー元をRTVへバインドしたままにしないよう、一度出力先から外す。
    // この時点のバックバッファには3D、ゲームUI、DirectWrite文字まで描画済みで、
    // まだImGuiは描画されていないため、エディタ自身を含まないゲーム画面だけを保存できる。
    ReleaseRenderTargetSetNull();
    m_pImmediateContext->CopyResource(m_pFrameBufferTexture.Get(), pBackBuffer);
    pBackBuffer->Release();

    // コピー後は再びスワップチェインのバックバッファをImGuiの描画先に戻す。
    set_ViewPort(
        0.0f,
        0.0f,
        static_cast<float>(m_ScreenWidth),
        static_cast<float>(m_ScreenHeight));

    ChangeRenderTargetFrameBuffer();

    // 元の全画面ゲーム画像を消してからエディタを描画し、パネルの隙間への映り込みを防ぐ。
    const FLOAT editorBackground[] = { 0.075f, 0.075f, 0.085f, 1.0f };
    m_pImmediateContext->ClearRenderTargetView(m_pRenderTargetView, editorBackground);
}

//*---------------------------------------------------------------------------------------
//* @:RendererEngine Class 
//*【?】終了
//* 引数：なし
//* 戻値：void
//*----------------------------------------------------------------------------------------
void RendererEngine::Term()
{
    CleanupDX11();  // リソース解放
}


//*---------------------------------------------------------------------------------------
//* @:RendererEngine Class 
//*【?】スワップ・裏表切り替え
//* 引数：なし
//* 戻値：void
//*----------------------------------------------------------------------------------------
void RendererEngine::Swap()
{
    HRESULT hr;
    hr = m_pSwapChain->Present(1, 0);    // 引数１:垂直同期　２：特殊フラグ(0でいい)

    if (FAILED(hr))return;
}


//
//
/* ------------------------ ↓↓↓ ＤｉｒｅｃｔＸ１１ ↓↓↓ ------------------------ */
//
// 

//--------------------------------------------------------------------------------------
//      * RendererEngine Class - デバッグレイヤーの有効化 - *
//      https://zenn.dev/kd_gamegikenblg/articles/431326b7f0cb2b
//--------------------------------------------------------------------------------------
void RendererEngine::EnableDebugLayer()
{
    ID3D11Debug* debugLayer = nullptr;
}


//*---------------------------------------------------------------------------------------
//* @:RendererEngine Class 
//*【?】DirectX11の初期化
//* 引数：なし
//* 戻値：成功したか
//*----------------------------------------------------------------------------------------
bool RendererEngine::InitDx11()
{
    // ＤｉｒｅｃｔＸ１１ 各初期化
    if (FAILED(InitDX11_SwapChain()))       return false;
    if (FAILED(InitDX11_RenderTargetView()))return false;
    if (FAILED(InitDX11_ZBuff()))           return false;
    if (FAILED(InitDX11_Rasterizer()))      return false;
    if (FAILED(InitDX11_Sampler()))         return false;


    //if (FAILED(InitDX11_BlendState()))      return false;

    return true;
}

//*---------------------------------------------------------------------------------------
//* @:RendererEngine Class 
//*【?】スワップチェインの初期化
//* 引数：なし
//* 戻値：成功したか
//*----------------------------------------------------------------------------------------
HRESULT RendererEngine::InitDX11_SwapChain()
{
    HRESULT hr = S_OK;

    /* ここ 0指定だとDirect2Dを使う際は D3D11_CREATE_DEVICE_BGRA_SUPPORT を指定しないと動かない！！！！ */
    UINT createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT ;


    /* ここフラグ指定するとエラーが出るので一旦コメントアウト */
#ifdef  _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif 
    // ドライバタイプ 描画をハードウェアとソフトウェアのどちらで行うか
    // ※ Direct2Dを使う場合、HADWAREじゃないと動かない http://mitsunagistudio.net/old_posts/tips/d2d-d3d11-sharing/
    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,     // GPUにレンダリングを任せる
        //D3D_DRIVER_TYPE_WARP,       // 高機能？
        //D3D_DRIVER_TYPE_REFERENCE,  // 精度重視(速度遅め)
    };

    UINT numDriverTypes = ARRAYSIZE(driverTypes);   // 配列サイズ取得

    // ドライバの機能レベル
    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

    UINT numFeatureLevels = ARRAYSIZE(featureLevels); // 配列サイズ取得

    //
    /* スワップチェインの設定 */
    //
    {
        DXGI_SWAP_CHAIN_DESC sd;
        ZeroMemory(&sd, sizeof(sd));    // メモリを０で埋める
        sd.BufferCount          = 2;                                // バッファ(裏画面)の数(絵を描くキャンパスの枚数)
        sd.BufferDesc.Width     = m_ScreenWidth;                    // バッファの横幅
        sd.BufferDesc.Height    = m_ScreenHeight;                   // バッファの縦幅
        sd.BufferDesc.Format    = DXGI_FORMAT_B8G8R8A8_UNORM;       // カラーフォーマット
        sd.BufferDesc.RefreshRate.Numerator   = 60;                 // リフレッシュレートの分母
        sd.BufferDesc.RefreshRate.Denominator = 1;                  // リフレッシュレートの分子
        sd.BufferUsage          = DXGI_USAGE_RENDER_TARGET_OUTPUT;  // バッファの使い方
        sd.OutputWindow         = m_hWnd;                           // 出力対象のウインドウハンドル
        sd.SampleDesc.Count     = 1;                                // マルチサンプリングのサンプル数(未使用は1)
        sd.SampleDesc.Quality   = 0;                                // マルチサンプリングの品質(未使用は0)
        sd.Windowed             = TRUE;                             // ウインドウモード指定
        sd.SwapEffect           = DXGI_SWAP_EFFECT_FLIP_DISCARD;    // ..._FLIP...のものが現在は推奨(それ以外はビットで切り替え。これはポインタの差し替えで画面更新)

        // ドライバー種別を上から検証し選択
        // 選択したデバイスを用いて描画する
        for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
        {
            m_driverType = driverTypes[driverTypeIndex];
            hr = D3D11CreateDeviceAndSwapChain(
                NULL,                   // ビデオアダプタの指定(NULLで既定)
                m_driverType,           // ドライバのタイプ
                NULL,                   // D3D_DRIVER_TYPE_SOFTWARE(ソフトウェア)指定時に使用
                createDeviceFlags,      // フラグ指定
                featureLevels,          // D3D_FEATURE_LEVEL指定で自分で定義した配列を指定可能
                numFeatureLevels,       // 上のD3D_FEATURE_LEVEL配列の要素数
                D3D11_SDK_VERSION,      // SDKバージョン
                &sd,                    // DXGI_SWAP_CHAIN_DESC
                &m_pSwapChain,          // 関数成功時のSwapChainの出力先
                &m_pd3dDevice,          // 関数成功時のDeviceの出力先
                &m_featureLevel,        // 成功したD3D_FEATURE_LEVELの出力先
                &m_pImmediateContext    // 関数成功時のContextの出力先
            );

            if (SUCCEEDED(hr)) break;
        }

        if (FAILED(hr))return hr;
    }

    return hr;
}


//*---------------------------------------------------------------------------------------
//* @:RendererEngine Class 
//*【?】レンダーターゲットビューの作成
//* 引数：なし
//* 戻値：成功したか
//*----------------------------------------------------------------------------------------
HRESULT RendererEngine::InitDX11_RenderTargetView()
{
    HRESULT hr = S_OK;

    ID3D11Texture2D* pBackBuffer = NULL;

    // バックバッファ取得
    hr = m_pSwapChain->GetBuffer(
        0,                          // バックバッファのインデックス(0は裏)
        __uuidof(ID3D11Texture2D),  // 取得したいインターフェース型のGUID
        (LPVOID*)&pBackBuffer       // バックバッファの格納先
    );
    if (FAILED(hr)) return hr;

    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
    rtvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Texture2D.MipSlice = 0;

    // 作成
    hr = m_pd3dDevice->CreateRenderTargetView(
        pBackBuffer,                // 元となるテクスチャ(バックバッファ)
        &rtvDesc,                   // D3D11_RENDER_TARGET_VIEW_DESC ( NULLなら自動設定 )
        &m_pRenderTargetView        // 生成したレンダーターゲットの出力先
    );
    if (FAILED(hr))
    {
        pBackBuffer->Release();
        return hr;
    }

    // 完成したバックバッファをGameウィンドウで表示するため、
    // シェーダーから参照できる同サイズのコピー先を用意する。
    // CopyResourceには同じフォーマット系列が必要なので、BGRA8のTYPELESSを使用する。
    D3D11_TEXTURE2D_DESC gameViewTextureDesc{};
    pBackBuffer->GetDesc(&gameViewTextureDesc);
    gameViewTextureDesc.Format = DXGI_FORMAT_B8G8R8A8_TYPELESS;
    gameViewTextureDesc.Usage = D3D11_USAGE_DEFAULT;
    gameViewTextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    gameViewTextureDesc.CPUAccessFlags = 0;
    gameViewTextureDesc.MiscFlags = 0;

    hr = m_pd3dDevice->CreateTexture2D(
        &gameViewTextureDesc,
        nullptr,
        m_pFrameBufferTexture.GetAddressOf());
    if (FAILED(hr))
    {
        pBackBuffer->Release();
        return hr;
    }

    // バックバッファはSRGBのRTVを通して書き込まれている。
    // 読み出し側もSRGBビューにすることで、ImGuiで再描画した際の二重ガンマ補正を防ぐ。
    D3D11_SHADER_RESOURCE_VIEW_DESC gameViewSrvDesc{};
    gameViewSrvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
    gameViewSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    gameViewSrvDesc.Texture2D.MostDetailedMip = 0;
    gameViewSrvDesc.Texture2D.MipLevels = 1;

    hr = m_pd3dDevice->CreateShaderResourceView(
        m_pFrameBufferTexture.Get(),
        &gameViewSrvDesc,
        m_pFrameBufferSRV.GetAddressOf());
    pBackBuffer->Release();
    if (FAILED(hr)) return hr;


    //
    /* レンダリングターゲットのどの部分に絵を描くか */
    //
    D3D11_VIEWPORT vp;
    ZeroMemory(&vp, sizeof(vp));
    vp.Width = (FLOAT)m_ScreenWidth;   // 描画範囲の横幅       
    vp.Height = (FLOAT)m_ScreenHeight; // 描画範囲の縦幅
    vp.MinDepth = 0.0f;                // Ｚバッファの最小値
    vp.MaxDepth = 1.0f;                // Ｚバッファの最大値
    vp.TopLeftX = 0.0f;                // 描画範囲の左側Ｘ座標
    vp.TopLeftY = 0.0f;                // 描画範囲の上部Ｙ座標
    m_pImmediateContext->RSSetViewports(1, &vp);

    return hr;
}


//*---------------------------------------------------------------------------------------
//* @:RendererEngine Class 
//*【?】Z（デプス）バッファの初期化
//* 引数：なし
//* 戻値：成功したか
//*----------------------------------------------------------------------------------------
HRESULT RendererEngine::InitDX11_ZBuff()
{
    HRESULT hr = S_OK;

    // 深度ステンシル用テクスチャリソース作成
    D3D11_TEXTURE2D_DESC descDepth;                     // テクスチャの情報を入れる構造体
    ZeroMemory(&descDepth, sizeof(descDepth));          // メモリを0で埋める
    descDepth.Width              = m_ScreenWidth;       // バックバッファと同じサイズ
    descDepth.Height             = m_ScreenHeight;      //...
    descDepth.MipLevels          = 1;                   // LOD処理の一つ0にすると限界まで作成する
    descDepth.ArraySize          = 1;                   // 基本的には１(キューブマップとかやる場合は使う)
    descDepth.SampleDesc.Count   = 1;                   // MSAA(マルチサンプリングエイリアス)の設定
    descDepth.SampleDesc.Quality = 0;                   // サンプリングのモード切替え(ハードウェア依存) 基本的に 0 
    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;   // デプスバッファを24分割(0.0～1.0) 32bitのうちデプス24、ステンシル8

    // D3D11_USAGE_DEFAULT = 0,     // GPUからRead,Write
    // D3D11_USAGE_IMMUTABLE = 1,   // GPUからRead
    // D3D11_USAGE_DYNAMIC = 2,     // CPUからWrite,GPUからRead
    // D3D11_USAGE_STAGING = 3      // CPUからRead,Write
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    // D3D11_CPU_ACCESS_READ
    // D3D11_CPU_ACCESS_WRITE
    descDepth.CPUAccessFlags = 0;   // ＣＰＵの読み書き
    descDepth.MiscFlags      = 0;   // 基本的に 0 (特殊なフラグ)

    // 作成
    hr = m_pd3dDevice->CreateTexture2D(&descDepth, NULL, &m_pDepthStencil);
    if (FAILED(hr)) return hr;

    // 深度ステンシルビュー作成
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    ZeroMemory(&descDSV, sizeof(descDSV));
    descDSV.Format          = descDepth.Format;              // 上で設定したものを入れる
    descDSV.ViewDimension   = D3D11_DSV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
    descDSV.Texture2D.MipSlice = 0;

    // 作成
    hr = m_pd3dDevice->CreateDepthStencilView(m_pDepthStencil, &descDSV, &m_pDepthStencilView);
    if (FAILED(hr))return hr;

    // MRT(マルチレンダリングターゲット)の設定もできる
    //m_pImmediateContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);


    //
    /* Ｚテストの設定 */
    //
    D3D11_DEPTH_STENCIL_DESC depthDesc;
    ZeroMemory(&depthDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));

    // D3D11_DEPTH_WRITE_MASK_ZERO  = 0, Z(depth)Test OFF
    // D3D11_DEPTH_WRITE_MASK_ALL   = 1  Z(depth)Test ON

    // Zテスト有りver
    // 通常================================================================
    // ******** 深度テスト設定 ********
    depthDesc.DepthEnable = TRUE;                            // 深度テスト有効化
    depthDesc.DepthWriteMask  = D3D11_DEPTH_WRITE_MASK_ALL;  // 深度書き込み
    depthDesc.DepthFunc       = D3D11_COMPARISON_LESS;       // ＜ 小なり(描画対象のＺ値が既存ピクセルより手前なら描画)

    // ******** ステンシルテスト設定 ********
    depthDesc.StencilEnable   = TRUE;                       // ステンシルテスト無効
    depthDesc.StencilReadMask = 0xFF;
    depthDesc.StencilWriteMask = 0xFF;
    // ポリゴンの前面（FrontFace）の設定
    depthDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;        // ステンシルテストの失敗時（KEEP : 今の値を保持）
    depthDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;   // 深度テストの失敗時（KEEP : 値の書き換えをしない）
    depthDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;     // 両方合格した場合は書き換え
    depthDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;        // 常にテストに合格させる（建物とか）
    depthDesc.BackFace = depthDesc.FrontFace;   // 裏面も同じ

    hr = m_pd3dDevice->CreateDepthStencilState(&depthDesc, &m_pDepthStencilState); // ステート作成
    if (FAILED(hr))return hr;
    

    // デカール用================================================================
    D3D11_DEPTH_STENCIL_DESC decalDesc;
    ZeroMemory(&decalDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
    // ******** 深度テスト設定 ********
    decalDesc.DepthEnable = FALSE;                            // 深度テスト無効
    decalDesc.DepthWriteMask  = D3D11_DEPTH_WRITE_MASK_ZERO;  // 書き込みなし
    decalDesc.DepthFunc       = D3D11_COMPARISON_LESS_EQUAL;

    // ******** ステンシルテスト設定 ********
    decalDesc.StencilEnable = TRUE;
    decalDesc.StencilReadMask = 0xFF;  // 全ビット読み取り対象
    decalDesc.StencilWriteMask = 0x00; // デカール描画でステンシルバッファを書き換えない
    // 前面（FrontFace）の設定
    decalDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;       // バッファの値が参照値と一致すれば描画
    decalDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;      // 失敗しても値の書き換えをしない
    decalDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP; // 失敗しても値の書き換えをしない
    decalDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;      // 成功しても値を変えない
    // 裏面も同じ
    decalDesc.BackFace = decalDesc.FrontFace;
    hr = m_pd3dDevice->CreateDepthStencilState(&decalDesc, &m_pDecal_DSS); // ステート作成
    if (FAILED(hr))return hr;


    // 深度書き込みなしver================================================================
    D3D11_DEPTH_STENCIL_DESC noWriteDesc;
    ZeroMemory(&noWriteDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
    noWriteDesc.DepthEnable = TRUE;
    noWriteDesc.StencilEnable = FALSE;
    noWriteDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;  // 書き込みなし
    noWriteDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    hr = m_pd3dDevice->CreateDepthStencilState(&noWriteDesc, &m_pDepthWriteDisabled_DSS); // ステート作成
    if (FAILED(hr))return hr;

    return hr;
}


//*---------------------------------------------------------------------------------------
//* @:RendererEngine Class 
//*【?】ラスタライザの初期化
//* 引数：なし
//* 戻値：成功したか
//*----------------------------------------------------------------------------------------
HRESULT RendererEngine::InitDX11_Rasterizer()
{
    /* ラスタライザにどんな処理をしてもらうか */

    HRESULT hr = S_OK;

    D3D11_RASTERIZER_DESC rd;
    ZeroMemory(&rd, sizeof(rd));
    
    // ※ CullMode
    // D3D11_CULL_NONE  = 1,      // カリングしない(重い)
    // D3D11_CULL_FRONT = 2,      // 表はカリング(時計回りのポリゴンが消える)
    // D3D11_CULL_BACK  = 3       // 裏はカリング(逆時計回りのポリゴンが消える)
    rd.CullMode              = D3D11_CULL_NONE;
    rd.FillMode              = D3D11_FILL_SOLID; // どう塗りつぶすか（ここでは普通に描画）
    rd.MultisampleEnable     = FALSE;   // マルチサンプリング時の配慮をするか(ポリゴンの端を滑らかにできるが処理コスト増)
    rd.FrontCounterClockwise = FALSE;   // 時計回りが表面
    rd.DepthBias             = 0;       // 深度バイアス
    rd.DepthBiasClamp        = 0.0;
    rd.SlopeScaledDepthBias  = 0.0;
    rd.DepthClipEnable       = TRUE;    // 0.0より小さい1.0より大きい場合計算しない(カメラのnarとfarのやつ)
    rd.ScissorEnable         = FALSE;   // シザー矩形なし
    rd.AntialiasedLineEnable = FALSE;   // ライン・アンチエイリアシングなし

    // カリングなしステート作成 **********************************************************
    m_pd3dDevice->CreateRasterizerState(&rd, &m_pRasterState_NoneCull);

    // 表カリングステート作成 **********************************************************
    rd.CullMode = D3D11_CULL_FRONT;
    rd.DepthBias            = 0; // 深度バイアス
    rd.SlopeScaledDepthBias = 0;
    m_pd3dDevice->CreateRasterizerState(&rd, &m_pRasterState_FrontCull);

    // 裏カリングステート作成 **********************************************************
    rd.CullMode = D3D11_CULL_BACK; 
    m_pd3dDevice->CreateRasterizerState(&rd, &m_pRasterState_BackCull); 

    return hr;
}


//*---------------------------------------------------------------------------------------
//* @:RendererEngine Class 
//*【?】サンプラーの初期化
//* 引数：なし
//* 戻値：成功したか
//*----------------------------------------------------------------------------------------
HRESULT RendererEngine::InitDX11_Sampler()
{
    /* サンプラーの設定 */

    HRESULT hr = S_OK;

    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));

    /* サンプリングするときに使用するフィルター方法 */
    // D3D11_FILTER_MIN_MAG_MIP_POINT   そのまま
    // D3D11_FILTER_MIN_MAG_MIP_LINEAR  色の平均値(ぼやける)
    // D3D11_FILTER_ANISOTROPIC         異方性フィルタ(3Dの時はこれ使う)

    sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;  // 異方性フィルタ

    /* 0～1の範囲外のu v w テクスチャ座標の解決方法 */
    // D3D11_TEXTURE_ADDRESS_WRAP = 1,       繰り返し
    // D3D11_TEXTURE_ADDRESS_MIRROR = 2,     反転
    // D3D11_TEXTURE_ADDRESS_CLAMP = 3,      最後のピクセルを繰り返す
    // D3D11_TEXTURE_ADDRESS_BORDER = 4,     自分で色を設定する
    // D3D11_TEXTURE_ADDRESS_MIRROR_ONCE = 5 一回だけ反転させる
    sampDesc.AddressU       = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV       = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW       = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;   // サンプリングされたデータの比較方法
    sampDesc.MinLOD         = 0;
    sampDesc.MaxLOD         = D3D11_FLOAT32_MAX;
    sampDesc.MipLODBias     = 0.0f;
    sampDesc.MaxAnisotropy  = 16;
    // 作成
    hr = m_pd3dDevice->CreateSamplerState(&sampDesc, &m_pSamplerLinear);
    if (FAILED(hr))return hr;

    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    hr = m_pd3dDevice->CreateSamplerState(&sampDesc, &m_pClampShadow);
    if (FAILED(hr))return hr;

    // シャドウマップ用サンプラー
    //sampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
    //sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
    sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    // 境界線の色を「1.0（一番遠い深度）」に設定する
    sampDesc.BorderColor[0] = 1.0f;
    sampDesc.BorderColor[1] = 1.0f;
    sampDesc.BorderColor[2] = 1.0f;
    sampDesc.BorderColor[3] = 1.0f;
    sampDesc.MaxAnisotropy  = 1;
    hr = m_pd3dDevice->CreateSamplerState(&sampDesc, &m_pSamplerShadow);
    if (FAILED(hr))return hr;

    return hr;
}

//*---------------------------------------------------------------------------------------
//* @:RendererEngine Class 
//*【?】リソース解放
//* 引数：なし
//* 戻値：void
//*----------------------------------------------------------------------------------------
void RendererEngine::CleanupDX11()
{
    if (m_pImmediateContext) m_pImmediateContext->ClearState();
    m_pFrameBufferSRV.Reset();
    m_pFrameBufferTexture.Reset();
    SAFE_RELEASE(m_pImmediateContext);
    SAFE_RELEASE(m_pRasterState_NoneCull);
    SAFE_RELEASE(m_pRasterState_FrontCull);
    SAFE_RELEASE(m_pRasterState_BackCull);
    SAFE_RELEASE(m_pDepthStencilState);
    SAFE_RELEASE(m_pSamplerLinear);
    SAFE_RELEASE(m_pSamplerShadow);
    SAFE_RELEASE(m_pVertexBuffer);
    SAFE_RELEASE(m_pDepthStencil);
    SAFE_RELEASE(m_pDepthStencilView);
    SAFE_RELEASE(m_pRenderTargetView);
    SAFE_RELEASE(m_pSwapChain);
    SAFE_RELEASE(m_pd3dDevice)
}

//*---------------------------------------------------------------------------------------
//*【?】メインカメラフラスタムの更新
//*
//* [引数] なし
//* [返値] なし
//*----------------------------------------------------------------------------------------
void RendererEngine::UpdateMainCameraFrustum()
{
    DirectX::BoundingFrustum viewFrustum;

    // 透視投影行列からフラスタムを作成
    DirectX::BoundingFrustum::CreateFromMatrix(viewFrustum, m_Proj);

    const DirectX::XMMATRIX invView =
        DirectX::XMMatrixInverse(nullptr, m_View);

    viewFrustum.Transform(m_MainCameraFrustum, invView);
}


//*---------------------------------------------------------------------------------------
//*【?】ワールド座標をスクリーン座標へ変換
//*
//* [引数]
//* const &_world : ワールド座標
//* &_screen：出力先
//* [返値]
//* true : 成功
//* false : 失敗
//*----------------------------------------------------------------------------------------
bool RendererEngine::WorldToScreen(const VECTOR3::VEC3 &_world, VECTOR2::VEC2 &_screen)
{
    XMVECTOR worldVec = _world;
    XMMATRIX viewProj = m_View * m_Proj;

    // XMVector3TransformCoordは内部でw除算も行ってくれているらしい
    XMVECTOR clipSpaceVec = XMVector3TransformCoord(worldVec, viewProj);

    XMFLOAT3 ndc;
    XMStoreFloat3(&ndc, clipSpaceVec);

    return true;
}

//*---------------------------------------------------------------------------------------
//*【?】被写界深度の設定
//*
//* [引数]
//* _maxRange   : 開始距離
//* _minRange   : 最大距離
//* [返値] なし
//*----------------------------------------------------------------------------------------
void RendererEngine::set_DofParam(float _minRange, float _maxRange)
{
    CB_Dof param;
    param.dof_MaxRange = _maxRange;
    param.dof_MinRange = _minRange;
    m_pRendererPipeline->set_DofParam(param);
}
//*---------------------------------------------------------------------------------------
//*【?】フォグの設定
//*
//* [引数]
//* _color      : フォグカラー
//* _startRange : 開始位置
//* _endRange   : 終了位置
//* [返値] なし
//*----------------------------------------------------------------------------------------
void RendererEngine::set_FogParam(const VECTOR3::VEC3& _color, float _startRange, float _endRange)
{
    CB_Fog param;
    param.Color = _color;
    param.Start = _startRange;
    param.End = _endRange;
    m_pRendererPipeline->set_FogParam(param);
}
//*---------------------------------------------------------------------------------------
//*【?】カラーグレーディングの設定
//*
//* [引数]
//* _exposure   : 露出補正
//* _contrast   : コントラスト
//* _saturation : 彩度
//* _colorTint  : 色味調整
//* [返値] なし
//*----------------------------------------------------------------------------------------
void RendererEngine::set_ColorGradingParam(float _exposure, float _contrast, float _saturation, const VECTOR3::VEC3& _colorTint)
{
    CB_ColorGrading param;
    param.Exposure = _exposure;
    param.Contrast = _contrast;
    param.Saturation = _saturation;
    param.ColorTint = _colorTint;
    m_pRendererPipeline->set_ColorGradingParam(param);
}

//*---------------------------------------------------------------------------------------
//* @:RendererEngine Class 
//*【?】ビューポートの設定
//* 引数：1.描画範囲の左上Ｘ座標、
//* 引数：2.描画範囲の左上Ｙ座標
//* 引数：3.描画範囲の横幅
//* 引数：4.描画範囲の縦幅
//* 戻値：void
//*----------------------------------------------------------------------------------------
void RendererEngine::set_ViewPort(float _topLeftX, float _topLeftY, float _width, float _height)
{
    D3D11_VIEWPORT vp;
    ZeroMemory(&vp, sizeof(vp));
    vp.Width    = (FLOAT)_width;        // 描画範囲の横幅       
    vp.Height   = (FLOAT)_height;       // 描画範囲の縦幅
    vp.MinDepth = 0.0f;                 // Ｚバッファの最小値
    vp.MaxDepth = 1.0f;                 // Ｚバッファの最大値
    vp.TopLeftX = (FLOAT)_topLeftX;     // 描画範囲の左側Ｘ座標
    vp.TopLeftY = (FLOAT)_topLeftY;     // 描画範囲の上部Ｙ座標
    m_pImmediateContext->RSSetViewports(1, &vp);


    m_CrntViewPort = vp;
}

//*---------------------------------------------------------------------------------------
//*【?】現在の描画パスの取得
//* [引数]
//* なし
//* [返値]
//* 描画ぱす
//*----------------------------------------------------------------------------------------
RENDER_PASS RendererEngine::get_CrntRenderPass()const
{
    return m_CrntRenderPass;
}

//*---------------------------------------------------------------------------------------
//*【?】現在の描画パスの設定
//* [引数]
//* pass : 描画パス
//* [返値]
//* void
//*----------------------------------------------------------------------------------------
void RendererEngine::set_CrntRenderPass(RENDER_PASS pass)
{
    m_CrntRenderPass = pass;
}

//*---------------------------------------------------------------------------------------
//*【?】カリングモードの設定
//*
//* ※以下の三つがある
//*    ・カリングなし
//*    ・表カリング
//*    ・裏カリング
//* 
//* [引数]
//* mode : カリングモード 
//* [返値]
//* void
//*----------------------------------------------------------------------------------------
void RendererEngine::RegisterCullMode(CULL_MODE mode)
{
    switch (mode)
    {
    case CULL_MODE::NONE:
        m_pImmediateContext->RSSetState(m_pRasterState_NoneCull);
        break;
    case CULL_MODE::FRONT:
        m_pImmediateContext->RSSetState(m_pRasterState_FrontCull);
        break;
    case CULL_MODE::BACK:
        m_pImmediateContext->RSSetState(m_pRasterState_BackCull);
        break;
    default:
        break;
    }
}

//*---------------------------------------------------------------------------------------
//* @:RendererEngine Class 
//*【?】レンダリングパイプラインのセットアップ
//* 引数：なし
//* 戻値：成功か
//*----------------------------------------------------------------------------------------
bool RendererEngine::CreateRendererPipeline(RENDER_PIPELINE_STATE type)
{
    switch (type)
    {
    case RENDER_PIPELINE_STATE::DEFAULT:
        // レンダーパイプラインの作成
        m_pRendererPipeline = new RenderPipeline();

        // レンダーパイプライン初期化
        if (!m_pRendererPipeline->Setup(*this))
        {
            return false;
        }
        break;
    default:
        return false;
        break;
    }

    return true;
}

//*---------------------------------------------------------------------------------------
//*【?】デフォルトレンダリングパイプラインの実行
//*
//* [引数]
//* type : パイプラインの種類
//* *pCam : カメラコンポーネント
//*
//* [返値]
//* true : 成功
//* false : 失敗
//*----------------------------------------------------------------------------------------
void RendererEngine::ExecuteDefaultRendererPipeline(RENDER_PIPELINE_STATE type, const Camera3D* pCam)
{
    float farClip = pCam->get_Far();
    float nearClip = pCam->get_Near();
    float fov = pCam->get_Fov();

    float screen_width = static_cast<float>(get_ScreenWidth());
    float screen_height = static_cast<float>(get_ScreenHeight());

    // プロジェクション変換行列の設定 一回でいいけど、今回はfovなど編集できるようにしているので...
    SetupProjectionTransform(screen_width, screen_height, fov, nearClip, farClip);

    // ビュー行列の更新
    auto viewMatrix = pCam->get_ViewMatrix();

    // ビュー変換し定数バッファへ送る
    if (!SetupViewTransform(viewMatrix)) {
        MessageBoxA(NULL, "ビュー行列を定数バッファに送信できませんでした", "Error", MB_OK);
        return;
    };

    // メインカメラフラスタムの更新
    UpdateMainCameraFrustum();

    // パイプライン実行
    m_pRendererPipeline->Execute(*this);
}

//*---------------------------------------------------------------------------------------
//* @:RendererEngine Class 
//*【?】透視投影変換計算  ※カメラ処理に組み込んだ方が良さそう
//* 引数：
//* _w      : 横幅
//* _h      : 縦幅
//* _fovDeg : 視野（中でラジアン化しているのでデグリーでいい） 
//* _near   : 手前クリップ
//* _far    : 奥クリップ
//* 戻値：成功したか
//*----------------------------------------------------------------------------------------
bool RendererEngine::SetupProjectionTransform(float _w, float _h, float _fovDeg, float _near, float _far)
{
    auto pDeviceContext = get_DeviceContext();
    float aspect = _w / _h; // アスペクト比

    // 遠近投影行列作成 https://learn.microsoft.com/ja-jp/windows/win32/api/directxmath/nf-directxmath-xmmatrixperspectivefovlh
    XMMATRIX psProj = XMMatrixPerspectiveFovLH(
        XMConvertToRadians(_fovDeg),
        aspect,
        _near,
        _far
    );

    // 透視投影保持
    m_Proj = psProj;

    psProj = XMMatrixTranspose(psProj);   // 転置

    // 正射投影行列作成
    // 左上原点 Y軸下向き
    XMMATRIX ohProjMat = XMMatrixOrthographicOffCenterLH(
        0.0f,  // ViewLeft
        _w,    // ViewRight
        _h,    // ViewBottom (下に高さを入れる)
        0.0f,  // ViewTop    (上に0を入れる)
       -1.0f,  // NearZ を -1.0f にする！ (0.0fだとZ=0の板がクリップされて消える可能性があるため)
        1.0f
    );
    ohProjMat = XMMatrixTranspose(ohProjMat);   // 転置

    CB_PROJECTION cbProjection{};

    // 透視投影行列
    XMStoreFloat4x4(&cbProjection.Projection, psProj);

    // 透視投影逆行列
    XMMATRIX psInvProj = XMMatrixInverse(NULL, psProj);
    XMStoreFloat4x4(&cbProjection.InvProjection, psInvProj);
    // 正射投影行列
    XMStoreFloat4x4(&cbProjection.OrthographicProjection, ohProjMat);

    /* 定数バッファにセット */
    Master::m_pShaderManager->BindConstantBuffer(CONSTANT_BUFFER_TYPE::PROJECTION, (void*)&cbProjection, sizeof(CB_PROJECTION));

    //// サブリソース
    //D3D11_MAPPED_SUBRESOURCE mappedResource;
    //ZeroMemory(&mappedResource, sizeof(mappedResource));

    //// CPUアクセス
    //HRESULT hr = pDeviceContext->Map(
    //    cb.pBuff,
    //    0,
    //    D3D11_MAP_WRITE_DISCARD,
    //    0,
    //    &mappedResource
    //);

    //if (FAILED(hr))return false;

    //CopyMemory(mappedResource.pData, &cb.Data, sizeof(cb.Data));

    //// マップ解除
    //pDeviceContext->Unmap(cb.pBuff, 0);

    //// VSにProjectionMatrixをセット(ここで1度セットして以後不変)
    //pDeviceContext->VSSetConstantBuffers(2, 1, &cb.pBuff);

    //// PSにも
    //pDeviceContext->PSSetConstantBuffers(2, 1, &cb.pBuff);

    return true;
}


//*---------------------------------------------------------------------------------------
//* @:RendererEngine Class 
//*【?】ビュー変換更新
//* 引数：ビュー行列
//* 戻値：成功したか
//*----------------------------------------------------------------------------------------
bool RendererEngine::SetupViewTransform(const XMMATRIX& viewMat)
{
    auto pDeviceContext = get_DeviceContext();

    // ビュー行列保持
    m_View = viewMat;

    CB_VIEW cbView{ };
    XMStoreFloat4x4(&cbView.View, XMMatrixTranspose(viewMat));

    // ビュープロジェクション逆行列取得
    cbView.ViewProjInvMatrix = get_ViewProjectionInvMatrix();

    /* 定数バッファにセット */
    Master::m_pShaderManager->BindConstantBuffer(CONSTANT_BUFFER_TYPE::VIEW, (void*)&cbView, sizeof(CB_VIEW));


    //D3D11_MAPPED_SUBRESOURCE mappedResource;
    //ZeroMemory(&mappedResource, sizeof(mappedResource));
    //// CPUアクセス
    //HRESULT hr = pDeviceContext->Map(
    //    cb.pBuff,
    //    0,
    //    D3D11_MAP_WRITE_DISCARD,
    //    0,
    //    &mappedResource
    //);

    //if (FAILED(hr))return false;

    //CopyMemory(mappedResource.pData, &cb.Data, sizeof(cb.Data));

    //// マップ解除
    //pDeviceContext->Unmap(cb.pBuff, 0);

    //// VSにViewMatrixをセット
    //pDeviceContext->VSSetConstantBuffers(1, 1, &cb.pBuff);

    //// ピクセルシェーダにもセット！！
    //pDeviceContext->PSSetConstantBuffers(1, 1, &cb.pBuff);

    return true;
}

//*---------------------------------------------------------------------------------------
//* @:RendererEngine Class 
//*【?】プロジェクション行列を取得
//* 引数：なし
//* 戻値：XMMATRIX
//*----------------------------------------------------------------------------------------
XMMATRIX RendererEngine::get_ProjectionMatrix()const
{
    return m_Proj;
}

//*---------------------------------------------------------------------------------------
//* @:RendererEngine Class 
//*【?】ビュープロジェクション行列を取得
//* 引数：なし
//* 戻値：XMMATRIX
//*----------------------------------------------------------------------------------------
XMMATRIX RendererEngine::get_ViewProjectionMatrix() const
{
    XMMATRIX view = m_View;
    XMMATRIX proj = m_Proj;
    return XMMatrixMultiply(view, proj);
}


//*---------------------------------------------------------------------------------------
//* @:RendererEngine Class 
//*【?】ビュープロジェクション行列の逆行列取得
//* 引数：なし
//* 戻値：XMFLOAT4X4
//*----------------------------------------------------------------------------------------
XMFLOAT4X4 RendererEngine::get_ViewProjectionInvMatrix()const
{
    XMMATRIX vp = get_ViewProjectionMatrix();
    XMFLOAT4X4 res{};

    // 転置逆行列
    XMStoreFloat4x4(&res, XMMatrixTranspose(XMMatrixInverse(NULL, vp)));
    return res;
}


//*---------------------------------------------------------------------------------------
//* @:RendererEngine Class 
//*【?】ビュー行列の行列取得
//* 引数：なし
//* 戻値：行列
//*----------------------------------------------------------------------------------------
XMMATRIX RendererEngine::get_ViewMatrix()const
{
    return m_View;
}

//*---------------------------------------------------------------------------------------
//* @:RendererEngine Class 
//*【?】ビュー行列の逆行列取得
//* 引数：なし
//* 戻値：逆行列
//*----------------------------------------------------------------------------------------
XMMATRIX RendererEngine::get_ViewInvMatrix()const
{
    return XMMatrixInverse(NULL, m_View);
}


//*---------------------------------------------------------------------------------------
//* @:RendererEngine Class 
//*【?】レンダーターゲットをフレームバッファに変更
//* 引数：なし
//* 戻値：void
//*----------------------------------------------------------------------------------------
void RendererEngine::ChangeRenderTargetFrameBuffer()
{
    m_pImmediateContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);
}

//*---------------------------------------------------------------------------------------
//* @:RendererEngine Class 
//*【?】レンダーターゲットをフレームバッファに変更
//      デプスステンシルびゅーを指定可能
//* 引数：なし
//* 戻値：void
//*----------------------------------------------------------------------------------------
void RendererEngine::ChangeRenderTargetFrameBuffer(ID3D11DepthStencilView* pDsv)
{
    m_pImmediateContext->OMSetRenderTargets(1, &m_pRenderTargetView, pDsv);
}


//*---------------------------------------------------------------------------------------
//* @:RendererEngine Class 
//*【?】レンダーターゲットを解除しNULL設定
//* 引数：なし
//* 戻値：void
//*----------------------------------------------------------------------------------------
void RendererEngine::ReleaseRenderTargetSetNull()
{
    m_pImmediateContext->OMSetRenderTargets(0, nullptr, nullptr);
}

//*---------------------------------------------------------------------------------------
//* @:RendererEngine Class 
//*【?】MRTレンダーターゲットの登録
//* 引数：1.ターゲットの数
//* 引数：2.ターゲットの配列
//* 引数：3.深度ステンシル
//* 戻値：void
//*----------------------------------------------------------------------------------------
void RendererEngine::RegisterRenderTargets(UINT num, class DX_RenderTarget *renderTargets[], ID3D11DepthStencilView *pDsv)
{
    ID3D11RenderTargetView *rtv[8]{};

    for (UINT i = 0; i < num; i++) {
        if (renderTargets[i]->get_RTV() != nullptr) {
            rtv[i] = renderTargets[i]->get_RTV();
        }
    }
    m_pImmediateContext->OMSetRenderTargets(num, rtv, pDsv);
}

//*---------------------------------------------------------------------------------------
//* @:RendererEngine Class 
//*【?】単一レンダーターゲットの登録
//* 引数：1.レンダーターゲットビュー
//* 引数：2.デプスステンシルビュー
//* 戻値：void
//*----------------------------------------------------------------------------------------
void RendererEngine::RegisterRenderTarget(ID3D11RenderTargetView* pRtv, ID3D11DepthStencilView* pDsv)
{
    m_pImmediateContext->OMSetRenderTargets(1, &pRtv, pDsv);
}


//*---------------------------------------------------------------------------------------
//* @:RendererEngine Class 
//*【?】単一のレンダーターゲットの登録とそのRTのに設定された大きさでビューポート設定
//* 引数：1.レンダーターゲット
//* 戻値：void
//*----------------------------------------------------------------------------------------
void RendererEngine::RegisterRenderTargetAndViewPort(class DX_RenderTarget* pRT)
{
    // ビューポートの設定
    this->set_ViewPort(0.0f, 0.0f, FLOAT_CAST(pRT->get_Width()), FLOAT_CAST(pRT->get_Height()));

    // 深度ビューあり
    if (pRT->HasDepthStencilBuffer()) {
        ID3D11RenderTargetView* rtv = pRT->get_RTV();
        ID3D11DepthStencilView* dsv = pRT->get_DSV();
        if (rtv == nullptr) {
            m_pImmediateContext->OMSetRenderTargets(0, nullptr , dsv);
        }
        else {
            m_pImmediateContext->OMSetRenderTargets(1, &rtv, dsv);
        }
    }
    // 深度ビューなし
    else
    {
        ID3D11RenderTargetView* rtv = pRT->get_RTV();
        m_pImmediateContext->OMSetRenderTargets(1, &rtv, NULL);
    }
}



//*---------------------------------------------------------------------------------------
//* @:RendererEngine Class 
//*【?】レンダーターゲットのクリア
//* 引数：1.ターゲットの数
//* 引数：2.ターゲットの配列
//* 戻値：void
//*----------------------------------------------------------------------------------------
void RendererEngine::ClearRenderTargetViews(UINT num, class DX_RenderTarget *renderTargets[])
{
    for (UINT i = 0; i < num; i++) 
    {
        // デプスステンシルバッファがあるならクリア 
        if (renderTargets[i]->HasDepthStencilBuffer()) {
            m_pImmediateContext->ClearDepthStencilView(renderTargets[i]->get_DSV(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0);
        }

        if (renderTargets[i]->get_RTV() != nullptr) {
            float col[4] = { 1.0,1.0,1.0,1.0 };

            // ターゲットのクリア
            m_pImmediateContext->ClearRenderTargetView(renderTargets[i]->get_RTV(), col);
        }
    }
}


//*---------------------------------------------------------------------------------------
//* @:RendererEngine Class 
//*【?】単一のレンダーターゲットのクリア
//* 引数：1.ターゲット
//* 戻値：void
//*----------------------------------------------------------------------------------------
void RendererEngine::ClearRenderTargetView(class DX_RenderTarget* pRT)
{
    if (pRT->HasDepthStencilBuffer()){
        // デプスステンシルバッファがあるならクリア
        m_pImmediateContext->ClearDepthStencilView(pRT->get_DSV(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0);
    }

    if (pRT->get_RTV() != nullptr) {
        float col[4] = {0.0,0.0,0.0,1.0};
        // ターゲットのクリア
        m_pImmediateContext->ClearRenderTargetView(pRT->get_RTV(), col);
    }
}


//*---------------------------------------------------------------------------------------
//* @:RendererEngine Class 
//*【?】デプスステンシルの登録
//* 引数：1.ID3D11DepthStencilState*
//* 戻値：void
//*----------------------------------------------------------------------------------------
void RendererEngine::RegisterDepthStencilState(ID3D11DepthStencilState* pDss, UINT stencilRef)
{
    m_pImmediateContext->OMSetDepthStencilState(pDss, stencilRef);
}



//*---------------------------------------------------------------------------------------
//* @:RendererEngine Class 
//*【?】RendererEngine内で定義しているデフォルトのものを設定する
//* 引数：なし
//* 戻値：デフォルトの深度ステンシルを登録
//*----------------------------------------------------------------------------------------
void RendererEngine::RegisterDefaultDepthStencilState(UINT _ref)
{
    m_pImmediateContext->OMSetDepthStencilState(m_pDepthStencilState, _ref);
}



//*---------------------------------------------------------------------------------------
//* @:RendererEngine Class 
//*【?】深度書き込みなしの深度ステンシルを取得
//* 引数：なし
//* 戻値：ID3D11DepthStencilState *
//*----------------------------------------------------------------------------------------
ID3D11DepthStencilState* RendererEngine::get_DepthWriteDisabled_DSS()const
{
    return m_pDepthWriteDisabled_DSS;
}

//*---------------------------------------------------------------------------------------
//* @:RendererEngine Class 
//*【?】デカール用の深度ステンシルを取得
//* 引数：なし
//* 戻値：ID3D11DepthStencilState *
//*----------------------------------------------------------------------------------------
ID3D11DepthStencilState *RendererEngine::get_Decal_DSS()const
{
    return m_pDecal_DSS;
}
