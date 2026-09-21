#include "pch.h"
#include "Component_BoxCollider.h"
#include "RendererEngine.h"
#include "DebugMesh.h"
#include "GameObject.h"

using namespace DirectX;
using namespace GIGA_Engine;
using namespace VECTOR3;
using namespace PhysicsData;

//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//* [引数]
//* pOwner : オーナーオブジェクト
//* updateRank : 更新レイヤー
//*----------------------------------------------------------------------------------------
BoxCollider::BoxCollider(std::weak_ptr<GameObject> pOwner, int updateRank)
    :Collider(pOwner, updateRank),
	m_HarfSize(VEC3(1.0f, 1.0f, 1.0f)),
    m_CollisionJudgmentType(COLLISION_JUDGMENT::AABB)
{
    this->set_Tag("BoxCollider");
}

//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
BoxCollider::~BoxCollider()
{

}

//*---------------------------------------------------------------------------------------
//*【?】初期化
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void BoxCollider::Start(RendererEngine &renderer)
{
    m_pBoxMesh = std::make_unique<DebugMesh>();
    bool res = m_pBoxMesh->Setup(renderer,DEBUG_MESHS_TYPE::CUBE);
    if (res == false)
    {
        assert(false);
        MessageBoxA(NULL, "デバッグ用メッシュの生成ができませんでした", "Collider", MB_OK);
    }

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
void BoxCollider::Update(RendererEngine &renderer)
{
	//auto pOwner = get_OwnerObj().lock();
	//if (!pOwner) return;

	//auto trans = pOwner->get_Component<MyTransform>();
	//if (!trans) return;

	//// DirectXMathの名前空間を使用
	//using namespace DirectX;

	//// 1. 最新のワールド行列を取得し、SIMDレジスタにロード
	//XMMATRIX matWorldCustom = trans->get_WorldMtx();
	//// ※お使いのMATRIX型がXMFLOAT4X4互換であれば、以下のようにロード可能です
	//XMMATRIX matWorld = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&matWorldCustom));

	//// 2. ワールド空間での中心座標を計算 (SIMDによる座標変換)
	//VECTOR3::VEC3 localCenterCustom = get_Center();
	//XMVECTOR localCenter = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&localCenterCustom));
	//XMVECTOR worldCenter = XMVector3TransformCoord(localCenter, matWorld);
	//XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&m_WorldOBB._center), worldCenter);

	//// 3. サイズ（ハーフサイズ）を同期
	//m_WorldOBB._harfLength = m_Size;
	//XMVECTOR extents = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&m_Size));

	//// 4. ワールド行列の回転成分（各行）からローカル3軸を取り出して正規化
	//// 行列のr[0], r[1], r[2]には、それぞれワールド空間上でのオブジェクトのX, Y, Z軸の向きが入っています
	//XMVECTOR axisX = XMVector3Normalize(matWorld.r[0]);
	//XMVECTOR axisY = XMVector3Normalize(matWorld.r[1]);
	//XMVECTOR axisZ = XMVector3Normalize(matWorld.r[2]);

	//m_WorldOBB._axis[0] = VEC3::FromXMVECTOR(axisX);
	//m_WorldOBB._axis[1] = VEC3::FromXMVECTOR(axisY);
	//m_WorldOBB._axis[2] = VEC3::FromXMVECTOR(axisZ);

	//// 5. 【最適化】OBBからAABBの広がりをSIMDで一括逆算
	//// 各ハーフサイズ（幅・高さ・奥行き）をレジスタの全成分に複製
	//XMVECTOR extX = XMVectorReplicate(XMVectorGetX(extents)); // (size.x, size.x, size.x, size.x)
	//XMVECTOR extY = XMVectorReplicate(XMVectorGetY(extents)); // (size.y, size.y, size.y, size.y)
	//XMVECTOR extZ = XMVectorReplicate(XMVectorGetZ(extents)); // (size.z, size.z, size.z, size.z)

	//// AABBのハーフサイズ = |AxisX| * size.x + |AxisY| * size.y + |AxisZ| * size.z
	//// 軸ベクトルの全成分の絶対値をとり、それぞれのサイズを掛けて足し合わせる処理をSIMDで並列化
	//XMVECTOR aabbExtents = XMVectorMultiplyAdd(XMVectorAbs(axisX), extX,
	//	XMVectorMultiplyAdd(XMVectorAbs(axisY), extY,
	//		XMVectorMultiply(XMVectorAbs(axisZ), extZ)));

	//// Min / Max の計算
	//XMVECTOR aabbMin = XMVectorSubtract(worldCenter, aabbExtents);
	//XMVECTOR aabbMax = XMVectorAdd(worldCenter, aabbExtents);

	//// キャッシュに保存
	//m_BroadPhaseAABB._min = VEC3::FromXMVECTOR(aabbMin);
	//m_BroadPhaseAABB._max = VEC3::FromXMVECTOR(aabbMax);
}


//*---------------------------------------------------------------------------------------
//*【?】描画
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void BoxCollider::Draw(RendererEngine &renderer)
{
	if (m_IsDrawDebugMesh == false || m_pBoxMesh == nullptr)return;

	auto owner = m_pOwner.lock();
	if (!owner)return;

	auto transform = owner->get_Transform().lock();
	if (!transform)return;

	// CollisionManagerと同じ中心座標を使用する
	const VEC3 colliderCenter = transform->get_VEC3ToPos() + m_Center;

	// m_Sizeは判定側では半サイズとして使用されるため、描画時は全体の大きさに変換する
	const VEC3 colliderFullSize = m_HarfSize * 2.0f;

	const XMMATRIX scaleMtx = XMMatrixScaling(
		colliderFullSize.x,
		colliderFullSize.y,
		colliderFullSize.z);
	const XMMATRIX translationMtx = XMMatrixTranslation(
		colliderCenter.x,
		colliderCenter.y,
		colliderCenter.z);

	// 現在の衝突判定と同じ、回転を含まないAABBとして表示する
	m_pBoxMesh->Draw(renderer, scaleMtx * translationMtx);
}

PhysicsShapeDesc BoxCollider::GetShapeDesc()const
{
	PhysicsData::BoxShapeDesc desc;
	desc.boxHalfExtents = m_HarfSize;
	return desc;
}
