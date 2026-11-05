#include "pch.h"
#include "Component_ImpactDamage.h"
#include "GameObject.h"

using namespace GIGA_Engine;
using namespace VECTOR3;

//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//* [引数]
//* pOwner : オーナーオブジェクト
//* updateRank : 更新レイヤー
//*----------------------------------------------------------------------------------------
ImpactDamage::ImpactDamage(std::weak_ptr<GameObject> pOwner, int updateRank)
    :IComponent(pOwner, updateRank)
{
    this->set_Tag("ImpactDamage");
}

//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
ImpactDamage::~ImpactDamage()
{

}

//*---------------------------------------------------------------------------------------
//*【?】初期化
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void ImpactDamage::Start(RendererEngine& renderer)
{
}


//*---------------------------------------------------------------------------------------
//*【?】更新
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void ImpactDamage::Update(RendererEngine& renderer)
{
    if (m_IsImapctEnabled == false)
    {
        return;
    }

    float deltaTime = Master::m_pTimeManager->get_DeltaTime();



    m_ElapsedTime += deltaTime;
}


//*---------------------------------------------------------------------------------------
//*【?】衝突処理
//*
//* [引数]
//* & _other : 衝突相手の情報
//* [返値]なし
//*----------------------------------------------------------------------------------------
void ImpactDamage::OnCollisionEnter(const PhysicsData::CollisionInfo& _other)
{
    if (m_IsImapctEnabled == false)
    {
        return;
    }

	VEC3 normal = _other.hitNormal;
}