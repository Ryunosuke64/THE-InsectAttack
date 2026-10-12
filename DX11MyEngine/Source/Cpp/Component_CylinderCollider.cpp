#include "pch.h"
#include "Component_CylinderCollider.h"


using namespace VECTOR3;
using namespace PhysicsData;

//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//* [引数]
//* pOwner : オーナーオブジェクト
//* updateRank : 更新レイヤー
//*----------------------------------------------------------------------------------------
CylinderCollider::CylinderCollider(std::weak_ptr<GameObject> pOwner, int updateRank)
	:Collider(pOwner, updateRank),
	m_Height(0.0f),
	m_Radius(0.0f),
	m_CapsuleAxis(CAPSULE_AXIS::Y)
{
	this->set_Tag("CylinderCollider");
}

//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
CylinderCollider::~CylinderCollider()
{

}

//*---------------------------------------------------------------------------------------
//*【?】初期化
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void CylinderCollider::Start(RendererEngine& renderer)
{
}

//*---------------------------------------------------------------------------------
//*【?】シェイプ情報の取得
//*
//* [引数]なし
//* [返値]
//* シェイプ情報 
//*----------------------------------------------------------------------------------------
PhysicsShapeDesc CylinderCollider::GetShapeDesc()const
{
	PhysicsData::CylinderShapeDesc desc;
	desc.halfExtents = VEC3(m_Radius, m_Height, m_Radius);
	return desc;
}
