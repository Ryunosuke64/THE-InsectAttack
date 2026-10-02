#include "pch.h"
#include "RendererEngine.h"
#include "Texture.h"
#include "GameObject.h"
#include "DebugMesh.h"
#include "DirectWriteManager.h"
#include "Component_ModelMeshRenderer.h"
#include "Component_ModelMeshResource.h"


using namespace DirectX;
using namespace VERTEX;
using namespace Tool::UV;
using namespace RenderData;

//*---------------------------------------------------------------------------------------
//* @:ModelMeshRenderer Class 
//*【?】コンストラクタ
//* 引数：1.オーナーオブジェクト
//* 引数：2.更新レイヤー
//*----------------------------------------------------------------------------------------
ModelMeshRenderer::ModelMeshRenderer(std::weak_ptr<GameObject> pOwner, int updateRank) : Render(pOwner, updateRank),
m_IsDrawWireframe(false),
m_DebugDrawBoneNum(0)
{
    this->set_Tag("ModelMeshRenderer");
}


//*---------------------------------------------------------------------------------------
//* @:ModelMeshRenderer Class 
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
ModelMeshRenderer::~ModelMeshRenderer()
{

}


//*---------------------------------------------------------------------------------------
//* @:ModelMeshRenderer Class 
//*【?】初期化
//* 引数：1.RendererEngine
//* 返値：void
//*----------------------------------------------------------------------------------------
void ModelMeshRenderer::Start(RendererEngine &renderer)
{
    // 初期化時に自身にMeshResourceコンポーネントがあれば設定する
    this->set_MeshResource(m_pOwner.lock()->get_Component<ModelMeshResource>());


}


//*---------------------------------------------------------------------------------------
//* @:ModelMeshRenderer Class 
//*【?】更新
//* 引数：1.RendererEngine
//* 返値：void
//*----------------------------------------------------------------------------------------
void ModelMeshRenderer::Update(RendererEngine &renderer)
{

}


//*---------------------------------------------------------------------------------------
//* @:ModelMeshRenderer Class 
//*【?】描画
//* 引数：1.RendererEngine
//* 返値：void
//*----------------------------------------------------------------------------------------
void ModelMeshRenderer::Draw(RendererEngine &renderer)
{
    if (this->get_IsEnable() == false)
    {
        return;
    }

    if (m_pMeshResource.lock() == nullptr) return;
    auto meshResource = m_pMeshResource.lock();

    //========================================================================
    // 距離に合わせて、LODの切り替えを行う
    //========================================================================
    VEC3 cameraPos = Master::m_pDataManager->get_CameraPos();
    VEC3 myPos = m_pOwner.lock()->get_TransformConst()->get_VEC3ToPos();
    float distanceFormCameraSq = VEC3::DistanceSq(cameraPos, myPos);

    // カメラからの距離からLODモデルを求める
    auto outLODModelData = meshResource->get_LODModelData(distanceFormCameraSq);
    auto& modelData = outLODModelData._pModelData;
    if (modelData == nullptr)return;

    auto pDeviceContext             = renderer.get_DeviceContext();
    std::vector<std::weak_ptr<Material>>matList    = modelData->get_MaterialList();
    const aiScene *pScene           = modelData->get_Scene();
    ModelMesh *pMeshes              = modelData->get_Meshes();
    UINT meshNum                    = modelData->get_MeshNum();
    UINT vertexNum                  = pMeshes->get_VertexNum();
    SHADER_TYPE shaderType          = modelData->get_ShaderType();
    SHADER_TYPE shadowShaderType    = modelData->get_ShadowShaderType();
	CB_TRANSFORM cbTransform{};
	CB_MATERIAL cbMaterial{};

    auto transform = m_pOwner.lock()->get_Transform().lock();

    /* デバッグモード指定の場合、ワイヤーフレームで表示 */
    if (m_IsDrawWireframe) {
        pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    }
    else {
        pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }
    
    // ワールド行列更新 ==========================
    XMMATRIX worldMtx = transform->get_WorldMtx();
    worldMtx = XMMatrixTranspose(worldMtx);                 // 行列の転置
    XMStoreFloat4x4(&cbTransform.WorldMtx, worldMtx);  // XMMATRIX → XMFLOAT4X4変換

    // 通常パス **********************************************************
    if (renderer.get_CrntRenderPass() == RENDER_PASS::MAIN) {
        Master::m_pShaderManager->DeviceToSetShader(shaderType);

        // トランスフォーム用定数バッファのセット
		Master::m_pShaderManager->BindConstantBuffer(CONSTANT_BUFFER_TYPE::TRANSFORM, (void*)&cbTransform, sizeof(CB_TRANSFORM));

        // メッシュの描画
        for (uint32_t meshIdx = 0; meshIdx < meshNum; meshIdx++)
        {
			// メッシュが非表示なら飛ばす
            if(pMeshes[meshIdx].get_IsVisible() == false) continue;

            aiMesh *mesh = pScene->mMeshes[meshIdx];
            auto mat = matList[mesh->mMaterialIndex].lock();

            // マテリアルが設定されていればマテリアル情報をセット
            if (mat != nullptr)
            {
                // 定数バッファにマテリアル情報セット ==========================
                cbMaterial.Diffuse = mat->m_DiffuseColor;
                cbMaterial.Specular = mat->m_SpecularColor;
                cbMaterial.SpecularPower = mat->m_SpecularPower;
                cbMaterial.EmissivePower = mat->m_EmissivePower;
                cbMaterial.EmissiveColor = mat->m_EmissiveColor;
                cbMaterial.EnvironmentReflectionStrength = mat->m_EnvironmentReflectionStrength;
                Master::m_pShaderManager->BindConstantBuffer(CONSTANT_BUFFER_TYPE::MATERIAL, (void*)&cbMaterial, sizeof(CB_MATERIAL));

                //ブレンドステート設定 ==========================
                Master::m_pBlendManager->DeviceToSetBlendState(mat->m_BlendMode);

                /* SRVの設定 */

                // ディフューズ
                if (mat->m_DiffuseMap.Texture.lock() != nullptr) {
                    auto diff = mat->m_DiffuseMap.Texture.lock()->get_SRV();
                    if (diff != nullptr)
                        pDeviceContext->PSSetShaderResources(0, 1, &diff);
                }

                // ノーマル
                if (mat->m_NormalMap.Texture.lock() != nullptr) {
                    auto norm = mat->m_NormalMap.Texture.lock()->get_SRV();
                    if (norm != nullptr)
                        pDeviceContext->PSSetShaderResources(1, 1, &norm);
                }
                // スペキュラ
                if (mat->m_SpecularMap.Texture.lock() != nullptr) {
                    auto spec = mat->m_SpecularMap.Texture.lock()->get_SRV();
                    if (spec != nullptr)
                        pDeviceContext->PSSetShaderResources(2, 1, &spec);
                }
            }

            // メッシュの描画
            pMeshes[meshIdx].Draw(renderer);

            // ブレンドオフ
            Master::m_pBlendManager->DeviceToSetBlendState(BLEND_MODE::NONE);
        }
    }
    // シャドウパス **********************************************************
    else if (renderer.get_CrntRenderPass() == RENDER_PASS::SHADOW) {

        // LODクオリティが低品質のものなら、シャドウは飛ばす
        if (outLODModelData._quality == LOD_QUALITY::LOW){
            return;
        }

        // スキニングメッシュかどうか
		if (pScene->mMeshes[0]->HasBones()) {  
            Master::m_pShaderManager->DeviceToSetShader(SHADER_TYPE::POST_SHADOWMAP_SKINNED);
        }
        else{
            Master::m_pShaderManager->DeviceToSetShader(SHADER_TYPE::POST_SHADOWMAP);
        }
        // トランスフォーム用定数バッファのセット
        Master::m_pShaderManager->BindConstantBuffer(CONSTANT_BUFFER_TYPE::TRANSFORM, (void*)&cbTransform, sizeof(CB_TRANSFORM));

        // メッシュの描画 マテリアル等なし
        for (u_int meshIdx = 0; meshIdx < meshNum; meshIdx++)
        {
            pMeshes[meshIdx].Draw(renderer);
        }
    }
}


//*---------------------------------------------------------------------------------------
//* @:ModelMeshRenderer Class 
//*【?】IMeshResource参照用のポインタ設定
//* 引数：1.IMeshResource
//* 返値：void
//*----------------------------------------------------------------------------------------
void ModelMeshRenderer::set_MeshResource(std::shared_ptr<class ModelMeshResource> meshResource)
{
    m_pMeshResource = meshResource;
}

//*---------------------------------------------------------------------------------------
//*【?】メッシュ数取得
//*
//* [引数] なし
//* [返値]
//* uint32_t : メッシュ数
//*----------------------------------------------------------------------------------------
uint32_t ModelMeshRenderer::get_MeshNum()const
{
    auto pMeshResource = m_pMeshResource.lock();
    if (pMeshResource == nullptr) {
        return 0;
    }

    return pMeshResource->get_ModelData().lock()->get_MeshNum();
}

//*---------------------------------------------------------------------------------------
//*【?】指定メッシュの表示 / 非表示を設定
//*
//* [引数]
//* meshIdx : メッシュインデックス
//* _flag : true:表示する false:表示しない
//*
//* [返値]
//* bool : 成功したかどうか 
//*----------------------------------------------------------------------------------------
bool ModelMeshRenderer::set_IsMeshVisible(uint32_t _meshIdx, bool _flag)
{
	auto pMeshResource = m_pMeshResource.lock();
    if (pMeshResource == nullptr){
        return false;
    }
    
    // メッシュ数
    uint32_t meshNum = get_MeshNum();

    // 範囲外ならfalse
    if (_meshIdx >= meshNum || _meshIdx < 0){
        return false;
    }

    // メッシュデータを取得して、表示 / 非表示を設定
	auto modelData = pMeshResource->get_ModelData().lock();
    auto meshes = modelData->get_Meshes();
	meshes[_meshIdx].set_IsVisible(_flag);

    return true;
}

//*---------------------------------------------------------------------------------------
//*【?】指定メッシュの表示 / 非表示のフラグを取得
//*
//* [引数]
//* meshIdx : メッシュインデックス
//*
//* [返値]
//* true : 表示 
//* false : 非表示
//*----------------------------------------------------------------------------------------
bool ModelMeshRenderer::get_IsMeshVisible(uint32_t _meshIdx)const
{
    auto pMeshResource = m_pMeshResource.lock();
    if (pMeshResource == nullptr){
        return false;
    }
    
    // メッシュ数
    uint32_t meshNum = get_MeshNum();
    // 範囲外ならfalse
    if (_meshIdx >= meshNum || _meshIdx < 0){
        return false;
    }
    // メッシュデータを取得して、表示 / 非表示を設定
    auto modelData = pMeshResource->get_ModelData().lock();
    auto meshes = modelData->get_Meshes();
    return meshes[_meshIdx].get_IsVisible();
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
bool ModelMeshRenderer::IsVisible(const DirectX::BoundingFrustum& _frustum) const
{
    const auto resource = m_pMeshResource.lock();
    if (!resource)
        return true;
    
    // モデルデータ＆ローカル境界があるか
    auto modelData = resource->get_ModelData().lock();
    if (!modelData || !modelData->get_HasLocalBounds())
        return true;

    // オーナーからトランスフォームを取得する
    const auto owner = m_pOwner.lock();
    const auto transform = owner ? owner->get_Transform().lock() : nullptr;
    if (!transform)
        return true;

    DirectX::BoundingBox worldBounds;

    // バウンディングボックスをワールド変換
    modelData->get_LocalBounds().Transform(
        worldBounds,
        transform->get_WorldMtx());

    // 判定する
    return _frustum.Intersects(worldBounds);
}
