#pragma once
//--------------------------------------------------------------------------------------
//      * Includes *
//--------------------------------------------------------------------------------------
//using namespace DirectX;
#include "ConstantRenderData.h"

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:RendererEngine Class --- */
//
// 【?】描画関連(dirextX)の処理をまとめたもの
//
// ***************************************************************************************
class RendererEngine
{
private:
    D3D_DRIVER_TYPE                           m_driverType;
    D3D_FEATURE_LEVEL                         m_featureLevel;           // DirectXのバージョン
    ID3D11Device                            * m_pd3dDevice;             // GPUを抽象化してくれる
    ID3D11DeviceContext                     * m_pImmediateContext;      // 絵を描く部分を抽象化したもの
    IDXGISwapChain                          * m_pSwapChain;             // バッファの入れ替え(バッファ数増やすと入力遅延が起きる)
    ID3D11RenderTargetView                  * m_pRenderTargetView;      // 絵を描く領域を切り替える(メモリ領域を覗く、眼鏡みたいなもの)
    ID3D11Texture2D                         * m_pDepthStencil;          // 画像を読み込んで使えるようにするもの
    ID3D11DepthStencilView                  * m_pDepthStencilView;      // 深度バッファ(ZBuffer)奥行き
    ID3D11DepthStencilState                 * m_pDepthStencilState;      // Z比較をするための設定
    ID3D11DepthStencilState                 * m_pDepthWriteDisabled_DSS; // Z比較をしないための設定
    ID3D11DepthStencilState                 *m_pDecal_DSS;               // デカール用の深度ステンシル

    ID3D11SamplerState                      * m_pSamplerLinear;         // テクスチャからどうピクセルをもらうか、サンプルをどうするか
    ID3D11SamplerState                      * m_pSamplerShadow;         // シャドウマップ用サンプラー
    ID3D11SamplerState                      * m_pClampShadow;           // クランプ用サンプラー

    // どこを塗るのか決める(実際には塗るのはピクセルシェーダ)
    ID3D11RasterizerState                   * m_pRasterState_NoneCull;           // カリングなし
    ID3D11RasterizerState                   * m_pRasterState_FrontCull;          // 表カリング
    ID3D11RasterizerState                   * m_pRasterState_BackCull;           // 裏カリング

	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_pFrameBufferTexture;      // エディタ用ゲーム画面コピー
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pFrameBufferSRV; // エディタ用ゲーム画面SRV

    DirectX::XMMATRIX m_Proj;
    DirectX::XMMATRIX m_View;
    ;

    ID3D11Buffer                            * m_pVertexBuffer;        // 頂点バッファ(実際の頂点のデータが詰まっている)
    ID3D11Buffer                            * m_pIndexBuffer;         // 頂点バッファ(実際の頂点のデータが詰まっている)

    D3D11_VIEWPORT m_CrntViewPort;  // 現在のビューポート設定
    //RenderParam m_RenderParam;  // 描画パラメータ

    UINT m_ScreenWidth; // ウインドウ横幅
    UINT m_ScreenHeight;// ウインドウ縦幅
    HWND m_hWnd;        // ウインドウハンドル

    DWORD m_StartTime;

    RenderData::RENDER_PASS m_CrntRenderPass;   // 現在の描画パス

    class RenderPipeline *m_pRendererPipeline;      // 描画パイプラインの実体を持つ
	CB_WINDOW_SET *m_pCBWindowSet;                  // ウインドウ情報の定数バッファセット

    DirectX::BoundingFrustum m_MainCameraFrustum;   // メインカメラフラスタム
public:
    RendererEngine();
    ~RendererEngine();

    bool Init(HWND hWnd);
	void BeginRender();
    void EndRender();
	// 描画済みバックバッファをGameパネル用テクスチャへ保存し、ImGuiの描画先を準備する
	void CaptureFrameBufferForEditor();
	void Term();
    void Swap();    // 裏表切り替え
    bool SetupProjectionTransform(float _w, float _h, float _fovDeg, float _near, float _far); //透視投影変換計算
    bool SetupOrthographicProjectionTransform(float _w, float _h,  float _near, float _far); //正射投影変換計算


private:
    bool InitDx11();                            // ＤＸ１１の初期化
    HRESULT InitDX11_SwapChain();               // ＤＸ１１ スワップチェイン初期化
    HRESULT InitDX11_RenderTargetView();        // ＤＸ１１ レンダーターゲットビューの初期化
    HRESULT InitDX11_ZBuff();                   // ＤＸ１１ Ｚバッファ初期化
    HRESULT InitDX11_Rasterizer();              // ＤＸ１１ ラスタライザ初期化
    HRESULT InitDX11_Sampler();                 // ＤＸ１１ サンプラー初期化
    //HRESULT InitDX11_BlendState();            // ＤＸ１１ ブレンドステート初期化
    void CleanupDX11();                         // ＤＸ１１ 開放
    void EnableDebugLayer();                    // デバッグレイヤーの有効化


    void UpdateMainCameraFrustum(); // メインカメラフラスタムの更新

public:
    inline ID3D11Device* get_Device()const { return m_pd3dDevice; }                         // デバイス取得
    inline ID3D11DeviceContext* get_DeviceContext()const { return m_pImmediateContext; }    // デバイスコンテキスト取得
    inline IDXGISwapChain *get_SwapChain() const { return m_pSwapChain; }                   // スワップチェイン取得
    inline ID3D11SamplerState* get_Sampler() const { return m_pSamplerLinear; }             // サンプラー取得
    //inline RenderParam &get_RenderParam() { return m_RenderParam; }                       // 描画に必要な定数バッファ取得                        
    inline HWND get_WndHandle()const { return m_hWnd; }                                     // ウインドウハンドル取得
    inline UINT get_ScreenWidth()const { return m_ScreenWidth; };                       
    inline UINT get_ScreenHeight()const { return m_ScreenHeight; };
    inline Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> get_FrameBufferSRV_ComPtr() const { return m_pFrameBufferSRV; };
    D3D11_VIEWPORT get_CrntViewPort()const { return m_CrntViewPort; }                               // ビューポートの取得
    const DirectX::BoundingFrustum& get_MainCameraFrustum() const { return m_MainCameraFrustum; }   // メインカメラのフラスタム取得
    
    void set_DofParam(float _minRange, float _maxRange);                                                              // フォグの設定
    void set_FogParam(const VECTOR3::VEC3& _color, float _startRange, float _endRange);                               // 被写界深度の設定
    void set_ColorGradingParam(float _exposure, float _contrast, float _saturation, const VECTOR3::VEC3& _colorTint); // カラーグレーディングの設定

    /// <summary>
    /// ワールド座標をスクリーン座標へ変換
    /// </summary>
    /// <param name="_world">ワールド座標</param>
    /// <param name="_screen">スクリーン座標（出力先）</param>
    /// <returns></returns>
    bool WorldToScreen(const VECTOR3::VEC3 &_world, VECTOR2::VEC2 &_screen);

    /// <summary>
    ///  ビューポート設定
    /// </summary>
    /// <param name="_topLeftX"></param>
    /// <param name="_topLeftY"></param>
    /// <param name="_width"></param>
    /// <param name="_height"></param>
    void set_ViewPort(float _topLeftX, float _topLeftY, float _width, float _height);

    /// <summary>
    /// 現在の描画パスを取得
    /// </summary>
    /// <returns></returns>
    RenderData::RENDER_PASS get_CrntRenderPass()const;

    /// <summary>
    /// 描画パスを設定
    /// </summary>
    void set_CrntRenderPass(RenderData::RENDER_PASS pass);

    /// <summary>
    /// カリングモードの設定
    /// </summary>
    /// <param name="mode"></param>
    void RegisterCullMode(RenderData::CULL_MODE mode);

    void RegisterRendererPipeline(class RenderPipeline *pPipeline);

    /// <summary>
    /// レンダリングパイプラインのセットアップ
    /// </summary>
    /// <returns></returns>
    bool CreateRendererPipeline(RenderData::RENDER_PIPELINE_STATE type);

    /// <summary>
    /// デフォルトのレンダリングパイプラインを実行
    /// </summary>
    void ExecuteDefaultRendererPipeline(RenderData::RENDER_PIPELINE_STATE type, const class Camera3D* pCam);

    // ************************************************************************
    // 
    // レンダーターゲット関連
    //
    // ************************************************************************

    /// <summary>
    /// レンダーターゲットをフレームバッファに変更
    /// </summary>
    void ChangeRenderTargetFrameBuffer();

    /// <summary>
    /// レンダーターゲットをフレームバッファに変更
    /// デプスステンシルびゅーを指定可能
    /// </summary>
    /// <param name="pDsv"></param>
    void ChangeRenderTargetFrameBuffer(ID3D11DepthStencilView* pDsv);

    /// <summary>
    /// レンダーターゲットを解除しNULL設定
    /// </summary>
    void ReleaseRenderTargetSetNull();

    /// <summary>
    /// MRTレンダーターゲットを登録
    /// </summary>
    /// <param name="num"></param>
    /// <param name="renderTargets"></param>
    void RegisterRenderTargets(UINT num, class DX_RenderTarget *renderTargets[], ID3D11DepthStencilView *pDsv);
    
    /// <summary>
    /// 単一のレンダーターゲットを登録
    /// </summary>
    /// <param name="pRtv"></param>
    /// <param name="pDsv"></param>
    void RegisterRenderTarget(ID3D11RenderTargetView* pRtv, ID3D11DepthStencilView* pDsv);
    
    /// <summary>
    /// 単一のレンダーターゲットの登録とそのRTのに設定された大きさでビューポート設定
    /// </summary>
    /// <param name="pRtv"></param>
    /// <param name="pDsv"></param>
    void RegisterRenderTargetAndViewPort(class DX_RenderTarget* pRt);

    /// <summary>
    /// レンダーターゲットのクリア
    /// </summary>
    void ClearRenderTargetViews(UINT num, class DX_RenderTarget *renderTargets[]);

    /// <summary>
    /// 単一のレンダーターゲットのクリア
    /// </summary>
    void ClearRenderTargetView(class DX_RenderTarget *renderTargets);
    
    
    // ************************************************************************
    // 
    // ビュープロジェクション行列関連
    //
    // ************************************************************************

    /// <summary>
    /// ビューマトリクスを設定し更新
    /// </summary>
    /// <param name="viewMat"></param>
    /// <returns></returns>
    bool SetupViewTransform(const DirectX::XMMATRIX& viewMat);

    /// <summary>
    /// プロジェクション行列の取得
    /// </summary>
    /// <returns></returns>
    DirectX::XMMATRIX get_ProjectionMatrix()const;
    
    /// <summary>
    /// ビュープロジェクション行列の取得
    /// </summary>
    /// <returns></returns>
    DirectX::XMMATRIX get_ViewProjectionMatrix()const;

    /// <summary>
    /// ビュープロジェクション行列の逆行列取得
    /// </summary>
    /// <returns></returns>
    DirectX::XMFLOAT4X4 get_ViewProjectionInvMatrix()const;

    /// <summary>
    /// ビュー行列の取得
    /// </summary>
    /// <returns></returns>
    DirectX::XMMATRIX get_ViewMatrix()const;

    /// <summary>
    /// ビュー逆行列の取得
    /// </summary>
    /// <returns></returns>
    DirectX::XMMATRIX get_ViewInvMatrix()const;

    // ************************************************************************
    // 
    // 深度テスト関連
    //
    // ************************************************************************

    /// <summary>
    /// 深度書き込みなしの深度ステンシルを取得
    /// </summary>
    /// <returns></returns>
    ID3D11DepthStencilState*get_DepthWriteDisabled_DSS()const;

    /// <summary>
    /// デカール用の深度ステンシルを取得
    /// </summary>
    /// <returns></returns>
    ID3D11DepthStencilState*get_Decal_DSS()const;

    /// <summary>
    /// デプスステンシルステートの登録
    /// </summary>
    /// <param name="pDss"></param>
    /// <param name="stencilRef"></param>
    void RegisterDepthStencilState(ID3D11DepthStencilState* pDss, UINT stencilRef);

    /// <summary>
    /// デフォルトのデプスステンシルステートの登録
    /// </summary>
    void RegisterDefaultDepthStencilState(UINT _ref);
};

