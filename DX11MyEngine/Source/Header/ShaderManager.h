#pragma once

/*------------------------------------------*/
// 前方宣言
/*------------------------------------------*/
class PixelShader;
class VertexShader;
class InputLayout;


/* =========================================================================
/* - @:SHADER_TYPE 列挙体 Class - */
/* 【?】シェーダの切り替え用
/* =========================================================================*/
enum class SHADER_TYPE
{
    NONE,       // なし

    //* **********************************************
    //* 【命名規則的なやつ】
    //* 
    //* STD   : 標準ライティング
    //* UNLIT : ライティングなし
    //* _N    : 法線マップあり
    //* 
    //* ◆    : 未実装
    //* _     : ...
    //* **********************************************

    //スキニング用はボーンが入ってないと動かないよ

    /////////////////////////////////////////////////////////////////////
    // ディファードシェーディング **************************************
    // 不透明オブジェクト用
    DEFERRED_STD_STATIC,                // 簡易静的3Dオブジェクト * 法線マップなし        
    DEFERRED_STD_STATIC_N,              // 静的3Dオブジェクト * 法線マップあり
    _DEFERRED_STD_SKINNED,              // ◆ スキニング3Dモデル * 法線マップなし
    DEFERRED_STD_SKINNED_N,             // スキニング3Dモデル * 法線マップあり     
    DEFERRED_STD_RT_SPRITE,             // RT描画用スプライト        
    _DEFERRED_STD_BILLBOARD,            // ◆ ビルボード                  
    DEFERRED_STD_TRAIL,                 // 軌跡                  
    DEFERRED_STD_DECAL,                 // デカール                  

    /////////////////////////////////////////////////////////////////////
    // フォワードシェーディング ****************************************
    // UIやαブレンドがあるオブジェクト用
    _FORWARD_STD_STATIC,                // ◆ 簡易3Dオブジェクト
    FORWARD_UNLIT_UI_SPRITE,            // スプライト 標準 UI用  ライティングなし
    FORWARD_UNLIT_UI_NOTEXTURE_SPRITE,  // スプライト 標準 UI用  ライティングなし テクスチャ無し
    _FORWARD_STD_BILLBOARD,             // ◆ ビルボード
    FORWARD_UNLIT_STATIC,               // 簡易3Dオブジェクト ライティングなし
    FORWARD_UNLIT_WATER,                // 水用 ライティングなし
    FORWARD_UNLIT_TRAIL,                // 軌跡 ライティングなし

    /////////////////////////////////////////////////////////////////////
    // その他 ポストエフェクト的なやつとか ******************************
    POST_GAUSSIAN_BLUR_HORIZONTAL,      // ガウシアン水平ブラー 
    POST_GAUSSIAN_BLUR_VERTICAL,        // ガウシアン垂直ブラー 
    POST_SKYBOX,                        // スカイボックス用
    POST_LUMINANCE_FILTER,              // 輝度抽出用
    POST_KAWASE_FILTER,                 // 川瀬式ブルーム（ダウンサンプリングしたガウス適用後のテクスチャをぼかす）
    POST_SHADOWMAP,                     // シャドウマップ
    POST_SHADOWMAP_SKINNED,             // シャドウマップスキニング用
    POST_DEPTH_OF_FILED,                // 被写界深度
    POST_TONEMAPPING,                   // トーンマッピング（ACES使用）
    POST_DISTORTION,                    // ディストーション
    
    /* 旧仕様 */
    //SIMPLE,     // 単純な3Dオブジェクト表示用
    //MODEL,      // 3Dモデルの表示用
    //SPRITE,     // スプライト
    //DEFFERD,    // ディファードシェーディング

    NUM,
};


//* =========================================================================
//* - @:シェーダの作成方法列挙体 - */
//* =========================================================================
enum class SHADER_CREATE_TYPE
{
    RUNTIME,    // ランタイム実行
    CSO,        // コンパイル済みシェーダ読み込み
};

/* =========================================================================
/* - @:ShaderInfo構造体 - */
/* 【?】シェーダ機能をまとめたもの
/* =========================================================================*/
struct ShaderInfo
{
    SHADER_TYPE Type;
    std::shared_ptr<PixelShader> pPShader;
    std::shared_ptr<VertexShader> pVShader;
    std::shared_ptr<InputLayout> pInputLayout;
};


//* =========================================================================
//* - @:InputLayoutData構造体 - */
//* 【?】入力レイアウトセットアップ用のデータをまとめたもの
//* =========================================================================
struct InputLayoutSetupData
{
    SHADER_TYPE type;
    WORD LayoutArraySize;   // 配列サイズ
    const D3D11_INPUT_ELEMENT_DESC *pLayout;  // レイアウト
};

//* ==========================================================================
//* - @:CONSTANT_BUFFER_TYPE列挙体 Class - */
//* 【?】定数バッファの種類
//*      レジスタスロットはこの順番で揃える 
//* ==========================================================================
enum class CONSTANT_BUFFER_TYPE
{
    TRANSFORM = 0,          // ワールド行列
    VIEW,                   // ビュー行列
    PROJECTION,             // プロジェクション行列
	BONE,                   // スキニング用ボーン行列
	MATERIAL,   	        // マテリアル情報
	LIGHTS,                 // ライト
	BLUR_WEIGHTS,			// ブラーの重み
	POSTEFFECT,             // ポストエフェクト用
	SHADOW,				    // シャドウマップ用
	SPRITE,                 // スプライト用
	DECAL,  		        // デカール用  
	WINDOW, 			    // ウインドウ情報用
	DISTORTION,             // ディストーション用
	FRAME,                  // 毎フレーム代わるデータ用

    NUM,
};

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:ShaderManager Class --- */
//
//  ★★★シングルトン★★★
//
// 【?】シェーダの作成、セットなどを行う
//      シェーダクラスの所有権を持つ
//
// ***************************************************************************************
class ShaderManager
{
private:
    std::vector<ShaderInfo> m_ShaderList;
    std::weak_ptr<class RendererEngine> m_pRenderer;    // 描画クラスの弱参照を持つ
    std::vector<InputLayoutSetupData> m_InputLayoutSetupDataList;
    std::array<std::unique_ptr<IConstantBuffer>, UINT_CAST(CONSTANT_BUFFER_TYPE::NUM)> m_ConstantBuffers;    // 定数バッファの所有権を持つ

public:
    ShaderManager() = default;
    ~ShaderManager();

    /// <summary>
    /// 初期化
    /// </summary>
    /// <returns>初期化出来たか</returns>
    bool Init(std::shared_ptr<class RendererEngine> renderer);

    /// <summary>
    /// シェーダの作成
    /// </summary>
    /// <param name="type">シェーダタイプ</param>
    /// <returns>作成出来たか</returns>
    bool CreateShader(SHADER_TYPE type, SHADER_CREATE_TYPE createType);

    /// <summary>
    /// デバイスにシェーダをセット
    /// </summary>
    /// <param name="type">セットするシェーダ種別</param>
    void DeviceToSetShader(SHADER_TYPE type);

    /// <summary>
    /// 頂点・ピクセル両方にnull設定
    /// </summary>
    void NullSetAllShader();

	/// <summary>
	/// 定数バッファの更新＆セット
	/// </summary>
	/// <param name="type">種類</param>
	/// <param name="_data">データ</param>
	/// <param name="_size">データサイズ</param>
	void BindConstantBuffer(CONSTANT_BUFFER_TYPE type, const void* _data, UINT _size);

    
    void Term();

private:
    // コピー禁止
    ShaderManager(const ShaderManager &) = delete;
    ShaderManager &operator=(const ShaderManager &) = delete;
    // ------------------------------------------------------



    /// <summary>
    /// シェーダのコンパイル
    /// </summary>
    /// <param name="szFileName">ファイル名</param>
    /// <param name="szEntryPoint">シェーダのエントリーポイント</param>
    /// <param name="szShaderModel">シェーダモデル</param>
    /// <param name="ppBlobOut"></param>
    /// <returns>コンパイル出来たか</returns>
    HRESULT CompileShader(LPCWSTR szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob **ppBlobOut);
    bool VertexShaderFactory(SHADER_TYPE type, ShaderInfo *out, SHADER_CREATE_TYPE createType);
    bool InputLayoutFactory(SHADER_TYPE type, ShaderInfo *out, ID3DBlob *blob);
    bool InputLayoutFactory_CSO(SHADER_TYPE type, ShaderInfo *out, std::vector<uint8_t> &byteCode);
    bool PixelShaderFactory(SHADER_TYPE type, ShaderInfo* out, SHADER_CREATE_TYPE createType);

    /// <summary>
    /// .csoファイルの読み込み
    /// </summary>
    /// <returns></returns>
    bool LoadCSOFile(const wchar_t *csoName, std::vector<uint8_t> *pByteCodeOUT);
};

