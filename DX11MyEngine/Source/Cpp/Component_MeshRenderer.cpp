#include "pch.h"
#include "GameObject.h"
#include "RendererEngine.h"
#include "Component_IMeshResource.h"
#include "Component_MeshRenderer.h"
#include "Texture.h"
#include "BlendManager.h"

using namespace DirectX;
using namespace VERTEX;
using namespace RenderData;
using namespace Tool::UV;

//*---------------------------------------------------------------------------------------
//* @:MeshRenderer Class 
//*【?】コンストラクタ
//* 引数：1.オーナーオブジェクト
//* 引数：2.更新レイヤー
//*----------------------------------------------------------------------------------------
MeshRenderer::MeshRenderer(std::weak_ptr<GameObject> pOwner, int updateRank) : Render(pOwner,updateRank)
{
    this->set_Tag("MeshRenderer");
}


//*---------------------------------------------------------------------------------------
//* @:MeshRenderer Class 
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
MeshRenderer::~MeshRenderer()
{

}


//*---------------------------------------------------------------------------------------
//* @:MeshRenderer Class 
//*【?】初期化
//* 引数：1.RendererEngine
//* 返値：void
//*----------------------------------------------------------------------------------------
void MeshRenderer::Start(RendererEngine& renderer)
{

}


//*---------------------------------------------------------------------------------------
//* @:MeshRenderer Class 
//*【?】更新
//* 引数：1.RendererEngine
//* 返値：void
//*----------------------------------------------------------------------------------------
void MeshRenderer::Update(RendererEngine& renderer)
{

}


//*---------------------------------------------------------------------------------------
//* @:MeshRenderer Class 
//*【?】描画
//* 引数：1.RendererEngine
//* 返値：void
//*----------------------------------------------------------------------------------------
void MeshRenderer::Draw(RendererEngine& renderer)
{
    if (this->get_IsEnable() == false)
    {
        return;
    }

    auto pContext = renderer.get_DeviceContext();
    std::shared_ptr<MeshResourceData> meshInfo = m_pMeshResource.lock()->m_pMeshData;
    ID3D11Buffer* vtxBuff = meshInfo->pVertexBuffer;
    UINT vtxStride = meshInfo->VertexStride;
    ID3D11Buffer* idxBuff = meshInfo->pIndexBuffer;
    CB_TRANSFORM cbTransform = {};  
    CB_MATERIAL cbMaterial = {};

    auto transform = m_pOwner.lock()->get_Transform().lock();

    /* ========== 定数バッファの更新 ========== */
    // ワールド行列セット ==========================
    XMMATRIX worldMtx = transform->get_WorldMtx();
    worldMtx = XMMatrixTranspose(worldMtx);                 // 行列の転置
    XMStoreFloat4x4(&cbTransform.WorldMtx, worldMtx);       // XMMATRIX → XMFLOAT4X4変換

    // 通常パス **********************************************************
    if (renderer.get_CrntRenderPass() == RENDER_PASS::MAIN) {
        Master::m_pShaderManager->DeviceToSetShader(m_pMeshResource.lock()->get_ShaderType());

        // マテリアル取得
        auto pMatData = meshInfo->pMaterials.lock();

        // マテリアル情報セット ==========================
        CB_MATERIAL mat{};
        cbMaterial.Diffuse = pMatData->m_DiffuseColor;
        cbMaterial.Specular = pMatData->m_SpecularColor;
        cbMaterial.SpecularPower = pMatData->m_SpecularPower;
        cbMaterial.EmissivePower = pMatData->m_EmissivePower;
        cbMaterial.EmissiveColor = pMatData->m_EmissiveColor;
        cbMaterial.EnvironmentReflectionStrength = pMatData->m_EnvironmentReflectionStrength;
        cbMaterial.OffsetUV ;

        // 定数バッファをセット ==========================
        Master::m_pShaderManager->BindConstantBuffer(CONSTANT_BUFFER_TYPE::TRANSFORM, (void*)&cbTransform, sizeof(CB_TRANSFORM));
        Master::m_pShaderManager->BindConstantBuffer(CONSTANT_BUFFER_TYPE::MATERIAL, (void*)&cbMaterial, sizeof(CB_MATERIAL));

        // テクスチャセット ==========================
        ID3D11ShaderResourceView *diffuseSRV = nullptr;
        ID3D11ShaderResourceView *normalSRV = nullptr;
        ID3D11ShaderResourceView *specularSRV = nullptr;
        if (auto tex = pMatData->m_DiffuseMap.Texture.lock()) {
            diffuseSRV = tex.get()->get_SRV();
        }
        if (auto tex = pMatData->m_NormalMap.Texture.lock()) {
            normalSRV = tex.get()->get_SRV();
        }
        if (auto tex = pMatData->m_SpecularMap.Texture.lock()) {
            specularSRV = tex.get()->get_SRV();
        }

        // シェーダーリソースビューをセット
        pContext->PSSetShaderResources(0, 1, &diffuseSRV);
        pContext->PSSetShaderResources(1, 1, &normalSRV);
        pContext->PSSetShaderResources(2, 1, &specularSRV);
    }
    // シャドウパス **********************************************************
    else if (renderer.get_CrntRenderPass() == RENDER_PASS::SHADOW) {
        Master::m_pShaderManager->DeviceToSetShader(SHADER_TYPE::POST_SHADOWMAP);

        // 定数バッファをセット ==========================
        Master::m_pShaderManager->BindConstantBuffer(CONSTANT_BUFFER_TYPE::TRANSFORM, (void*)&cbTransform, sizeof(CB_TRANSFORM));
    }


    //カリング設定 ==========================
    renderer.RegisterCullMode(meshInfo->CullMode);

    //ブレンドステート設定 ==========================
    Master::m_pBlendManager->DeviceToSetBlendState(meshInfo->pMaterials.lock()->m_BlendMode);


    // 頂点＆インデックスバッファ設定 ==========================
    UINT offset = 0;
    pContext->IASetVertexBuffers(0, 1, &vtxBuff, &vtxStride, &offset);      // 頂点バッファをセット
    pContext->IASetIndexBuffer(idxBuff, DXGI_FORMAT_R16_UINT, 0);           // インデックスバッファをセット
    pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);// Set primitive topology 頂点の組み合わせ方

    // 描画コール：インデックス数は（三角形個 × 3頂点） ==========================
    pContext->DrawIndexed(meshInfo->NumIndex, 0, 0);
    //pContext->Draw(24,0);

    pContext->PSSetShaderResources(0, 0, nullptr);
    pContext->PSSetShaderResources(1, 0, nullptr);
    pContext->PSSetShaderResources(2, 0, nullptr);
}


//*---------------------------------------------------------------------------------------
//* @:MeshRenderer Class 
//*【?】IMeshResource参照用のポインタ設定
//* 引数：1.IMeshResource
//* 返値：void
//*----------------------------------------------------------------------------------------
void MeshRenderer::set_MeshResource(std::weak_ptr<class IMeshResource> meshResource)
{
    m_pMeshResource = meshResource;
}


//*---------------------------------------------------------------------------------------
//*【?】表示するかどうか
//*     フラスタムの判定 
//*
//* [引数]
//* & frustum : フラスタム
//*
//* [返値]
//* true : 表示
//* false : 非表示
//*----------------------------------------------------------------------------------------
bool MeshRenderer::IsVisible(const DirectX::BoundingFrustum& _frustum) const
{
    const auto resource = m_pMeshResource.lock();
    if (!resource)
        return true;
    
    const auto& bounds = resource->get_LocalBounds();

    if (!bounds.IsValid)
        return true;

    const auto owner = m_pOwner.lock();
    const auto transform = owner ? owner->get_Transform().lock() : nullptr;
    if (!transform)
        return true;

    // バウンディングボックスをワールド変換
    DirectX::BoundingBox worldBounds;
    bounds.Box.Transform(
        worldBounds,
        transform->get_WorldMtx()
    );

    return worldBounds.Intersects(_frustum);
}


