#include "pch.h"
#include "Component_BillboardRenderer.h"
#include "Component_BillboardResource.h"
#include "RendererEngine.h"
#include "GameObject.h"
#include "Texture.h"
#include "BlendManager.h"


using namespace DirectX;
using namespace VERTEX;
using namespace RenderData;
using namespace VECTOR3;
using namespace VECTOR4;

//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//* [引数]
//* std::weak_ptr<class GameObject> : オーナーオブジェクト
//* updateRank : 更新レイヤー
//*----------------------------------------------------------------------------------------
BillboardRenderer::BillboardRenderer(std::weak_ptr<class GameObject> pOwner, int updateRank) : 
    Render(pOwner, updateRank)
, m_ColorFactor(VEC4(1.0f))
{
    this->set_Tag("BillboardRenderer");
}

//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
BillboardRenderer::~BillboardRenderer()
{

}

//*---------------------------------------------------------------------------------------
//*【?】初期化
//*
//* [引数]
//* RendererEngine : 描画エンジンの参照
//*
//* [返値]
//* void
//*----------------------------------------------------------------------------------------
void BillboardRenderer::Start(RendererEngine& renderer)
{

}


//*---------------------------------------------------------------------------------------
//*【?】更新
//*
//* [引数]
//* RendererEngine : 描画エンジンの参照
//*
//* [返値]
//* void
//*----------------------------------------------------------------------------------------
void BillboardRenderer::Update(RendererEngine& renderer)
{

}


//*---------------------------------------------------------------------------------------
//*【?】描画
//*
//* [引数]
//* RendererEngine : 描画エンジンの参照
//*
//* [返値]
//* void
//*----------------------------------------------------------------------------------------
void BillboardRenderer::Draw(RendererEngine& renderer)
{
    if(this->get_IsEnable() == false)
    {
        return;
    }

    // シャドウパス時には返す
    if (renderer.get_CrntRenderPass() == RENDER_PASS::SHADOW) {
        return;
    }

    auto pContext = renderer.get_DeviceContext();
	auto resourcePtr = m_pResource.lock();
    std::shared_ptr<MeshResourceData> meshData = resourcePtr->m_pMeshData;
    //CB_TRANSFORM_SET* cbTransSet = resourcePtr->m_pCBTransformSet;
    //CB_MATERIAL_SET* cbMatSet = resourcePtr->m_pCBMaterialDataSet;
    ID3D11Buffer* vtxBuff = meshData->pVertexBuffer;
    ID3D11Buffer* idxBuff = meshData->pIndexBuffer;
    CB_MATERIAL cbMaterial{};
	CB_TRANSFORM cbTransform{};


    std::shared_ptr<MyTransform> transform = m_pOwner.lock()->get_Transform().lock();

    // シェーダセット ==========================
    Master::m_pShaderManager->DeviceToSetShader(SHADER_TYPE::FORWARD_UNLIT_STATIC);

    /* ========== 定数バッファの更新 ========== */
    XMMATRIX viewInvMtx = renderer.get_ViewInvMatrix(); // ビュー逆行列取得


	// ビルボードの回転を計算 ==========================

	// デフォルトの上方向ベクトル（ワールドのY軸）を基準にする
    const XMVECTOR baseUP = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    // ----------------------------------------------
    // 前方ベクトル
    // ----------------------------------------------
    XMVECTOR forward = viewInvMtx.r[2];     // [2]に前方向が入ってる（0:rg1:up2:fw）

    if (resourcePtr->m_FixedAxisBitFlag & FIXED_AXIS_BITFLAG_X) {
        forward = XMVectorSetX(forward, 0.0f);  // X成分を0にする（左右向かない）
    }
    if (resourcePtr->m_FixedAxisBitFlag & FIXED_AXIS_BITFLAG_Y) {
        forward = XMVectorSetY(forward, 0.0f);  // Y成分を0にする（上向かない）
    }
    if (resourcePtr->m_FixedAxisBitFlag & FIXED_AXIS_BITFLAG_Z) {
        forward = XMVectorSetZ(forward, 0.0f);  // Z成分を0にする（前後向かない）
    }
    forward = XMVector3Normalize(forward);  // 正規化

    
    // ----------------------------------------------
    // 右方向ベクトル
    // ----------------------------------------------
    XMVECTOR right = XMVector3Cross(baseUP, forward);   // 外積を使って右方向ベクトルを求める
    right = XMVector3Normalize(right);      // 正規化


    // ----------------------------------------------
	// 上方向ベクトル
    // ----------------------------------------------
    XMVECTOR up = XMVector3Cross(forward, right);
    up = XMVector3Normalize(up);            // 正規化


    viewInvMtx.r[0] = right;   // X軸
    viewInvMtx.r[1] = up;      // Y軸
    viewInvMtx.r[2] = forward; // Z軸
    viewInvMtx.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);// 平行移動成分をゼロにする

    // ワールド行列セット ==========================


    float rollRad =
        transform->get_VEC3ToRotateToRad().z;

    XMMATRIX rollMtx =
        XMMatrixRotationZ(rollRad);

    // 先に板をZ回転させ、その後カメラ方向へ向ける
    XMMATRIX billboardRotMtx =
        rollMtx * viewInvMtx;

    // 明示的に行列を設定
    XMMATRIX worldMtx =
        transform->get_WorldMtx(
            transform->get_MtxScale(),
            billboardRotMtx,
            transform->get_MtxPos()
        );

    XMMATRIX mtx = XMMatrixTranspose(worldMtx);        // 行列の転置
    XMStoreFloat4x4(&cbTransform.WorldMtx, mtx);  // XMMATRIX → XMFLOAT4X4変換

    auto pMatData = meshData->pMaterials.lock();

    // マテリアル情報セット ==========================
    VEC4 diffuse = pMatData->m_DiffuseColor * m_ColorFactor;
    cbMaterial.Diffuse = diffuse;
    cbMaterial.Specular = pMatData->m_SpecularColor;
    cbMaterial.SpecularPower = pMatData->m_SpecularPower;


    // 定数バッファをセット ==========================
    Master::m_pShaderManager->BindConstantBuffer(CONSTANT_BUFFER_TYPE::TRANSFORM, (void*)&cbTransform, sizeof(CB_TRANSFORM));
    Master::m_pShaderManager->BindConstantBuffer(CONSTANT_BUFFER_TYPE::MATERIAL, (void*)&cbMaterial, sizeof(CB_MATERIAL));


    // テクスチャセット ==========================
    ID3D11ShaderResourceView* diffuseSRV = nullptr;
    ID3D11ShaderResourceView* normalSRV = nullptr;
    ID3D11ShaderResourceView* specularSRV = nullptr;

    if (auto tex = pMatData->m_DiffuseMap.Texture.lock()) {
        diffuseSRV = tex.get()->get_SRV();
    }
    if (auto tex = pMatData->m_NormalMap.Texture.lock()) {
        normalSRV = tex.get()->get_SRV();
    }
    if (auto tex = pMatData->m_SpecularMap.Texture.lock()) {
        specularSRV = tex.get()->get_SRV();
    }

    pContext->PSSetShaderResources(0, 1, &diffuseSRV);
    //pContext->PSSetShaderResources(1, 1, &normalSRV);
    //pContext->PSSetShaderResources(2, 1, &specularSRV);

    // 頂点＆インデックスバッファ設定 ==========================
    UINT stride = sizeof(VERTEX_Static);
    UINT offset = 0;
    pContext->IASetVertexBuffers(0, 1, &vtxBuff, &stride, &offset); // 頂点バッファをセット
    pContext->IASetIndexBuffer(idxBuff, DXGI_FORMAT_R16_UINT, 0);    // インデックスバッファをセット
    pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);// Set primitive topology 頂点の組み合わせ方

    //ブレンドステート設定 ==========================
    Master::m_pBlendManager->DeviceToSetBlendState(pMatData->m_BlendMode);

    // 描画コール：インデックス数は（三角形個 × 3頂点） ==========================
    pContext->DrawIndexed(meshData->NumIndex, 0, 0);

    Master::m_pBlendManager->DeviceToSetBlendState(BLEND_MODE::NONE);
}


//*---------------------------------------------------------------------------------------
//* @:BillboardRenderer Class 
//*【?】BillboardResource参照用のポインタ設定
//* 引数：1.BillboardResource
//* 返値：void
//*----------------------------------------------------------------------------------------
void BillboardRenderer::set_BillboardResource(std::weak_ptr<BillboardResource> billboardResource)
{
    m_pResource = billboardResource;
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
bool BillboardRenderer::IsVisible(const DirectX::BoundingFrustum& _frustum) const
{
    const auto resource = m_pResource.lock();
    if (!resource)
        return true;

    const auto& bounds = resource->get_LocalBounds();

    // オーナーからトランスフォームを取得する
    const auto owner = m_pOwner.lock();
    const auto transform = owner ? owner->get_Transform().lock() : nullptr;
    if (!transform)
        return true;

    DirectX::BoundingSphere worldSphere;

    // バウンディングスフィアをワールド変換
    // ※スフィアなので回転は考慮しなくていい
    bounds.Sphere.Transform(
        worldSphere,
        transform->get_WorldMtx());

    // 判定する
    return _frustum.Intersects(worldSphere);
}