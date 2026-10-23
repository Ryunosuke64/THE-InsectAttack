#include "pch.h"
#include "Component_BoneCollider.h"
#include "RendererEngine.h"
#include "DebugMesh.h"
#include "GameObject.h"

using namespace GIGA_Engine;
using namespace VECTOR3;
using namespace PhysicsData;

//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//* [引数]
//* pOwner : オーナーオブジェクト
//* updateRank : 更新レイヤー
//*----------------------------------------------------------------------------------------
BoneCollider::BoneCollider(std::weak_ptr<GameObject> pOwner, int updateRank)
    :Collider(pOwner, updateRank)
{
    this->set_Tag("BoneCollider");
}

//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
BoneCollider::~BoneCollider()
{

}

//*---------------------------------------------------------------------------------------
//*【?】初期化
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void BoneCollider::Start(RendererEngine& renderer)
{

}

//*---------------------------------------------------------------------------------------
//*【?】更新
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void BoneCollider::Update(RendererEngine& renderer)
{

}


//*---------------------------------------------------------------------------------
//*【?】シェイプ情報の取得
//*
//* [引数]なし
//* [返値]
//* シェイプ情報 
//*----------------------------------------------------------------------------------------
PhysicsShapeDesc BoneCollider::GetShapeDesc()const
{
	PhysicsData::BoxShapeDesc desc;
	return desc;
}
