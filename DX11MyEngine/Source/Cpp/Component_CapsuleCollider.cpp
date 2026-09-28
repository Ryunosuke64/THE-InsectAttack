#include "pch.h"
#include "Component_CapsuleCollider.h"


using namespace VECTOR3;
using namespace PhysicsData;

//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//* [引数]
//* pOwner : オーナーオブジェクト
//* updateRank : 更新レイヤー
//*----------------------------------------------------------------------------------------
CapsuleCollider::CapsuleCollider(std::weak_ptr<GameObject> pOwner, int updateRank)
	:Collider(pOwner, updateRank),
	m_Height(0.0f),
	m_Radius(0.0f)
{
	this->set_Tag("CapsuleCollider");
}

//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
CapsuleCollider::~CapsuleCollider()
{

}

//*---------------------------------------------------------------------------------------
//*【?】初期化
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void CapsuleCollider::Start(RendererEngine& renderer)
{
}

//*---------------------------------------------------------------------------------
//*【?】シェイプ情報の取得
//*
//* [引数]なし
//* [返値]
//* シェイプ情報 
//*----------------------------------------------------------------------------------------
PhysicsShapeDesc CapsuleCollider::GetShapeDesc()const
{
	PhysicsData::CapsuleShapeDesc desc;
	desc.height = m_Height;
	desc.radius = m_Radius;
	return desc;
}
