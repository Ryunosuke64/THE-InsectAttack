#include "pch.h"
#include "Component_SkyRenderer.h"
#include "pch.h"
#include "GameObject.h"
#include "RendererEngine.h"
#include "Component_IMeshResource.h"
#include "Component_MeshRenderer.h"
#include "Texture.h"
#include "BlendManager.h"

using namespace DirectX;
using namespace VERTEX;
using namespace Tool::UV;

//*---------------------------------------------------------------------------------------
//* @:SkyRenderer Class 
//*【?】コンストラクタ
//* 引数：1.オーナーオブジェクト
//* 引数：2.更新レイヤー
//*----------------------------------------------------------------------------------------
SkyRenderer::SkyRenderer(std::weak_ptr<GameObject> pOwner, int updateRank) : IComponent(pOwner, updateRank),
m_GeometryType(SKY_GEOMETRY_TYPE::BOX)
{
    this->set_Tag("SkyRenderer");
}


//*---------------------------------------------------------------------------------------
//* @:SkyRenderer Class 
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
SkyRenderer::~SkyRenderer()
{

}


//*---------------------------------------------------------------------------------------
//* @:SkyRenderer Class 
//*【?】初期化
//* 引数：1.RendererEngine
//* 返値：void
//*----------------------------------------------------------------------------------------
void SkyRenderer::Start(RendererEngine& renderer)
{

}


//*---------------------------------------------------------------------------------------
//* @:SkyRenderer Class 
//*【?】更新
//* 引数：1.RendererEngine
//* 返値：void
//*----------------------------------------------------------------------------------------
void SkyRenderer::Update(RendererEngine& renderer)
{

}


//*---------------------------------------------------------------------------------------
//* @:SkyRenderer Class 
//*【?】描画
//* 引数：1.RendererEngine
//* 返値：void
//*----------------------------------------------------------------------------------------
void SkyRenderer::Draw(RendererEngine& renderer)
{
    auto pContext = renderer.get_DeviceContext();
    std::shared_ptr<MeshResourceData> meshData = m_pMeshResource.lock()->m_pMeshData;
    ID3D11Buffer* vtxBuff = meshData->pVertexBuffer;
    ID3D11Buffer* idxBuff = meshData->pIndexBuffer;
	CB_TRANSFORM cbTransform = {};
	CB_MATERIAL cbMaterial = {};

	auto transform = m_pOwner.lock()->get_Transform().lock();


    // シェーダセット ==========================
    Master::m_pShaderManager->DeviceToSetShader(m_pMeshResource.lock()->get_ShaderType());

    /* ========== 定数バッファの更新 ========== */
    // ワールド行列セット ==========================
    XMMATRIX viewMtx = renderer.get_ViewMatrix();
    viewMtx.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);// 平行移動成分をゼロにする
    XMMATRIX worldMtx = transform->get_ExcludingRotWorldMtx();
    worldMtx *= viewMtx;
    XMMATRIX mtx = XMMatrixTranspose(worldMtx);         // 行列の転置
    XMStoreFloat4x4(&cbTransform.WorldMtx, mtx);        // XMMATRIX → XMFLOAT4X4変換


    // 定数バッファをセット ==========================
    Master::m_pShaderManager->BindConstantBuffer(CONSTANT_BUFFER_TYPE::TRANSFORM, (void*)&cbTransform, sizeof(CB_TRANSFORM));
    //pContext->PSSetConstantBuffers(4, 1, &cbMatSet->pBuff);   // マテリアルは今のところ必要なし

    // テクスチャセット ==========================
    ID3D11ShaderResourceView* diffuseSRV = nullptr;

    if(meshData->pMaterials.expired())
    {
        assert(false);
        return;
    }
    auto pMatData = meshData->pMaterials.lock();
    
    // 今のところディフューズのみ
    if (auto tex = pMatData->m_DiffuseMap.Texture.lock()) {
        diffuseSRV = tex.get()->get_SRV();
    }
    pContext->PSSetShaderResources(0, 1, &diffuseSRV);

    // 頂点＆インデックスバッファ設定 ==========================
    UINT stride = sizeof(VERTEX_Static);
    UINT offset = 0;
    pContext->IASetVertexBuffers(0, 1, &vtxBuff, &stride, &offset);         // 頂点バッファをセット
    pContext->IASetIndexBuffer(idxBuff, DXGI_FORMAT_R16_UINT, 0);           // インデックスバッファをセット
    pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);// Set primitive topology 頂点の組み合わせ方

    // 描画コール：インデックス数は（三角形個 × 3頂点） ==========================
    pContext->DrawIndexed(meshData->NumIndex, 0, 0);
}


//*---------------------------------------------------------------------------------------
//* @:SkyRenderer Class 
//*【?】IMeshResource参照用のポインタ設定
//* 引数：1.IMeshResource
//* 返値：void
//*----------------------------------------------------------------------------------------
void SkyRenderer::set_MeshResource(std::weak_ptr<class IMeshResource> meshResource)
{
    m_pMeshResource = meshResource;
}

std::shared_ptr<Material> SkyRenderer::get_Material() const
{
    const auto meshResource = m_pMeshResource.lock();
    if (!meshResource || !meshResource->m_pMeshData)
    {
        return nullptr;
    }
    return meshResource->m_pMeshData->pMaterials.lock();
}

bool SkyRenderer::set_Material(const std::shared_ptr<Material>& material)
{
    const auto meshResource = m_pMeshResource.lock();
    if (!meshResource || !meshResource->m_pMeshData || !material)
    {
        return false;
    }

    const auto texture = material->m_DiffuseMap.Texture.lock();
    if (!texture || !texture->get_SRV())
    {
        return false;
    }
    D3D11_SHADER_RESOURCE_VIEW_DESC textureDesc{};
    texture->get_SRV()->GetDesc(&textureDesc);
    if (textureDesc.ViewDimension != D3D11_SRV_DIMENSION_TEXTURECUBE)
    {
        return false;
    }

    // 共有プリセットの内容は変更せず、このSkyboxの参照先だけを切り替える。
    meshResource->m_pMeshData->pMaterials = material;
    return true;
}
