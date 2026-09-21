#include "pch.h"
#include "Component_SphereCollider.h"
#include "GameObject.h"
#include "RendererEngine.h"
#include "DebugMesh.h"

using namespace DirectX;
using namespace UtilityData;
using namespace GIGA_Engine;
using namespace VECTOR3;
using namespace PhysicsData;

//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//* [引数]
//* pOwner : オーナーオブジェクト
//* updateRank : 更新レイヤー
//*----------------------------------------------------------------------------------------
SphereCollider::SphereCollider(std::weak_ptr<GameObject> pOwner, int updateRank)
    :Collider(pOwner, updateRank),
    m_Radius(1.0f)
{
    this->set_Tag("SphereCollider");
    m_ColliderType = COLLIDER_TYPE::SPHERE;
}


//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
SphereCollider::~SphereCollider()
{

}


//*---------------------------------------------------------------------------------------
//*【?】初期化
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void SphereCollider::Start(RendererEngine &renderer)
{
    m_pSphereMesh = std::make_unique<DebugMesh>();
    bool res = m_pSphereMesh->Setup(renderer, DEBUG_MESHS_TYPE::CUBE);

    m_pTransform = m_pOwner.lock()->get_Transform().lock().get();

    if (m_pTransform == nullptr)
    {
        MessageBoxA(NULL, "コンポーネントが取得できませんでした。", "Collider", MB_OK);
    }
}


//*---------------------------------------------------------------------------------------
//*【?】更新
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void SphereCollider::Update(RendererEngine &renderer)
{

}


//*---------------------------------------------------------------------------------------
//*【?】描画
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void SphereCollider::Draw(RendererEngine &renderer)
{
    if (m_IsDrawDebugMesh == false)return;


    auto pContext = renderer.get_DeviceContext();
    XMMATRIX localMat = XMMatrixIdentity();

    auto transform = m_pOwner.lock()->get_Transform().lock();
    VEC3 ownerPos = transform->get_VEC3ToPos();

    XMVECTOR scl = VEC3(m_Radius);
    XMVECTOR pos = ownerPos + m_Center; // 中心位置のオフセットを足す

    XMMATRIX mtxS = XMMatrixScalingFromVector(scl);
    XMMATRIX mtxT = XMMatrixTranslationFromVector(pos);

    localMat = transform->get_ExcludingRotWorldMtx(mtxS, mtxT);

    // メッシュ表示
    m_pSphereMesh->Draw(renderer, localMat);
}

PhysicsShapeDesc SphereCollider::GetShapeDesc()const
{
    PhysicsData::SphereShapeDesc desc;
    desc.radius = m_Radius;
    return desc;
}