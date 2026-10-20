#pragma once



// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:RenderPipeline Class --- */
//
// 【?】レンダリングの一連の流れを管理するクラス
//      RendererEngineが実体を持つ
//
// ***************************************************************************************
class RenderPipeline
{
private:
    // Gバッファ用
    class DX_RenderTarget *m_pAlbedo_RT;        // アルベド用
    class DX_RenderTarget *m_pNormal_RT;        // 法線用
    class DX_RenderTarget *m_pDepth_RT;         // 深度用
    class DX_RenderTarget *m_pSpecular_RT;      // スペキュラ用
    class DX_RenderTarget *m_pEmissive_RT;      // エミッシブ用
    
    class DX_RenderTarget *m_pSceneFinal_RT;	// シーン最終合成用
    class DX_RenderTarget *m_pLuminance_RT;		// 輝度抽出用
    class DX_RenderTarget *m_pShadowMap_RT;		// シャドウマップ（ライトから見た深度を書き込む）
    
	std::weak_ptr<class SkyRenderer> m_pSkyRenderer;  // スカイボックス描画用

    class DX_RenderTarget *m_pSceneCopy_RT;	

    // シャドウマップの解像度
    const float SHADOW_SIZE_X = 4096.0f;    
    const float SHADOW_SIZE_Y = 4096.0f;

    static const int NUM_WEIGHTS = 8;
    float m_GaussWeights[NUM_WEIGHTS];  // ガウス重み係数

    static const int BLUR_COUNT = 4;         // ブラー回数（ブルーム用）
    class GaussianBlur *m_pBloomGaussianBlur;// ブルーム用ガウスブラー
    float m_Bloom_BlurIncensity;             // ブルームぼかしの強さ

    class GaussianBlur *m_pDoF_GaussianBlur; // 被写界深度用ガウスブラー
    float m_DoF_BlurIncensity;               // DOFブラーの強さ

    class GaussianBlur *m_pShadowGaussianBlur; // シャドウのVSM用ガウスブラー

    // レンダーターゲットのスプライト
    std::shared_ptr<class SpriteRenderer> m_pAlbed_Sprite;
    std::shared_ptr<class SpriteRenderer> m_pNormal_Sprite;
    std::shared_ptr<class SpriteRenderer> m_pSpecular_Sprite;
    std::shared_ptr<class SpriteRenderer> m_pEmissive_Sprite;
    std::shared_ptr<class SpriteRenderer> m_pDepth_Sprite;
    std::shared_ptr<class SpriteRenderer> m_pDefferdLighting_Sprite;
    std::shared_ptr<class SpriteRenderer> m_pLuminance_Sprite;
    std::shared_ptr<class SpriteRenderer> m_pBloom_Sprite;
    std::shared_ptr<class SpriteRenderer> m_pDoF_Sprite;
    std::shared_ptr<class SpriteRenderer> m_pLensFlare_Sprite;
    std::shared_ptr<class SpriteRenderer> m_pFinalSceneToneMappingFilter_Sprite;

    // ポストエフェクト情報
    CB_POSTEFFECT m_PostEffectParam;

    // シャドウバイアス情報
    CB_SHADOW m_ShadowParam;


    // ビューポートの設定時にスムーズに渡す用
    float m_ScreenWidth;
    float m_ScreenHeight;

public:
    RenderPipeline();
    ~RenderPipeline();

    /// <summary>
    /// パイプラインを作成する
    /// </summary>
    /// <returns>正常に終了したか</returns>
    bool Setup(RendererEngine& renderer);

    /// <summary>
    /// パイプラインの実行
    /// </summary>
    void Execute(RendererEngine &renderer);

    /// <summary>
    /// 解放
    /// </summary>
    void Release();

    void set_DofParam(const CB_Dof& _param);
    void set_FogParam(const CB_Fog& _param);
    void set_ColorGradingParam(const CB_ColorGrading& _param);

private:
    //
    // 各描画パスの処理
    //
    void Shadow_PathRender(RendererEngine &renderer);               // シャドウ
    void Geometry_PathRender(RendererEngine &renderer);             // G-Buffer
    void Decal_PathRender(RendererEngine &renderer);                // デカール
    void Lighting_PathRender(RendererEngine &renderer);             // ディファードライティング
    void Skybox_PathRender(RendererEngine &renderer);               // スカイボックス
    void Dof_PathRender(RendererEngine& renderer);                  // 被写界深度
    void Forward_PathRender(RendererEngine &renderer);              // フォワードレンダリング
    void Bloom_PathRender(RendererEngine& renderer);                // ブルーム
    void CopyToFrameBuffer_PathRender(RendererEngine &renderer);    // フレームバッファへ描画＆トーンマッピング

    /// <summary>
    /// ImGuiを使用したレンダーターゲットのデバッグ表示
    /// </summary>
    void DebugRenderTargetImGui();

    /// <summary>
    /// レンダーターゲットの作成
    /// </summary>
    /// <param name="renderer"></param>
    /// <returns></returns>
    bool CreateRenderTargets(RendererEngine &renderer);

    /// <summary>
    /// ポストエフェクトの作成
    /// </summary>
    /// <param name="renderer"></param>
    /// <returns></returns>
    bool CreatePostEffect(RendererEngine &renderer);

    /// <summary>
    /// レンダーターゲット用スプライトの作成
    /// </summary>
    /// <param name="renderer"></param>
    /// <returns></returns>
    bool CreateRenderTargetSprites(RendererEngine &renderer);
};

