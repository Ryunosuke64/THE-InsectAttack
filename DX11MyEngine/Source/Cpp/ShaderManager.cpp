#include "pch.h"
#include "RendererEngine.h"
#include "ShaderManager.h"
#include "PixelShader.h"
#include "VertexShader.h"
#include "InputLayout.h"
#include "Path.h"
#include <algorithm>

using namespace Path;
using namespace RenderData;
using namespace Microsoft::WRL;

//*****************************************************************
//
// セマンティクスがあっていれば頂点の入力に渡る
//  オフセットは一致してる必要がある。
// 
//*****************************************************************

// 法線マップなし簡易静的3Dオブジェクト用
static const D3D11_INPUT_ELEMENT_DESC g_Simple_Layout[] =
{
    {"POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, offsetof(VERTEX::VERTEX_Simple, pos),   D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXCOORD",   0, DXGI_FORMAT_R32G32_FLOAT,       0, offsetof(VERTEX::VERTEX_Simple, uv),    D3D11_INPUT_PER_VERTEX_DATA, 0},
};

// 法線マップなし簡易静的3Dオブジェクト用
static const D3D11_INPUT_ELEMENT_DESC g_Static_Layout[] =
{
    {"POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, offsetof(VERTEX::VERTEX_Static, pos),   D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXCOORD",   0, DXGI_FORMAT_R32G32_FLOAT,       0, offsetof(VERTEX::VERTEX_Static, uv),    D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR",      0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(VERTEX::VERTEX_Static, color), D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"NORMAL",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0, offsetof(VERTEX::VERTEX_Static, normal),D3D11_INPUT_PER_VERTEX_DATA, 0},
};

// 法線マップ有り静的3Dオブジェクト用
static const D3D11_INPUT_ELEMENT_DESC g_StaticTan_Layout[] =
{
    {"POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT,      0, offsetof(VERTEX::VERTEX_Static_N, pos),        D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXCOORD",   0, DXGI_FORMAT_R32G32_FLOAT,         0, offsetof(VERTEX::VERTEX_Static_N, uv),         D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR",      0, DXGI_FORMAT_R32G32B32A32_FLOAT,   0, offsetof(VERTEX::VERTEX_Static_N, color),      D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"NORMAL",     0, DXGI_FORMAT_R32G32B32_FLOAT,      0, offsetof(VERTEX::VERTEX_Static_N, normal),     D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TANGENT",    0, DXGI_FORMAT_R32G32B32_FLOAT,      0, offsetof(VERTEX::VERTEX_Static_N, tangent),    D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"BINORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,      0, offsetof(VERTEX::VERTEX_Static_N, bitangent),  D3D11_INPUT_PER_VERTEX_DATA, 0},
};

// スキニング用モデルシェーダ
static const D3D11_INPUT_ELEMENT_DESC g_Skneed_Layout[] =
{
    {"POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT,      0, offsetof(VERTEX::VERTEX_Skneed, pos),        D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXCOORD",   0, DXGI_FORMAT_R32G32_FLOAT,         0, offsetof(VERTEX::VERTEX_Skneed, uv),         D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR",      0, DXGI_FORMAT_R32G32B32A32_FLOAT,   0, offsetof(VERTEX::VERTEX_Skneed, color),      D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"NORMAL",     0, DXGI_FORMAT_R32G32B32_FLOAT,      0, offsetof(VERTEX::VERTEX_Skneed, normal),     D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TANGENT",    0, DXGI_FORMAT_R32G32B32_FLOAT,      0, offsetof(VERTEX::VERTEX_Skneed, tangent),    D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"BINORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,      0, offsetof(VERTEX::VERTEX_Skneed, bitangent),  D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"BONEIDS",    0, DXGI_FORMAT_R8G8B8A8_UINT,        0, offsetof(VERTEX::VERTEX_Skneed, boneIDs),    D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"BONEWEIGHTS",0, DXGI_FORMAT_R32G32B32A32_FLOAT,   0, offsetof(VERTEX::VERTEX_Skneed, boneWeights),D3D11_INPUT_PER_VERTEX_DATA, 0},
};

// インスタンシング用スキニングモデルレイアウト
static const D3D11_INPUT_ELEMENT_DESC g_InstanceSkneed_Layout[] =
{
    // --- スロット0: 頂点データ（モデル自体の形状情報） ---
    {"POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT,      0, offsetof(VERTEX::VERTEX_Skneed, pos),        D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXCOORD",   0, DXGI_FORMAT_R32G32_FLOAT,         0, offsetof(VERTEX::VERTEX_Skneed, uv),         D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"COLOR",      0, DXGI_FORMAT_R32G32B32A32_FLOAT,   0, offsetof(VERTEX::VERTEX_Skneed, color),      D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"NORMAL",     0, DXGI_FORMAT_R32G32B32_FLOAT,      0, offsetof(VERTEX::VERTEX_Skneed, normal),     D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TANGENT",    0, DXGI_FORMAT_R32G32B32_FLOAT,      0, offsetof(VERTEX::VERTEX_Skneed, tangent),    D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"BINORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,      0, offsetof(VERTEX::VERTEX_Skneed, bitangent),  D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"BONEIDS",    0, DXGI_FORMAT_R8G8B8A8_UINT,        0, offsetof(VERTEX::VERTEX_Skneed, boneIDs),    D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"BONEWEIGHTS",0, DXGI_FORMAT_R32G32B32A32_FLOAT,   0, offsetof(VERTEX::VERTEX_Skneed, boneWeights),D3D11_INPUT_PER_VERTEX_DATA, 0},
};



/* ---------------------------------------------------------------------------------------
/* - @:ShaderManager Class - デストラクタ - * - */
/* 【?】リソースの破棄
/* ---------------------------------------------------------------------------------------*/
ShaderManager::~ShaderManager()
{

}


bool ShaderManager::Init(std::shared_ptr<RendererEngine> renderer)
{
    m_pRenderer = renderer;

    // 入力レイアウト定義
    InputLayoutSetupData layout[] =
    {        
        ///////////////////////////////////////////////////
        // ディファードシェーディング
        ///////////////////////////////////////////////////
        {
            /* 簡易静的3Dオブジェクト */
            SHADER_TYPE::DEFERRED_STD_STATIC,
            ARRAYSIZE(g_Static_Layout),
            g_Static_Layout,
        },
        {
            /* 静的3Dオブジェクト（法線マップ有り） */
            SHADER_TYPE::DEFERRED_STD_STATIC_N,
            ARRAYSIZE(g_StaticTan_Layout),
            g_StaticTan_Layout,
        },
        // DEFERRED_STD_SKINNED
        {
            /* スキニング3Dモデル */
            SHADER_TYPE::DEFERRED_STD_SKINNED_N,
            ARRAYSIZE(g_Skneed_Layout),
            g_Skneed_Layout,
        },
        // DEFERRED_STD_RT_SPRITE
        {
            /* RT用スプライト */
            SHADER_TYPE::DEFERRED_STD_RT_SPRITE,
            ARRAYSIZE(g_Static_Layout),
            g_Static_Layout,
        },
        //DEFERRED_STD_TRAIL
        {
            /* 軌跡 */
            SHADER_TYPE::DEFERRED_STD_TRAIL,
            ARRAYSIZE(g_Static_Layout),
            g_Static_Layout,
        },
        // DEFERRED_STD_DECAL
        {
            /* デカール */
            SHADER_TYPE::DEFERRED_STD_DECAL,
            ARRAYSIZE(g_Static_Layout),
            g_Static_Layout,
        },

        ///////////////////////////////////////////////////
        // フォワードシェーディング
        ///////////////////////////////////////////////////
        
        // FORWARD_STD_STATIC
        {
            /* スプライト 標準 UI用  */
            SHADER_TYPE::FORWARD_UNLIT_UI_SPRITE,
            ARRAYSIZE(g_Static_Layout),
            g_Static_Layout,
        },       
        
        // FORWARD_STD_STATIC
        {
            /* スプライト 標準 UI用  テクスチャ無し  */
            SHADER_TYPE::FORWARD_UNLIT_UI_NOTEXTURE_SPRITE,
            ARRAYSIZE(g_Static_Layout),
            g_Static_Layout,
        },       
        //FOWARD_STANDARD_BILLBOARD
        {
            /* 簡易3Dオブジェクト ライティング無し  */
            SHADER_TYPE::FORWARD_UNLIT_STATIC,
            ARRAYSIZE(g_Static_Layout),
            g_Static_Layout,
        }, 
        //FORWARD_UNLIT_TRAIL
        {
            /* 軌跡 ライティング無し  */
            SHADER_TYPE::FORWARD_UNLIT_TRAIL,
            ARRAYSIZE(g_Static_Layout),
            g_Static_Layout,
        },         
        //FORWARD_UNLIT_TRAIL
        {
            /* 水 ライティング無し  */
            SHADER_TYPE::FORWARD_UNLIT_WATER,
            ARRAYSIZE(g_Static_Layout),
            g_Static_Layout,
        }, 

        ///////////////////////////////////////////////////
        // ポストエフェクト的な奴
        ///////////////////////////////////////////////////
        {
            /* 水平ガウシアンブラー用  */
            SHADER_TYPE::POST_GAUSSIAN_BLUR_HORIZONTAL,
            ARRAYSIZE(g_Static_Layout),
            g_Static_Layout,
        },
        {
            /* 垂直ガウシアンブラー用  */
            SHADER_TYPE::POST_GAUSSIAN_BLUR_VERTICAL,
            ARRAYSIZE(g_Static_Layout),
            g_Static_Layout,
        },          
        {
             /* スカイボックス用  */
             SHADER_TYPE::POST_SKYBOX,
             ARRAYSIZE(g_Static_Layout),
             g_Static_Layout,
        },        

        {
            /* 輝度抽出用  */
            SHADER_TYPE::POST_LUMINANCE_FILTER,
            ARRAYSIZE(g_Static_Layout),
            g_Static_Layout,
        },       
        {
            /* 川瀬式ブルーム（ダウンサンプリングしたガウス適用後のテクスチャをぼかす）  */
            SHADER_TYPE::POST_KAWASE_FILTER,
            ARRAYSIZE(g_Static_Layout),
            g_Static_Layout,
        },  
        {
            /* 通常シャドウマップ */
            SHADER_TYPE::POST_SHADOWMAP,
            ARRAYSIZE(g_Simple_Layout),
            g_Simple_Layout,
        },
        {
            /* スキニングモデル用シャドウマップ */
            SHADER_TYPE::POST_SHADOWMAP_SKINNED,
            ARRAYSIZE(g_Skneed_Layout),
            g_Skneed_Layout,
        },
        {
            /* 被写界深度 */
            SHADER_TYPE::POST_DEPTH_OF_FILED,
            ARRAYSIZE(g_Static_Layout),
            g_Static_Layout,
        },
        {
            /* トーンマッピング */
            SHADER_TYPE::POST_TONEMAPPING,
            ARRAYSIZE(g_Static_Layout),
            g_Static_Layout,
        },        
        {
            /* ディストーション */
            SHADER_TYPE::POST_DISTORTION,
            ARRAYSIZE(g_Static_Layout),
            g_Static_Layout,
        },

    };

    // 格納
    for (size_t i = 0; i < ARRAYSIZE(layout); i++)
    {
        m_InputLayoutSetupDataList.push_back(layout[i]);
    }


	// 定数バッファの使用方法定義
    D3D11_USAGE usages[UINT_CAST(CONSTANT_BUFFER_TYPE::NUM)] =
    {
        D3D11_USAGE_DYNAMIC,  // TRANSFORM
        D3D11_USAGE_DYNAMIC,  // VIEW
        D3D11_USAGE_DYNAMIC,  // PROJECTION
        D3D11_USAGE_DYNAMIC,  // BONE
        D3D11_USAGE_DYNAMIC,  // MATERIAL
        D3D11_USAGE_DYNAMIC,  // LIGHTS
        D3D11_USAGE_DYNAMIC,  // BLUR_WEIGHTS
		D3D11_USAGE_DYNAMIC,  // POSTEFFECT
		D3D11_USAGE_DYNAMIC,  // SHADOW
        D3D11_USAGE_DYNAMIC,  // SPRITE
        D3D11_USAGE_DYNAMIC,  // DECAL
        D3D11_USAGE_DYNAMIC,  // WINDOW
        D3D11_USAGE_DEFAULT,  // DISTORTION
        D3D11_USAGE_DYNAMIC,  // FRAME
    };

    /* 定数バッファの作成 */
	m_ConstantBuffers[UINT_CAST(CONSTANT_BUFFER_TYPE::TRANSFORM)]           = std::make_unique<ConstantBuffer<CB_TRANSFORM>>();
	m_ConstantBuffers[UINT_CAST(CONSTANT_BUFFER_TYPE::VIEW)]                = std::make_unique<ConstantBuffer<CB_VIEW>>();
	m_ConstantBuffers[UINT_CAST(CONSTANT_BUFFER_TYPE::PROJECTION)]          = std::make_unique<ConstantBuffer<CB_PROJECTION>>();
	m_ConstantBuffers[UINT_CAST(CONSTANT_BUFFER_TYPE::BONE)]                = std::make_unique<ConstantBuffer<CB_BONES_DATA>>();
	m_ConstantBuffers[UINT_CAST(CONSTANT_BUFFER_TYPE::MATERIAL)]            = std::make_unique<ConstantBuffer<CB_MATERIAL>>();
	m_ConstantBuffers[UINT_CAST(CONSTANT_BUFFER_TYPE::LIGHTS)]              = std::make_unique<ConstantBuffer<CB_LIGHTS>>();
	m_ConstantBuffers[UINT_CAST(CONSTANT_BUFFER_TYPE::BLUR_WEIGHTS)]        = std::make_unique<ConstantBuffer<CB_BLUR>>();
	m_ConstantBuffers[UINT_CAST(CONSTANT_BUFFER_TYPE::POSTEFFECT)]          = std::make_unique<ConstantBuffer<CB_POSTEFFECT>>();
	m_ConstantBuffers[UINT_CAST(CONSTANT_BUFFER_TYPE::SHADOW)]              = std::make_unique<ConstantBuffer<CB_SHADOW>>();
	m_ConstantBuffers[UINT_CAST(CONSTANT_BUFFER_TYPE::SPRITE)]              = std::make_unique<ConstantBuffer<CB_SPRITE>>();
	m_ConstantBuffers[UINT_CAST(CONSTANT_BUFFER_TYPE::DECAL)]               = std::make_unique<ConstantBuffer<CB_DECAL>>();
	m_ConstantBuffers[UINT_CAST(CONSTANT_BUFFER_TYPE::WINDOW)]              = std::make_unique<ConstantBuffer<CB_WINDOW>>();
	m_ConstantBuffers[UINT_CAST(CONSTANT_BUFFER_TYPE::DISTORTION)]          = std::make_unique<ConstantBuffer<CB_DISTORTION>>();
	m_ConstantBuffers[UINT_CAST(CONSTANT_BUFFER_TYPE::FRAME)]               = std::make_unique<ConstantBuffer<CB_FRAME>>();
    
	auto device = m_pRenderer.lock()->get_Device();

    int slot = 0;
    for (auto &cb : m_ConstantBuffers)
    {
        cb->Setup(device, usages[slot]);
        slot++;
    }

    return true;
}


/* ---------------------------------------------------------------------------------------
/* - @:ShaderManager Class - シェーダの作成 - * - */
/* 【?】指定タイプのシェーダを作成
/* 引数：1.シェーダタイプ
/* 引数：2.作成の仕方
/* ---------------------------------------------------------------------------------------*/
bool ShaderManager::CreateShader(SHADER_TYPE type, SHADER_CREATE_TYPE createType)
{
    ShaderInfo shaderInfo{};

    // 頂点・ピクセルシェーダ、入力レイアウトのセットアップ
    if (!VertexShaderFactory(type, &shaderInfo, createType))return false;
    if (!PixelShaderFactory(type, &shaderInfo, createType))return false;
    
    shaderInfo.Type = type;

    m_ShaderList.push_back(shaderInfo); // 無事作成出来たらリストに追加

    return true;
}

/* ---------------------------------------------------------------------------------------
/* - @:ShaderManager Class - シェーダをデバイスにセット - * - */
/* 【?】指定タイプのシェーダをデバイスにセット
/* 引数：1.シェーダ種類
/* ---------------------------------------------------------------------------------------*/
void ShaderManager::DeviceToSetShader(SHADER_TYPE type)
{
    auto begin = m_ShaderList.begin();
    auto end = m_ShaderList.end();

    // 指定シェーダを探す
    auto shader = std::find_if(begin, end,
        [type](const ShaderInfo& val)
        {
            return (val.Type == type);
        }
    );

    if (shader == end){
        assert(false);
        return;
    }

    auto pContext = m_pRenderer.lock().get()->get_DeviceContext();

    pContext->IASetInputLayout((*shader).pInputLayout->get());
    pContext->VSSetShader((*shader).pVShader->get(), NULL, 0);
    pContext->PSSetShader((*shader).pPShader->get(), NULL, 0);
}


/* ---------------------------------------------------------------------------------------
/* 【?】頂点・ピクセル両方にnull設定
/* ---------------------------------------------------------------------------------------*/
void ShaderManager::NullSetAllShader()
{
    auto pContext = m_pRenderer.lock().get()->get_DeviceContext();

    pContext->VSSetShader(nullptr, nullptr, 0);
    pContext->PSSetShader(nullptr, nullptr, 0);
}

//*---------------------------------------------------------------------------------------
//*【?】定数バッファの更新＆セット
//* 
//* [引数]
//* _type : 種類
//* *_data : 定数バッファの内容
//* _size : サイズ
//* [返値] 
//* なし
//*----------------------------------------------------------------------------------------
void ShaderManager::BindConstantBuffer(CONSTANT_BUFFER_TYPE _type, const void* _data, UINT _size)
{
    size_t index = static_cast<size_t>(_type);

    auto pContext = m_pRenderer.lock().get()->get_DeviceContext();
	m_ConstantBuffers[index]->UpdateRaw(pContext, _data, _size);
    m_ConstantBuffers[index]->Bind(pContext, UINT_CAST(_type));
}

//*---------------------------------------------------------------------------------------
//*【?】終了
//*
//* [引数] なし
//* [返値] なし
//*----------------------------------------------------------------------------------------
void ShaderManager::Term()
{
    m_ShaderList.clear();
    m_InputLayoutSetupDataList.clear();

    for (int i = 0; i < m_ConstantBuffers.size(); i++)
    {
        m_ConstantBuffers[i]->Release();
    }
}

/* ---------------------------------------------------------------------------------------
/* - @:ShaderManager Class - 入力レイアウトの作成 - * - */
/* 【?】入力レイアウト作成
/* ---------------------------------------------------------------------------------------*/
bool ShaderManager::InputLayoutFactory(SHADER_TYPE type, ShaderInfo* out, ID3DBlob* blob)
{
    auto renderer = m_pRenderer.lock();
    if (renderer == nullptr)return false;

    auto pDevice = renderer.get()->get_Device();
    auto pContext = renderer.get()->get_DeviceContext();
    HRESULT hr = S_OK;


    // 指定種別と合致する要素を取得
    auto begin = m_InputLayoutSetupDataList.begin();
    auto end = m_InputLayoutSetupDataList.end();
    auto target = std::find_if(begin, end,
        [type](const InputLayoutSetupData& val)
        {
            return (val.type == type);
        }
    );

    if (target == end) {
        MessageBoxA(NULL, "入力レイアウトの作成に失敗しました", "Error", MB_OK);
        return false;
    }


    UINT numElements = target->LayoutArraySize;   // 配列の要素数取得
    const D3D11_INPUT_ELEMENT_DESC* pLayout = target->pLayout;

    ComPtr<ID3D11InputLayout> pOutputLayout = nullptr; // 結果格納用

    // 入力レイアウト作成
    hr = pDevice->CreateInputLayout(
        pLayout,                     // 入力データ型の配列設定    
        numElements,                 // 配列の要素数
        blob->GetBufferPointer(),    // コンパイルされたシェーダへのポインタ
        blob->GetBufferSize(),       // コンパイルされたシェーダのサイズ
        pOutputLayout.GetAddressOf() // 生成した入力レイアウトの出力先
    );

    if (FAILED(hr)){
        MessageBoxA(NULL, "入力レイアウトの作成に失敗しました", "Error", MB_OK);
        return false;
    }

    out->pInputLayout = std::make_shared<InputLayout>();    // 共有ポインタ作成
    out->pInputLayout->set(std::move(pOutputLayout));       // レイアウトのセット

    return true;
}


/* ---------------------------------------------------------------------------------------
/* - @:ShaderManager Class - 入力レイアウトの作成 - * - */
/* 【?】入力レイアウト作成 CSO対応
/* ---------------------------------------------------------------------------------------*/
bool ShaderManager::InputLayoutFactory_CSO(SHADER_TYPE type, ShaderInfo *out, std::vector<uint8_t> &byteCode)
{
    auto renderer = m_pRenderer.lock();
    if (renderer == nullptr)return false;

    auto pDevice = renderer.get()->get_Device();
    auto pContext = renderer.get()->get_DeviceContext();
    HRESULT hr = S_OK;


    // 指定種別と合致する要素を取得
    auto begin = m_InputLayoutSetupDataList.begin();
    auto end = m_InputLayoutSetupDataList.end();
    auto target = std::find_if(begin, end,
        [type](const InputLayoutSetupData &val)
        {
            return (val.type == type);
        }
    );

    if (target == end) {
        MessageBoxA(NULL, "入力レイアウトの作成に失敗しました", "Error", MB_OK);
        return false;
    }


    UINT numElements = target->LayoutArraySize;   // 配列の要素数取得
    const D3D11_INPUT_ELEMENT_DESC *pLayout = target->pLayout;

    ComPtr<ID3D11InputLayout> pOutputLayout = nullptr; // 結果格納用

    // 入力レイアウト作成
    hr = pDevice->CreateInputLayout(
        pLayout,                     // 入力データ型の配列設定    
        numElements,                 // 配列の要素数
        byteCode.data(),             // コンパイルされたシェーダへのポインタ
        byteCode.size(),             // コンパイルされたシェーダのサイズ
        pOutputLayout.GetAddressOf() // 生成した入力レイアウトの出力先
    );

    if (FAILED(hr)) {
        MessageBoxA(NULL, "入力レイアウトの作成に失敗しました", "Error", MB_OK);
        return false;
    }

    out->pInputLayout = std::make_shared<InputLayout>();    // 共有ポインタ作成
    out->pInputLayout->set(std::move(pOutputLayout));       // レイアウトのセット

    return true;
}


/* ---------------------------------------------------------------------------------------
/* - @:ShaderManager Class - 頂点シェーダの作成 - * - */
/* 【?】頂点シェーダの作成
/* ---------------------------------------------------------------------------------------*/
bool ShaderManager::VertexShaderFactory(SHADER_TYPE type, ShaderInfo* out, SHADER_CREATE_TYPE createType)
{
    auto renderer = m_pRenderer.lock();
    if (renderer == nullptr)return false;
    auto pDevice = renderer.get()->get_Device();
    auto pContext = renderer.get()->get_DeviceContext();

    HRESULT hr = S_OK;
    ID3DBlob* pVSBlob = NULL;                      // ランタイム用
    std::vector<uint8_t> csoByteCode;              // cso読み込み用
    ComPtr<ID3D11VertexShader> pVertex = nullptr;  // 結果格納用


    // ランタイム実行 ********************************************************************************************
    if (createType == SHADER_CREATE_TYPE::RUNTIME)
    {
        // 頂点シェーダのコンパイル
        switch (type)
        {  
        case SHADER_TYPE::NONE:
            MessageBoxA(NULL, "不明な頂点シェーダ", "Error", MB_OK);
            break;
        ///////////////////////////////////////////////////
        // ディファードシェーディング
        ///////////////////////////////////////////////////
        case SHADER_TYPE::DEFERRED_STD_RT_SPRITE:   // RT用スプライト
            hr = this->CompileShader(HLSL__Sprite_VS_PATH.c_str(), "VSMain", "vs_5_0", &pVSBlob);
            break;
        case SHADER_TYPE::DEFERRED_STD_STATIC:      // 静的簡易3Dオブジェクト（法線マップなし）
            hr = this->CompileShader(HLSL__Static_VS_PATH.c_str(), "VSMain", "vs_5_0", &pVSBlob);
            break;       
        case SHADER_TYPE::DEFERRED_STD_STATIC_N:    // 静的3Dオブジェクト（法線マップあり）
            hr = this->CompileShader(HLSL__Static_Tan_VS_PATH.c_str(), "VSMain", "vs_5_0", &pVSBlob);
            break;
        case SHADER_TYPE::DEFERRED_STD_SKINNED_N:   // スキニング3Dモデル
            hr = this->CompileShader(HLSL__Skinned_VS_PATH.c_str(), "VSMain", "vs_5_0", &pVSBlob);
            break;
        case SHADER_TYPE::DEFERRED_STD_TRAIL:       // 軌跡
            hr = this->CompileShader(HLSL__Trail_VS_PATH.c_str(), "VSMain", "vs_5_0", &pVSBlob);
            break;
        case SHADER_TYPE::DEFERRED_STD_DECAL:       // デカール
            hr = this->CompileShader(HLSL__Static_VS_PATH.c_str(), "VSMain", "vs_5_0", &pVSBlob);
            break;

        ///////////////////////////////////////////////////
        // フォワードシェーディング
        ///////////////////////////////////////////////////
        case SHADER_TYPE::FORWARD_UNLIT_UI_SPRITE:    // スプライト 標準 UI用
            hr = this->CompileShader(HLSL__Sprite_VS_PATH.c_str(), "VSMain", "vs_5_0", &pVSBlob);
            break;       
        case SHADER_TYPE::FORWARD_UNLIT_UI_NOTEXTURE_SPRITE:    // スプライト 標準 UI用 テクスチャ無し
            hr = this->CompileShader(HLSL__Sprite_VS_PATH.c_str(), "VSMain", "vs_5_0", &pVSBlob);
            break;       
        case SHADER_TYPE::FORWARD_UNLIT_STATIC:       // 簡易3Dオブジェクト ライティング無し
            hr = this->CompileShader(HLSL__Static_VS_PATH.c_str(), "VSMain", "vs_5_0", &pVSBlob);
            break; 
        case SHADER_TYPE::FORWARD_UNLIT_TRAIL:       // 軌跡 ライティング無し
            hr = this->CompileShader(HLSL__Trail_VS_PATH.c_str(), "VSMain", "vs_5_0", &pVSBlob);
            break;       
        case SHADER_TYPE::FORWARD_UNLIT_WATER:       // 水 ライティング無し
            hr = this->CompileShader(HLSL__Static_VS_PATH.c_str(), "VSMain", "vs_5_0", &pVSBlob);
            break; 
            
        ///////////////////////////////////////////////////
        // ポストエフェクト
        ///////////////////////////////////////////////////
        case SHADER_TYPE::POST_GAUSSIAN_BLUR_HORIZONTAL:     // ガウシアン水平ブラー
            hr = this->CompileShader(HLSL__XBlur_VS_PATH.c_str(), "VSMain", "vs_5_0", &pVSBlob);
            break;
        case SHADER_TYPE::POST_GAUSSIAN_BLUR_VERTICAL:       // ガウシアン垂直ブラー
            hr = this->CompileShader(HLSL__YBlur_VS_PATH.c_str(), "VSMain", "vs_5_0", &pVSBlob);
            break;       
        case SHADER_TYPE::POST_SKYBOX:                      // スカイボックス
            hr = this->CompileShader(HLSL__Skybox_VS_PATH.c_str(), "VSMain", "vs_5_0", &pVSBlob);
            break;        
        case SHADER_TYPE::POST_LUMINANCE_FILTER:            // 輝度抽出
            hr = this->CompileShader(HLSL__Sprite_VS_PATH.c_str(), "VSMain", "vs_5_0", &pVSBlob);
            break;       
        case SHADER_TYPE::POST_KAWASE_FILTER:               // 川瀬ブルーム用
            hr = this->CompileShader(HLSL__Sprite_VS_PATH.c_str(), "VSMain", "vs_5_0", &pVSBlob);
            break;
        case SHADER_TYPE::POST_SHADOWMAP:                   // シャドウマップ
            hr = this->CompileShader(HLSL__ShadowMap_PATH.c_str(), "VSMain", "vs_5_0", &pVSBlob);
            break;     
        case SHADER_TYPE::POST_SHADOWMAP_SKINNED:           // スキニングモデル用シャドウマップ
            hr = this->CompileShader(HLSL__ShadowMap_Skinned_PATH.c_str(), "VSMain", "vs_5_0", &pVSBlob);
            break;           
        case SHADER_TYPE::POST_DEPTH_OF_FILED:              // 被写界深度
            hr = this->CompileShader(HLSL__Sprite_VS_PATH.c_str(), "VSMain", "vs_5_0", &pVSBlob);
            break;     
        case SHADER_TYPE::POST_TONEMAPPING:                 // トーンマッピング
            hr = this->CompileShader(HLSL__Sprite_VS_PATH.c_str(), "VSMain", "vs_5_0", &pVSBlob);
            break;          
        case SHADER_TYPE::POST_DISTORTION:                  // ディストーション
            hr = this->CompileShader(HLSL__Distortion_VS_PATH.c_str(), "VSMain", "vs_5_0", &pVSBlob);
            break;     
        default:
            MessageBoxA(NULL, "不明な頂点シェーダ", "Error", MB_OK);
            break;
        }

        // 失敗
        if (FAILED(hr)) {
            MessageBoxA(NULL, "頂点シェーダーがコンパイルできませんでした", "Error", MB_OK);
            return false;
        }

        // 頂点シェーダーの作成
        hr = pDevice->CreateVertexShader(
            pVSBlob->GetBufferPointer(),
            pVSBlob->GetBufferSize(),
            NULL,
            pVertex.GetAddressOf()
        );

        if (FAILED(hr)) {
            pVSBlob->Release();
            MessageBoxA(NULL, "頂点シェーダーの作成に失敗しました。", "Error", MB_OK);
            return false;
        }


        // 頂点レイアウトの作成
        bool res = InputLayoutFactory(type, out, pVSBlob);
        pVSBlob->Release();

        if (!res) {
            assert(false);
            return false;
        }
    }
    // .cso読み込み ********************************************************************************************
    else if (createType == SHADER_CREATE_TYPE::CSO)
    {
        // 頂点シェーダのコンパイル
        switch (type)
        {      
        case SHADER_TYPE::NONE:
            MessageBoxA(NULL, "不明な頂点シェーダ", "Error", MB_OK);
            break;
        case SHADER_TYPE::DEFERRED_STD_RT_SPRITE:           // RT用スプライト
            this->LoadCSOFile(HLSL_CSO__Sprite_VS_PATH.c_str(), &csoByteCode);
            break;
        case SHADER_TYPE::DEFERRED_STD_STATIC:              // 静的簡易3Dオブジェクト（法線マップなし）
            this->LoadCSOFile(HLSL_CSO__Static_VS_PATH.c_str(), &csoByteCode);
            break;       
        case SHADER_TYPE::DEFERRED_STD_STATIC_N:            // 静的3Dオブジェクト（法線マップあり）
            this->LoadCSOFile(HLSL_CSO__Static_Tan_VS_PATH.c_str(), &csoByteCode);
            break;
        case SHADER_TYPE::DEFERRED_STD_SKINNED_N:           // スキニング3Dモデル
            this->LoadCSOFile(HLSL_CSO__Skinned_VS_PATH.c_str(),&csoByteCode);
            break;
        case SHADER_TYPE::DEFERRED_STD_TRAIL:           // 軌跡
            this->LoadCSOFile(HLSL_CSO__Trail_VS_PATH.c_str(),&csoByteCode);
            break;
        case SHADER_TYPE::DEFERRED_STD_DECAL:           // デカール
            this->LoadCSOFile(HLSL_CSO__Static_VS_PATH.c_str(),&csoByteCode);
            break;
        case SHADER_TYPE::FORWARD_UNLIT_UI_SPRITE:          // スプライト 標準 UI用
            this->LoadCSOFile(HLSL_CSO__Sprite_VS_PATH.c_str(), &csoByteCode);
            break;
		case SHADER_TYPE::FORWARD_UNLIT_UI_NOTEXTURE_SPRITE: // スプライト 標準 UI用 テクスチャ無し
            this->LoadCSOFile(HLSL_CSO__Sprite_VS_PATH.c_str(), &csoByteCode);
            break;
        case SHADER_TYPE::FORWARD_UNLIT_STATIC:             // 簡易3Dオブジェクト ライティング無し
            this->LoadCSOFile(HLSL_CSO__Static_VS_PATH.c_str(), &csoByteCode);
            break;
        case SHADER_TYPE::FORWARD_UNLIT_TRAIL:              // 軌跡 ライティング無し
            this->LoadCSOFile(HLSL_CSO__Trail_VS_PATH.c_str(),&csoByteCode);
            break;        
        case SHADER_TYPE::FORWARD_UNLIT_WATER:              // 水 ライティング無し
            this->LoadCSOFile(HLSL_CSO__Static_VS_PATH.c_str(),&csoByteCode);
            break;
        case SHADER_TYPE::POST_GAUSSIAN_BLUR_HORIZONTAL:     // ガウシアン水平ブラー
            this->LoadCSOFile(HLSL_CSO__XBlur_VS_PATH.c_str(), &csoByteCode);
            break;
        case SHADER_TYPE::POST_GAUSSIAN_BLUR_VERTICAL:       // ガウシアン垂直ブラー
            this->LoadCSOFile(HLSL_CSO__YBlur_VS_PATH.c_str(), &csoByteCode);
            break;        
        case SHADER_TYPE::POST_SKYBOX:                       // スカイボックス
            this->LoadCSOFile(HLSL_CSO__Skybox_VS_PATH.c_str(), &csoByteCode);
            break;        
        case SHADER_TYPE::POST_LUMINANCE_FILTER:             // 輝度抽出
            this->LoadCSOFile(HLSL_CSO__Sprite_VS_PATH.c_str(), &csoByteCode);
            break;      
        case SHADER_TYPE::POST_KAWASE_FILTER:                // 川瀬ブルーム用
            this->LoadCSOFile(HLSL_CSO__Sprite_VS_PATH.c_str(), &csoByteCode);
            break;      
        case SHADER_TYPE::POST_SHADOWMAP:                   // シャドウマップ
            this->LoadCSOFile(HLSL_CSO__ShadowMap_PATH.c_str(), &csoByteCode);
            break;     
        case SHADER_TYPE::POST_SHADOWMAP_SKINNED:           // シャドウマップ
            this->LoadCSOFile(HLSL_CSO__ShadowMap_Skinned_PATH.c_str(), &csoByteCode);
            break;       
        case SHADER_TYPE::POST_DEPTH_OF_FILED:              // 被写界深度
            this->LoadCSOFile(HLSL_CSO__Sprite_VS_PATH.c_str(), &csoByteCode);
            break;     
        case SHADER_TYPE::POST_TONEMAPPING:                 // トーンマッピング
            this->LoadCSOFile(HLSL_CSO__Sprite_VS_PATH.c_str(), &csoByteCode);
            break;           
        case SHADER_TYPE::POST_DISTORTION:                  // トーンマッピング
            this->LoadCSOFile(HLSL_CSO__Distortion_VS_PATH.c_str(), &csoByteCode);
            break;     
        default:
            MessageBoxA(NULL, "不明な頂点シェーダ", "Error", MB_OK);
            break;
        }

        // 頂点シェーダーの作成
        hr = pDevice->CreateVertexShader(
            csoByteCode.data(),
            csoByteCode.size(),
            NULL,
            pVertex.GetAddressOf()
        );

        if (FAILED(hr)) {
            csoByteCode.clear();
            MessageBoxA(NULL, "頂点シェーダーの作成に失敗しました。", "Error", MB_OK);
            return false;
        }


        // 頂点レイアウトの作成
        bool res = InputLayoutFactory_CSO(type, out, csoByteCode);

        if (!res) {
            assert(false);
            return false;
        }

        csoByteCode.clear();
    }


    out->pVShader = std::make_shared<VertexShader>();   // 共有ポインタ作成
    out->pVShader->set(std::move(pVertex));  // 頂点シェーダセット

    return true;    // 成功
}


/* ---------------------------------------------------------------------------------------
/* - @:ShaderManager Class - ピクセルシェーダの作成 - * - */
/* 【?】ピクセルシェーダの作成
/* ---------------------------------------------------------------------------------------*/
bool ShaderManager::PixelShaderFactory(SHADER_TYPE type, ShaderInfo* out, SHADER_CREATE_TYPE createType)
{
    auto renderer = m_pRenderer.lock();
    if (renderer == nullptr)return false;
    auto pDevice = renderer.get()->get_Device();
    auto pContext = renderer.get()->get_DeviceContext();

    HRESULT hr = S_FALSE;
    ID3DBlob* pPSBlob = NULL;                      // ランタイム用
    std::vector<uint8_t> csoByteCode;              // cso読み込み用 
    ComPtr<ID3D11PixelShader> pPixel = nullptr;    // 結果格納用


    // ランタイム実行 ********************************************************************************************
    if (createType == SHADER_CREATE_TYPE::RUNTIME)
    {
        // ピクセルシェーダーのコンパイル
        switch (type)
        {   
        case SHADER_TYPE::NONE:
            MessageBoxA(NULL, "不明なピクセルシェーダ", "Error", MB_OK);
            break;

        ///////////////////////////////////////////////////
        // ディファードシェーディング
        ///////////////////////////////////////////////////
        case SHADER_TYPE::DEFERRED_STD_RT_SPRITE:   // RT用スプライト
            hr = this->CompileShader(HLSL__LightingPath_Standard_PS_PATH.c_str(), "PSMain", "ps_5_0", &pPSBlob);
            break;
        case SHADER_TYPE::DEFERRED_STD_STATIC:      // 静的簡易3Dオブジェクト(法線マップなし)
            hr = CompileShader(HLSL__GBuffer_Simple_PS_PATH.c_str(), "PSMain", "ps_5_0", &pPSBlob);
            break;      
        case SHADER_TYPE::DEFERRED_STD_STATIC_N:    // 静的3Dオブジェクト(法線マップあり)
            hr = CompileShader(HLSL__GBuffer_Standard_PS_PATH.c_str(), "PSMain", "ps_5_0", &pPSBlob);
            break;
        case SHADER_TYPE::DEFERRED_STD_SKINNED_N:   // スキニング3Dモデル
            hr = CompileShader(HLSL__GBuffer_Standard_PS_PATH.c_str(), "PSMain", "ps_5_0", &pPSBlob);
            break;
        case SHADER_TYPE::DEFERRED_STD_TRAIL:       // 軌跡
            hr = CompileShader(HLSL__GBuffer_Simple_PS_PATH.c_str(), "PSMain", "ps_5_0", &pPSBlob);
            break;
        case SHADER_TYPE::DEFERRED_STD_DECAL:       // デカール
            hr = CompileShader(HLSL__Decal_PS_PATH.c_str(), "PSMain", "ps_5_0", &pPSBlob);
            break;

        ///////////////////////////////////////////////////
        // フォワードシェーディング
        ///////////////////////////////////////////////////
        case SHADER_TYPE::FORWARD_UNLIT_UI_SPRITE:    // スプライト 標準 UI用
            hr = this->CompileShader(HLSL__Sprite_PS_PATH.c_str(), "PSMain", "ps_5_0", &pPSBlob);
            break;      
		case SHADER_TYPE::FORWARD_UNLIT_UI_NOTEXTURE_SPRITE:  // スプライト 標準 UI用 テクスチャ無し
            hr = this->CompileShader(HLSL__Sprite_NoTexture_PS_PATH.c_str(), "PSMain", "ps_5_0", &pPSBlob);
            break;      
        case SHADER_TYPE::FORWARD_UNLIT_STATIC:    // 簡易3Dオブジェクト ライティング無し
            hr = this->CompileShader(HLSL__Simple_PS_PATH.c_str(), "PSMain", "ps_5_0", &pPSBlob);
            break;        
        case SHADER_TYPE::FORWARD_UNLIT_TRAIL:    // 軌跡 ライティング無し
            hr = this->CompileShader(HLSL__Simple_PS_PATH.c_str(), "PSMain", "ps_5_0", &pPSBlob);
            break;          
        case SHADER_TYPE::FORWARD_UNLIT_WATER:    // 水 ライティング無し
            hr = this->CompileShader(HLSL__Water_PS_PATH.c_str(), "PSMain", "ps_5_0", &pPSBlob);
            break;        

        ///////////////////////////////////////////////////
        // ポストエフェクト
        ///////////////////////////////////////////////////
        case SHADER_TYPE::POST_GAUSSIAN_BLUR_HORIZONTAL:     // ガウシアン水平ブラー
            hr = this->CompileShader(HLSL__GaussianBlur_PS_PATH.c_str(), "PSMain", "ps_5_0", &pPSBlob);
            break;
        case SHADER_TYPE::POST_GAUSSIAN_BLUR_VERTICAL:       // ガウシアン垂直ブラー
            hr = this->CompileShader(HLSL__GaussianBlur_PS_PATH.c_str(), "PSMain", "ps_5_0", &pPSBlob);
            break;       
        case SHADER_TYPE::POST_SKYBOX:                       // スカイボックス
            hr = this->CompileShader(HLSL__Skybox_PS_PATH.c_str(), "PSMain", "ps_5_0", &pPSBlob);
            break;     
        case SHADER_TYPE::POST_LUMINANCE_FILTER:             // 輝度抽出
            hr = this->CompileShader(HLSL__HighLuminanceFilter_PS_PATH.c_str(), "PSMain", "ps_5_0", &pPSBlob);
            break;       
        case SHADER_TYPE::POST_KAWASE_FILTER:                // 川瀬ブルーム用
            hr = this->CompileShader(HLSL__KawaseFilter_PS_PATH.c_str(), "PSMain", "ps_5_0", &pPSBlob);
            break;
        case SHADER_TYPE::POST_SHADOWMAP:                    // シャドウマップ
            hr = this->CompileShader(HLSL__ShadowMap_PATH.c_str(), "PSMain", "ps_5_0", &pPSBlob);
            break;    
        case SHADER_TYPE::POST_SHADOWMAP_SKINNED:            // スキニングモデル用シャドウマップ
            hr = this->CompileShader(HLSL__ShadowMap_Skinned_PATH.c_str(), "PSMain", "ps_5_0", &pPSBlob);
            break;        
        case SHADER_TYPE::POST_DEPTH_OF_FILED:               // 被写界深度
            hr = this->CompileShader(HLSL__DoF_Filter_PS_PATH.c_str(), "PSMain", "ps_5_0", &pPSBlob);
            break;    
        case SHADER_TYPE::POST_TONEMAPPING:                  // トーンマッピング
            hr = this->CompileShader(HLSL__ToneMappingFilter_PS_PATH.c_str(), "PSMain", "ps_5_0", &pPSBlob);
            break;         
        case SHADER_TYPE::POST_DISTORTION:                   // ディストーション
            hr = this->CompileShader(HLSL__Distortion_PS_PATH.c_str(), "PSMain", "ps_5_0", &pPSBlob);
            break;    


        default:
            MessageBoxA(NULL, "不明なピクセルシェーダ", "Error", MB_OK);
            break;
        }


        // 失敗
        if (FAILED(hr)) {
            MessageBoxA(NULL, "ピクセルシェーダーがコンパイルできませんでした", "Error", MB_OK);
            return false;
        }

        if (pPSBlob == nullptr)
        {
            MessageBoxA(NULL, "ピクセルシェーダーがコンパイルできませんでした", "Error", MB_OK);
            return false;
        }
        // ピクセルシェーダーの作成
        hr = pDevice->CreatePixelShader(
            pPSBlob->GetBufferPointer(),
            pPSBlob->GetBufferSize(),
            NULL,
            pPixel.GetAddressOf()
        );
        pPSBlob->Release();
        
        if (FAILED(hr)) {
            MessageBoxA(NULL, "ピクセルシェーダの作成に失敗しました。", "Error", MB_OK);
            return false;
        }
    }
    // .cso読み込み ********************************************************************************************
    else if (createType == SHADER_CREATE_TYPE::CSO)
    {
        // 頂点シェーダのコンパイル
        switch (type)
        {
        case SHADER_TYPE::NONE:
            MessageBoxA(NULL, "不明なピクセルシェーダ", "Error", MB_OK);
            break;
        case SHADER_TYPE::DEFERRED_STD_RT_SPRITE:
            this->LoadCSOFile(HLSL_CSO__LightingPath_Standard_PS_PATH.c_str(), &csoByteCode);
            break;
        case SHADER_TYPE::DEFERRED_STD_STATIC:
            this->LoadCSOFile(HLSL_CSO__GBuffer_Simple_PS_PATH.c_str(), &csoByteCode);
            break;     
        case SHADER_TYPE::DEFERRED_STD_STATIC_N:
            this->LoadCSOFile(HLSL_CSO__GBuffer_Standard_PS_PATH.c_str(), &csoByteCode);
            break;
        case SHADER_TYPE::DEFERRED_STD_SKINNED_N:
            this->LoadCSOFile(HLSL_CSO__GBuffer_Standard_PS_PATH.c_str(), &csoByteCode);
            break;
        case SHADER_TYPE::DEFERRED_STD_TRAIL:       
            this->LoadCSOFile(HLSL_CSO__GBuffer_Simple_PS_PATH.c_str(), &csoByteCode);
            break;
        case SHADER_TYPE::DEFERRED_STD_DECAL:       
            this->LoadCSOFile(HLSL_CSO__Decal_PS_PATH.c_str(), &csoByteCode);
            break;
        case SHADER_TYPE::FORWARD_UNLIT_UI_SPRITE:
            this->LoadCSOFile(HLSL_CSO__Sprite_PS_PATH.c_str(), &csoByteCode);
            break;   
        case SHADER_TYPE::FORWARD_UNLIT_UI_NOTEXTURE_SPRITE:
            this->LoadCSOFile(HLSL_CSO__Sprite_NoTexture_PS_PATH.c_str(), &csoByteCode);
            break;   
        case SHADER_TYPE::FORWARD_UNLIT_STATIC:
            this->LoadCSOFile(HLSL_CSO__Simple_PS_PATH.c_str(), &csoByteCode);
            break;
        case SHADER_TYPE::FORWARD_UNLIT_TRAIL:
            this->LoadCSOFile(HLSL_CSO__Simple_PS_PATH.c_str(), &csoByteCode);
            break;     
        case SHADER_TYPE::FORWARD_UNLIT_WATER:
            this->LoadCSOFile(HLSL_CSO__Water_PS_PATH.c_str(), &csoByteCode);
            break;
        case SHADER_TYPE::POST_GAUSSIAN_BLUR_HORIZONTAL:
            this->LoadCSOFile(HLSL_CSO__GaussianBlur_PS_PATH.c_str(), &csoByteCode);
            break;
        case SHADER_TYPE::POST_GAUSSIAN_BLUR_VERTICAL:
            this->LoadCSOFile(HLSL_CSO__GaussianBlur_PS_PATH.c_str(), &csoByteCode);
            break;       
        case SHADER_TYPE::POST_SKYBOX:
            this->LoadCSOFile(HLSL_CSO__Skybox_PS_PATH.c_str(), &csoByteCode);
            break;      
        case SHADER_TYPE::POST_LUMINANCE_FILTER:
            this->LoadCSOFile(HLSL_CSO__HighLuminanceFilter_PS_PATH.c_str(), &csoByteCode);
            break;       
        case SHADER_TYPE::POST_KAWASE_FILTER:
            this->LoadCSOFile(HLSL_CSO__KawaseFilter_PS_PATH.c_str(), &csoByteCode);
            break;       
        case SHADER_TYPE::POST_SHADOWMAP:
            this->LoadCSOFile(HLSL_CSO__ShadowMap_PATH.c_str(), &csoByteCode);
            break;     
        case SHADER_TYPE::POST_SHADOWMAP_SKINNED:
            this->LoadCSOFile(HLSL_CSO__ShadowMap_Skinned_PATH.c_str(), &csoByteCode);
            break;       
        case SHADER_TYPE::POST_DEPTH_OF_FILED:
            this->LoadCSOFile(HLSL_CSO__DoF_Filter_PS_PATH.c_str(), &csoByteCode);
            break;     
        case SHADER_TYPE::POST_TONEMAPPING:
            this->LoadCSOFile(HLSL_CSO__ToneMappingFilter_PS_PATH.c_str(), &csoByteCode);
            break;          
        case SHADER_TYPE::POST_DISTORTION:
            this->LoadCSOFile(HLSL_CSO__Distortion_PS_PATH.c_str(), &csoByteCode);
            break;     
        
        default:
            MessageBoxA(NULL, "不明なピクセルシェーダ", "Error", MB_OK);
            break;
        }

        // 頂点シェーダーの作成
        hr = pDevice->CreatePixelShader(
            csoByteCode.data(),
            csoByteCode.size(),
            NULL,
            pPixel.GetAddressOf()
        );


        if (FAILED(hr)) {
            csoByteCode.clear();
            MessageBoxA(NULL, "頂点シェーダーの作成に失敗しました。", "Error", MB_OK);
            return false;
        }

        csoByteCode.clear();
    }

    out->pPShader = std::make_shared<PixelShader>();   // 共有ポインタ作成
    out->pPShader.get()->set(std::move(pPixel));   // ピクセルシェーダのセット
    
    return true;    // 成功
}


/* ---------------------------------------------------------------------------------------
/* - @:ShaderManager Class - シェーダのコンパイル - * - */
/* 【?】シェーダをコンパイルする
/* 引数：1.
/* ---------------------------------------------------------------------------------------*/
HRESULT ShaderManager::CompileShader(LPCWSTR szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob **ppBlobOut)
{
    HRESULT hr = S_OK;
    //DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
    DWORD dwShaderFlags =D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR;  // こっちの方が計算が速いらしい https://proglog.site/triangle-transform-on-shader/

#if defined(DEBUG) || defined(_DEBUG)
    // D3DCOMPILE_DEBUG フラグを設定して、シェーダーにデバッグ情報を埋め込む。
    // このフラグを設定すると、シェーダーのデバッグ体験が向上するが、それでも
    // シェーダーは最適化され、このプログラムのリリース構成で実行されるのと同じように
    // 実行される。
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob *pErrorBlob;
    hr = D3DCompileFromFile(
        szFileName,         // コンパイルするhlslファイルパス
        NULL,               // hlsl上で事前に定義する#defineの指定
        D3D_COMPILE_STANDARD_FILE_INCLUDE,               // インクルードファイルを取り扱うためのID3DIncludeの指定
        szEntryPoint,       // エントリーポイントとなる関数名指定
        szShaderModel,      // コンパイル時のターゲットとなるシェーダモデル指定
        dwShaderFlags,      // コンパイルオプション
        0,                  // エフェクトファイル用のコンパイルオプション(シェーダのコンパイルの場合は０)
        ppBlobOut,          // コンパイル結果のバイナリコードの格納
        &pErrorBlob         // コンパイル時のError内容の格納
    );

    if (FAILED(hr))
    {
        if (pErrorBlob != NULL)
        {
            OutputDebugStringA((char *)pErrorBlob->GetBufferPointer());
            if (pErrorBlob)pErrorBlob->Release();
            return hr;
        }

        return hr;
    }
    if (pErrorBlob)pErrorBlob->Release();

    return S_OK;
}



//*---------------------------------------------------------------------------------------
//*【?】CSOファイルの読み込み
//*
//* [引数]
//* csoName : .csoファイル名
//* pBlob : データの格納先
//* 
//* [返値]
//* true : 成功
//* false : 失敗
//*----------------------------------------------------------------------------------------
bool ShaderManager::LoadCSOFile(const wchar_t *csoName, std::vector<uint8_t> *pByteCodeOUT)
{
    *pByteCodeOUT = DX::ReadData(csoName);  // 読み込み マイクロソフトのやつ

    if (pByteCodeOUT->empty())return false;

    return true;
}