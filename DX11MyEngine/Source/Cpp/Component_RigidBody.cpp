#include "pch.h"
#include "Component_RigidBody.h"
#include "PhysicsEngine.h"

using namespace GIGA_Engine;
using namespace VECTOR3;
using namespace UtilityData;

//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//* [引数]
//* pOwner : オーナーオブジェクト
//* updateRank : 更新レイヤー
//*----------------------------------------------------------------------------------------
RigidBody::RigidBody(std::weak_ptr<GameObject> pOwner, int updateRank)
    :IComponent(pOwner, updateRank),
    m_Velocity(VEC3()),
    m_ForceAccumulator(VEC3()),
    m_Mass(1.0f),
    m_GravityScale(9.8f),
    m_MaxSpeed(7.0f),
    m_Restitution(0.3f),
    m_MoveDrag(0.8f),
    m_AirDrag(0.95f),
    m_IsEnable(true),
    m_pRigidBodyBT(nullptr)
{
    this->set_Tag("RigidBody");
}

//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
RigidBody::~RigidBody()
{
}

//*---------------------------------------------------------------------------------------
//*【?】開始
//*
//* [引数]
//* &renderer : 描画エンジン
//* [返値]なし
//*----------------------------------------------------------------------------------------
void RigidBody::Start(RendererEngine& renderer)
{

}

//*---------------------------------------------------------------------------------------
//*【?】更新
//*
//* [引数]
//* &renderer : 描画エンジン
//* [返値]なし
//*----------------------------------------------------------------------------------------
void RigidBody::Update(RendererEngine& renderer)
{

}