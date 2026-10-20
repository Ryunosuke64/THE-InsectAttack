#include "pch.h"
#include "MeshFactory.h"
#include "GameObjectManager.h"
#include "GameObject.h"
#include "RendererEngine.h"
#include "Component_ModelMeshResource.h"
#include "Component_ModelMeshRenderer.h"
#include "Component_MeshRenderer.h"
#include "Component_SkinnedMeshAnimator.h"
#include "Component_SpriteRenderer.h"
#include "Component_BillboardRenderer.h"
#include "Component_BillboardResource.h"
#include "Component_SkyRenderer.h"
#include "Component_DecalRenderer.h"
#include "ResourceManager.h"
#include "Texture.h"

using namespace GIGA_Engine;
using namespace VECTOR3;
using namespace VECTOR2;

//*---------------------------------------------------------------------------------------
//* @:MeshFactory Class 
//*【?】モデルの生成
//* 引数：1.CreateModelInfo&
//* 返値：std::weak_ptr<GameObject>
//*----------------------------------------------------------------------------------------
std::shared_ptr<class GameObject> MeshFactory::CreateModel(const CreateModelInfo& info)
{
    std::array<LODModelData, MAX_REGISTER_LOD>lodModels;

    // =================================================
    // LODモデルがあるなら
    // =================================================
    for (int i = 0; i < info.LODModels.size(); i++)
    {
        // パスが入ってないなら、飛ばす
        if (info.LODModels[i]._path.empty())
            continue;

        // モデルの読み込み
        std::shared_ptr<ModelData> modeldata = Master::m_pResourceManager->LoadModel(info.LODModels[i]._path.c_str());
        if (modeldata == nullptr)
        {
            MessageBoxA(NULL, "モデルが読み込めませんでした", "Error", MB_OK);
            return{};
        }

        // マテリアルの設定
        for (size_t i = 0; i < info.MatNum; i++)
        {
            if (!modeldata->SetupTextureMap(info.SetupMaterial[i].pMaterialData, info.SetupMaterial[i].Index))
            {
                MessageBoxA(NULL, "マテリアルが設定できませんでした", "Error", MB_OK);
            }
        }

        // 使用するシェーダの設定
        modeldata->set_ShaderType(info.ShaderType);
        modeldata->set_ShadowShaderType(info.Shadow_ShaderType);
        
        lodModels[i]._pModelData = modeldata;
        lodModels[i]._distance = info.LODModels[i]._distance;
    }

    auto obj = std::make_shared<GameObject>();
    obj->set_LayerRank(info.ObjLayer);  // 更新レイヤーの設定

    // オブジェクトの生成
    std::shared_ptr<GameObject> pModelObj = Instantiate3D(std::move(obj), info.IsTransparent);
    pModelObj->set_Tag(info.ObjTag.c_str());

    // オブジェクトの状態をアクティブにする
    if (info.IsActive) {
        pModelObj->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
    }

    // メッシュリソースコンポーネントを追加
    std::shared_ptr<ModelMeshResource> meshResource = pModelObj->add_Component<ModelMeshResource>();

    // リソースにモデル情報を持たせる（LODの品質は配列の順番準拠）
    meshResource->set_LODModelData(lodModels);

    // スキニングモデルの場合、アニメーションコンポーネントを追加
    if (info.ShaderType == SHADER_TYPE::DEFERRED_STD_SKINNED_N || info.ShaderType == SHADER_TYPE::_DEFERRED_STD_SKINNED)
    {
        // アニメーションコンポーネント追加
        if (info.IsAnim) {
            // Renderer よりも更新を速めに（シェーダに渡る情報がおかしくなるため）
            pModelObj->add_Component<SkinnedMeshAnimator>(99)->set_MeshResource(meshResource);;
            pModelObj->get_Component<SkinnedMeshAnimator>()->Start(*info.pRenderer);
            pModelObj->get_Component<SkinnedMeshAnimator>()->set_AnimIndex(info.InitAnimIndex);
        }
    }


    // 描画コンポーネント追加
    std::weak_ptr<ModelMeshRenderer> meshRenderer = pModelObj->add_Component<ModelMeshRenderer>();
    meshRenderer.lock()->set_MeshResource(meshResource);


    return pModelObj;
}



//*---------------------------------------------------------------------------------------
//* @:MeshFactory Class 
//*【?】汎用メッシュの生成
//* 引数：1.CreateUtilityMeshInfo&
//* 返値：std::weak_ptr<GameObject>
//*----------------------------------------------------------------------------------------
std::shared_ptr<class GameObject> MeshFactory::CreateUtilityMesh(const CreateUtilityMeshInfo& info)
{
    // オブジェクトの生成
    std::shared_ptr<GameObject> pObj = Instantiate3D(std::move(std::make_shared<GameObject>()), info.IsTransparent);
    pObj->set_Tag(info.ObjTag.c_str());
    pObj->set_LayerRank(info.ObjLayer);  // 更新レイヤーの設定


    // オブジェクトの状態をアクティブにする
    if (info.IsActive) {
        pObj->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
    }

    // 種別がなければそのままのオブジェクトを返す
    if (info.Type == UTILITY_MESH_TYPE::NONE)return pObj;

    // コンポーネントの追加
    auto meshResource = pObj->add_Component<IMeshResource>();
    auto meshRenderer = pObj->add_Component<MeshRenderer>();

    // リソースのセットアップ
    if (!meshResource->Setup(*info.pRenderer,info.ShaderType, info.Type, info.MaterialData->pMaterialData, info.MatNum, info.IsNormalMap, info.TilingScale))return {};
    
    // Rendererにリソースを設定
    meshRenderer->set_MeshResource(meshResource);

    return pObj;
}


//*---------------------------------------------------------------------------------------
//* @:MeshFactory Class 
//*【?】スプライトの生成
//* 引数：1.CreateSpriteInfo&
//* 返値：std::weak_ptr<GameObject>
//*----------------------------------------------------------------------------------------
std::shared_ptr<class GameObject> MeshFactory::CreateSprite(const CreateSpriteInfo &info)
{
    // オブジェクトの生成
    std::shared_ptr<GameObject> pObj = Instantiate2D(std::move(std::make_shared<GameObject>()), info.IsTransparent);
    pObj->set_Tag(info.ObjTag.c_str());

    // オブジェクトの状態をアクティブにする
    if (info.IsActive) {
        pObj->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
    }

    // TODO:無条件DontDestroy（RTが消されてしまうため）
    pObj->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_DONT_DESTROY);


    // コンポーネントの追加
    auto sprite = pObj->add_Component<SpriteRenderer>();

    // リソースのセットアップ
    if (!sprite->Setup(info))return {};

    return pObj;
}


//*---------------------------------------------------------------------------------------
//* @:MeshFactory Class 
//*【?】ビルボードの生成
//* 引数：1.CreateBillboradInfo&
//* 返値：std::weak_ptr<GameObject>
//*----------------------------------------------------------------------------------------
std::shared_ptr<class GameObject> MeshFactory::CreateBillboard(const CreateBillboradInfo& info)
{
    // オブジェクトの生成
    std::shared_ptr<GameObject> pObj = Instantiate3D(std::move(std::make_shared<GameObject>()), info.IsTransparent);
    pObj->set_Tag(info.ObjTag.c_str());

    // オブジェクトの状態をアクティブにする
    if (info.IsActive) {
        pObj->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
    }

    // コンポーネントの追加
    auto billboardResource = pObj->add_Component<BillboardResource>();
    auto billboardRenderer = pObj->add_Component<BillboardRenderer>();

    // リソースのセットアップ
    if (!billboardResource->Setup(*info.pRenderer, info.Type, info.MaterialData->pMaterialData, info.MatNum))return {};

    // Rendererにリソースを設定
    billboardRenderer->set_BillboardResource(billboardResource);

    return pObj;
}


//*---------------------------------------------------------------------------------------
//* @:MeshFactory Class 
//*【?】スカイボックスの生成
//* 引数：1.CreateSkyboxInfo&
//* 返値：std::weak_ptr<GameObject>
//*----------------------------------------------------------------------------------------
std::shared_ptr<class GameObject> MeshFactory::CreateSkybox(const CreateSkyboxInfo& info)
{
    // オブジェクトの生成
    std::shared_ptr<GameObject> pObj = Instantiate3D(std::move(std::make_shared<GameObject>()), info.IsTransparent);
    pObj->set_Tag(info.ObjTag.c_str());

    // オブジェクトの状態をアクティブにする
    if (info.IsActive) {
        pObj->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
    }

    // コンポーネントの追加
    auto meshResource = pObj->add_Component<IMeshResource>();
    auto skyRenderer = pObj->add_Component<SkyRenderer>();

    // リソースのセットアップ                                  ↓キューブにする
    if (!meshResource->Setup(*info.pRenderer, info.ShaderType, UTILITY_MESH_TYPE::CUBE, info.MaterialData->pMaterialData, info.MatNum, false, VEC2(1.0f,1.0f)))return {};

    // Rendererにリソースを設定
    skyRenderer->set_MeshResource(meshResource);

    return pObj;
}

//*---------------------------------------------------------------------------------------
//* @:MeshFactory Class 
//*【?】デカールボックスの生成
//* 引数：1.CreateDecalInfo&
//* 返値：std::shared_ptr<GameObject>
//*----------------------------------------------------------------------------------------
std::shared_ptr<class GameObject> MeshFactory::CreateDecal(const CreateDecalInfo &info)
{
    // オブジェクトの生成
    std::shared_ptr<GameObject> pObj = Instantiate3D(std::move(std::make_shared<GameObject>()), info.IsTransparent);
    pObj->set_Tag(info.ObjTag.c_str());

    // オブジェクトの状態をアクティブにする
    if (info.IsActive) {
        pObj->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
    }

    // コンポーネントの追加
    auto meshResource = pObj->add_Component<IMeshResource>();
    auto decalRenderer = pObj->add_Component<DecalRenderer>();

    // リソースのセットアップ                                  ↓キューブにする
    if (!meshResource->Setup(*info.pRenderer, info.ShaderType, UTILITY_MESH_TYPE::CUBE, info.MaterialData->pMaterialData, info.MatNum, false, VEC2(1.0f,1.0f)))return {};

    // Rendererにリソースを設定
    decalRenderer->set_MeshResource(meshResource);
    decalRenderer->set_IsDynamic(true);
    return pObj;
}
