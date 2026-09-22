#include "pch.h"
#include "Component_MeshCollider.h"
#include "ModelData.h"

using namespace UtilityData;
using namespace VECTOR3;
using namespace PhysicsData;
using namespace VERTEX;

//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//* [引数]
//* pOwner : オーナーオブジェクト
//* updateRank : 更新レイヤー
//*----------------------------------------------------------------------------------------
MeshCollider::MeshCollider(std::weak_ptr<GameObject> pOwner, int updateRank)
    :Collider(pOwner, updateRank)
{
    this->set_Tag("MeshCollider");
}

//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
MeshCollider::~MeshCollider()
{

}


//*---------------------------------------------------------------------------------------
//*【?】初期化
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値] なし
//*----------------------------------------------------------------------------------------
void MeshCollider::Start(RendererEngine& renderer)
{
    m_pTransform = m_pOwner.lock()->get_Transform().lock().get();
}

//*---------------------------------------------------------------------------------------
//*【?】更新
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値] なし
//*----------------------------------------------------------------------------------------
void MeshCollider::Update(RendererEngine& renderer)
{

}

//*---------------------------------------------------------------------------------
//*【?】シェイプ情報の取得
//*
//* [引数]
//* modelData : モデルデータ
//* 
//* [返値] なし
//*----------------------------------------------------------------------------------------
void MeshCollider::SetupModelData(std::weak_ptr<class ModelData> modelData)
{
    m_pModelData = modelData;
}

//*---------------------------------------------------------------------------------
//*【?】シェイプ情報の取得
//*
//* [引数] なし
//* [返値]
//* シェイプ情報 
//*----------------------------------------------------------------------------------------
PhysicsShapeDesc MeshCollider::GetShapeDesc()const
{
    if (m_pModelData.expired())
    {
        MessageBox(NULL, L"使用するモデルの参照が設定されていません", L"MeshCollider", MB_OK);
        assert(false);
        return PhysicsData::ErrorShapeDesc();
    }

    // モデルデータからコリジョンメッシュ用データの取り出し
    std::vector<CollisionVertex> vertices = m_pModelData.lock()->get_CollisionVertices();
    std::vector<uint32_t> indices = m_pModelData.lock()->get_CollisionIndices();
    
    //*****************************************************************************************
    // 静的メッシュ 凸形状
    //*****************************************************************************************
    if (m_IsStatic)
    {
        PhysicsData::BvhTriangleShapeDesc desc;
        desc.vertexPositions = vertices;
        desc.indices = indices;
        
        return desc;
    }
    //*****************************************************************************************
    // 動的メッシュ 凸形状
    //*****************************************************************************************
    else if (m_IsConvex)
    {
        PhysicsData::ConvexHullShapeDesc desc;
        desc.vertexPositions = vertices;

        return desc;
    }
    //*****************************************************************************************
    // 動的メッシュ 凹形状
    //*****************************************************************************************
    else if(!m_IsConvex)
    {
        PhysicsData::GImpactShapeDesc desc;
        desc.vertexPositions = vertices;
        desc.indices = indices;

        return desc;
    }
}