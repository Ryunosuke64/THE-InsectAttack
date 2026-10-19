#include "pch.h"
#include "Component_DecalRenderer.h"
#include "RendererEngine.h"
#include "GameObject.h"
#include "Component_IMeshResource.h"
#include "Texture.h"
#include "BlendManager.h"

using namespace DirectX;
using namespace RenderData;
using namespace VERTEX;

//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//* [引数]
//* pOwner : オーナーオブジェクト
//* updateRank : 更新レイヤー
//*----------------------------------------------------------------------------------------
DecalRenderer::DecalRenderer(std::weak_ptr<GameObject> pOwner, int updateRank) 
    : IComponent(pOwner, updateRank),
    m_IsDynamic(false)
{
	this->set_Tag("DecalRenderer");
}


//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
DecalRenderer::~DecalRenderer()
{

}


//*---------------------------------------------------------------------------------------
//*【?】初期化
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void DecalRenderer::Start(RendererEngine &renderer)
{
    // デカール用定数バッファ作成
    //if (!CreateDecalConstantBuffer(renderer))
    //{
    //    MessageBox(NULL, "デカール用定数バッファが作成できませんでした", "Error", MB_OK);
    //}
}


//*---------------------------------------------------------------------------------------
//*【?】更新
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void DecalRenderer::Update(RendererEngine &renderer)
{

}


//*---------------------------------------------------------------------------------------
//*【?】描画
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void DecalRenderer::Draw(RendererEngine &renderer)
{
    // シャドウパスは返す
    if (renderer.get_CrntRenderPass() == RENDER_PASS::SHADOW) {
        return;
    }

    auto pContext = renderer.get_DeviceContext();
    std::shared_ptr<MeshResourceData> meshInfo = m_pMeshResource.lock()->m_pMeshData;
    ID3D11Buffer *vtxBuff = meshInfo->pVertexBuffer;
    UINT vtxStride = meshInfo->VertexStride;
    ID3D11Buffer *idxBuff = meshInfo->pIndexBuffer;
    CB_TRANSFORM cbTransform = {};

    auto transform = m_pOwner.lock()->get_Transform().lock();

    /* ========== 定数バッファの更新 ========== */
    // ワールド行列セット ==========================
    XMMATRIX worldMtx = transform->get_WorldMtx();
    worldMtx = XMMatrixTranspose(worldMtx);                 // 行列の転置
    XMStoreFloat4x4(&cbTransform.WorldMtx, worldMtx);       // XMMATRIX → XMFLOAT4X4変換

    // デカール情報セット ==========================
    XMMATRIX invWorld = XMMatrixInverse(NULL, worldMtx);
    XMStoreFloat4x4(&m_CBDecalData.DecalWorldInvMtx, invWorld); // ワールド逆行列
	m_CBDecalData.DecalColor = VEC3(1.0f, 1.0f, 1.0f);          // デカールの色（白でテクスチャの色をそのまま使う）

    // 通常パス **********************************************************
    if (renderer.get_CrntRenderPass() == RENDER_PASS::MAIN) {
        Master::m_pShaderManager->DeviceToSetShader(SHADER_TYPE::DEFERRED_STD_DECAL);

        // マテリアル取得
        auto pMatData = meshInfo->pMaterials.lock();

        // マテリアル情報セット ==========================
        m_CBMaterialData.Diffuse = pMatData->m_DiffuseColor;
        m_CBMaterialData.Specular = pMatData->m_SpecularColor;
        m_CBMaterialData.SpecularPower = pMatData->m_SpecularPower;
        //m_CBMaterialData.EmissivePower = pMatData->m_EmissivePower;
        m_CBMaterialData.EmissiveColor = pMatData->m_EmissiveColor;
        m_CBMaterialData.EnvironmentReflectionStrength = 0.0f;
        m_CBMaterialData.OffsetUV;

        // 定数バッファをセット ==========================
        Master::m_pShaderManager->BindConstantBuffer(CONSTANT_BUFFER_TYPE::TRANSFORM, (void*)&cbTransform, sizeof(CB_TRANSFORM));
        Master::m_pShaderManager->BindConstantBuffer(CONSTANT_BUFFER_TYPE::MATERIAL, (void*)&m_CBMaterialData, sizeof(CB_MATERIAL));
        Master::m_pShaderManager->BindConstantBuffer(CONSTANT_BUFFER_TYPE::DECAL, (void*)&m_CBDecalData, sizeof(CB_DECAL));

        // テクスチャセット（アルベドと法線のみ） ==========================
        ID3D11ShaderResourceView *diffuseSRV = nullptr;
        ID3D11ShaderResourceView *normalSRV = nullptr;
        if (auto tex = pMatData->m_DiffuseMap.Texture.lock()) {
            diffuseSRV = tex.get()->get_SRV();
        }
        if (auto tex = pMatData->m_NormalMap.Texture.lock()) {
            normalSRV = tex.get()->get_SRV();
        }

        // シェーダーリソースビューをセット
        pContext->PSSetShaderResources(0, 1, &diffuseSRV);
        pContext->PSSetShaderResources(1, 1, &normalSRV);
    }

    // 頂点＆インデックスバッファ設定 ==========================
    UINT offset = 0;
    pContext->IASetVertexBuffers(0, 1, &vtxBuff, &vtxStride, &offset);      // 頂点バッファをセット
    pContext->IASetIndexBuffer(idxBuff, DXGI_FORMAT_R16_UINT, 0);           // インデックスバッファをセット
    pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);// Set primitive topology 頂点の組み合わせ方

    // 描画コール ==========================
    pContext->DrawIndexed(meshInfo->NumIndex, 0, 0);
}

//*---------------------------------------------------------------------------------------
//*【?】デカール用定数バッファの作成
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
bool DecalRenderer::CreateDecalConstantBuffer(RendererEngine &renderer)
{
    //auto pDevice = renderer.get_Device();

    //D3D11_USAGE usage;  // 使用方法
    //UINT accesssFlags;  // CPU書き込みフラグ

    //// 動的変更をするか
    //if (m_IsDynamic){
    //    usage = D3D11_USAGE_DYNAMIC;
    //    accesssFlags = D3D11_CPU_ACCESS_WRITE;
    //}
    //else{
    //    usage = D3D11_USAGE_DEFAULT;
    //    accesssFlags = 0;
    //}

    //m_pCBDecalSet = new CB_DECAL_SET();

    //D3D11_BUFFER_DESC bd;
    //ZeroMemory(&bd, sizeof(D3D11_BUFFER_DESC));
    //bd.Usage = usage;
    //bd.ByteWidth = sizeof(CB_DECAL);
    //bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    //bd.CPUAccessFlags = accesssFlags;

    //D3D11_SUBRESOURCE_DATA initData;
    //ZeroMemory(&initData, sizeof(D3D11_SUBRESOURCE_DATA));
    //initData.pSysMem = nullptr;

    //// バッファの作成
    //HRESULT hr = pDevice->CreateBuffer(&bd, nullptr, &m_pCBDecalSet->pBuff);
    //if (FAILED(hr))
    //{
    //    return false;
    //}

    return true;
}


//*---------------------------------------------------------------------------------------
//* @:DecalRenderer Class 
//*【?】IMeshResource参照用のポインタ設定
//* 引数：1.IMeshResource
//* 返値：void
//*----------------------------------------------------------------------------------------
void DecalRenderer::set_MeshResource(std::weak_ptr<class IMeshResource> meshResource)
{
    m_pMeshResource = meshResource;
}
