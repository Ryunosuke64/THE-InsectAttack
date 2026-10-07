#include "pch.h"
#include "RendererEngine.h"
#include "Component_DistortionEffect.h"

using namespace DirectX;
using namespace RenderData;
using namespace VERTEX;
using namespace VECTOR3;
using namespace VECTOR2;


//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//* [引数]
//* pOwner : オーナーオブジェクト
//* updateRank : 更新レイヤー
//*----------------------------------------------------------------------------------------
DistortionEffect::DistortionEffect(std::weak_ptr<GameObject> pOwner, int updateRank)
   :IComponent(pOwner, updateRank)
{
    this->set_Tag("DistortionEffect");
}

//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
DistortionEffect::~DistortionEffect()
{

}

//*---------------------------------------------------------------------------------------
//*【?】初期化
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void DistortionEffect::Start(RendererEngine& renderer)
{
}

//*---------------------------------------------------------------------------------------
//*【?】更新
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void DistortionEffect::Update(RendererEngine& renderer)
{
}

//*---------------------------------------------------------------------------------------
//*【?】描画
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void DistortionEffect::Draw(RendererEngine& renderer)
{
    // シャドウパス時には返す
    if (renderer.get_CrntRenderPass() == RENDER_PASS::SHADOW) {
        return;
    }

    auto pContext = renderer.get_DeviceContext();
    //CB_TRANSFORM_SET* cbTransSet = resourcePtr->m_pCBTransformSet;
    //CB_MATERIAL_SET* cbMatSet = resourcePtr->m_pCBMaterialDataSet;
    ID3D11Buffer* vtxBuff = m_pMeshData->pVertexBuffer;
    ID3D11Buffer* idxBuff = m_pMeshData->pIndexBuffer;
    CB_MATERIAL cbMaterial{};
    CB_TRANSFORM cbTransform{};

    std::shared_ptr<MyTransform> transform = m_pOwner.lock()->get_Transform().lock();

    // シェーダセット ==========================
    Master::m_pShaderManager->DeviceToSetShader(SHADER_TYPE::POST_DISTORTION);

    /* ========== 定数バッファの更新 ========== */
    XMMATRIX viewInvMtx = renderer.get_ViewInvMatrix(); // ビュー逆行列取得


    // ビルボードの回転を計算 ==========================
     
    // デフォルトの上方向ベクトル（ワールドのY軸）を基準にする
    const XMVECTOR baseUP = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    // ----------------------------------------------
    // 前方ベクトル
    // ----------------------------------------------
    XMVECTOR forward = viewInvMtx.r[2];     // [2]に前方向が入ってる（0:rg1:up2:fw）
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
     
    // 明示的に行列を設定
    XMMATRIX worldMtx =
        transform->get_WorldMtx(
            transform->get_MtxScale(),
            viewInvMtx,
            transform->get_MtxPos()
        );

    XMMATRIX mtx = XMMatrixTranspose(worldMtx);        // 行列の転置
    XMStoreFloat4x4(&cbTransform.WorldMtx, mtx);  // XMMATRIX → XMFLOAT4X4変換

	static float time = 0.0f;
	time += 0.01f;

	m_CBDistortion.Power = 0.3f; // ディストーションの強さ
	m_CBDistortion.Time = time;   // 時間を更新（アニメーションさせるためのもの）

    // 定数バッファをセット ==========================
    Master::m_pShaderManager->BindConstantBuffer(CONSTANT_BUFFER_TYPE::TRANSFORM, (void*)&cbTransform, sizeof(CB_TRANSFORM));
    Master::m_pShaderManager->BindConstantBuffer(CONSTANT_BUFFER_TYPE::DISTORTION, (void*)&m_CBDistortion, sizeof(CB_DISTORTION));


    // テクスチャセット ==========================
    ID3D11ShaderResourceView* noizeSRV = nullptr;
    ID3D11ShaderResourceView* sceneSRV = nullptr;

    //auto sceneTexture = Master::m_pResourceManager->Convert_SRVToTexture("RT_SceneFinal");
    //sceneSRV = sceneTexture->get_SRV();
    auto noizeTexture = Master::m_pResourceManager->LoadWIC_Texture(L"Resource/Texture/PerlinNoise.png");
    noizeSRV = noizeTexture->get_SRV();

	// デバイスにシェーダをセット ===================================================
	Master::m_pShaderManager->DeviceToSetShader(SHADER_TYPE::POST_DISTORTION);

    //pContext->PSSetShaderResources(0, 1, &sceneSRV);
    pContext->PSSetShaderResources(1, 1, &noizeSRV);

    // 頂点＆インデックスバッファ設定 ==========================
    UINT stride = sizeof(VERTEX_Static);
    UINT offset = 0;
    pContext->IASetVertexBuffers(0, 1, &vtxBuff, &stride, &offset); // 頂点バッファをセット
    pContext->IASetIndexBuffer(idxBuff, DXGI_FORMAT_R16_UINT, 0);    // インデックスバッファをセット
    pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);// Set primitive topology 頂点の組み合わせ方

    //ブレンドステート設定 ==========================
    Master::m_pBlendManager->DeviceToSetBlendState(BLEND_MODE::ALPHA);

    // 描画コール：インデックス数は（三角形個 × 3頂点） ==========================
    pContext->DrawIndexed(m_pMeshData->NumIndex, 0, 0);

    Master::m_pBlendManager->DeviceToSetBlendState(BLEND_MODE::NONE);

}

bool DistortionEffect::Setup(RendererEngine& renderer)
{
    auto pDevice = renderer.get_Device();
    m_pMeshData = std::make_shared<MeshResourceData>();

    /*
    *  メッシュ情報の作成
    */
    *m_pMeshData = MeshInfoFactory::CreateMesh(pDevice, g_QuadVertices, g_QuadVertexNum, g_QuadIndices, g_QuadIndexNum);

    return true;
}
