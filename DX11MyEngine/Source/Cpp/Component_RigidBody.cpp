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
    :IComponent(pOwner, updateRank)
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
//*【?】セットアップ
//*
//* [引数]
//* &engine : 物理エンジン
//* &desc   : 剛体セットアップデータ
//* [返値]なし
//*----------------------------------------------------------------------------------------
bool RigidBody::Setup(PhysicsEngine& engine, const RigidBodyDesc& desc)
{
    m_pEngine = &engine;
    m_Desc = desc;

    return true;
}


//*---------------------------------------------------------------------------------------
//*【?】解放
//*
//* [引数]なし
//* [返値]なし
//*----------------------------------------------------------------------------------------
void RigidBody::Release()
{
    m_pEngine = nullptr;
}

void RigidBody::AddForce(const VECTOR3::VEC3& force)
{

}

void RigidBody::AddImpulse(const VECTOR3::VEC3& impulse)
{

}
