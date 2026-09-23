#include "pch.h"
#include "Component_Collider.h"
#include "RendererEngine.h"
#include "GameObjectManager.h"
#include "GameObject.h"
#include "ResourceManager.h"
#include "Component_RigidBody.h"

using namespace GIGA_Engine;
using namespace VECTOR3;
using namespace UtilityData;

//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//* [引数]
//* pOwner : オーナーオブジェクト
//* updateRank : 更新レイヤー
//*----------------------------------------------------------------------------------------
Collider::Collider(std::weak_ptr<GameObject> pOwner, int updateRank) 
    : IComponent(pOwner, updateRank),
    m_pTransform(nullptr),
    m_IsEnable(true),
    m_IsConvex(true),
    m_IsTrigger(false),
    m_IsHit(false),
    m_IsStatic(false),
    m_IsDrawDebugMesh(false),
    m_Center(VEC3()),
    m_ColliderType(COLLIDER_TYPE::BOX),
    m_CategoryBits(COLLISION_CATEGORY::NONE),
    m_CollisionBitMask(static_cast<unsigned>(COLLISION_CATEGORY::EVERY)),    // 初期値は全衝突にする
    m_ResponseBitMask(0xFFFFFFFF)
{
    this->set_Tag("Collider");
}



//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
Collider::~Collider()
{

}

//*---------------------------------------------------------------------------------------
//*【?】衝突判定マスク一括設定
//*----------------------------------------------------------------------------------------
void Collider::set_CollisionBitMask(unsigned _mask)
{ 
    if (m_CollisionBitMask == _mask) {
        return; // 変更がなければ通知しない
    }

    m_CollisionBitMask = _mask;

    if (auto rigidBody = m_pRigidBody.lock()) {
        rigidBody->RefreshCollisionFilter();
    }
}

//*---------------------------------------------------------------------------------------
//*【?】衝突判定マスク特定のカテゴリを追加
//*----------------------------------------------------------------------------------------
void Collider::add_CollisionBitMask(UtilityData::COLLISION_CATEGORY _category)
{
    if (GIGA_Engine::BitFlag::CheckAll(static_cast<unsigned>(_category), m_CollisionBitMask)) {
        return; // 変更がなければ通知しない
    }

    GIGA_Engine::BitFlag::SetFlag(static_cast<unsigned>(_category), m_CollisionBitMask);

    if (auto rigidBody = m_pRigidBody.lock()) {
        rigidBody->RefreshCollisionFilter();
    }
}

//*---------------------------------------------------------------------------------------
//*【?】衝突判定マスク特定のカテゴリを除外
//*----------------------------------------------------------------------------------------
void Collider::remove_CollisionBitMask(UtilityData::COLLISION_CATEGORY _category) 
{ 
    if (!GIGA_Engine::BitFlag::CheckAll(static_cast<unsigned>(_category), m_CollisionBitMask)) {
        return; // 変更がなければ通知しない
    }

    GIGA_Engine::BitFlag::UnsetFlag(static_cast<unsigned>(_category), m_CollisionBitMask);

    if (auto rigidBody = m_pRigidBody.lock()) {
        rigidBody->RefreshCollisionFilter();
    }
}



//*---------------------------------------------------------------------------------------
//*【?】衝突時の応答の設定
//*
//* [引数]
//* _category : 衝突のカテゴリ
//* _response : 判定時の応答
//* [返値]なし  
//*----------------------------------------------------------------------------------------
void Collider::set_CollisionResponse(
    COLLISION_CATEGORY _category,
    COLLISION_RESPONSE _response)
{
    const unsigned categoryBit =
        static_cast<unsigned>(_category);

    switch (_response)
    {
        /* 判定を行わない */
    case COLLISION_RESPONSE::RESPONSE_IGNORE:
        BitFlag::UnsetFlag(static_cast<unsigned>(categoryBit), m_CollisionBitMask);
        BitFlag::UnsetFlag(static_cast<unsigned>(categoryBit), m_ResponseBitMask);
        break;
        /* 判定のみ行う */
    case COLLISION_RESPONSE::RESPONSE_OVERLAP:
        BitFlag::SetFlag(static_cast<unsigned>(categoryBit), m_CollisionBitMask);
        BitFlag::UnsetFlag(static_cast<unsigned>(categoryBit), m_ResponseBitMask);
        break;
        /* 判定を行い、押し出し処理も行う */
    case COLLISION_RESPONSE::RESPONSE_BLOCK:
        BitFlag::SetFlag(static_cast<unsigned>(categoryBit), m_CollisionBitMask);
        BitFlag::SetFlag(static_cast<unsigned>(categoryBit), m_ResponseBitMask);
        break;
    }
}

//*---------------------------------------------------------------------------------------
//*【?】指定のカテゴリとの衝突時の応答を取得する
//*
//* [引数]
//* _category : 衝突のカテゴリ
//* [返値]衝突応答  
//*----------------------------------------------------------------------------------------
COLLISION_RESPONSE Collider::get_Response(COLLISION_CATEGORY _otherCategory)const
{
    const unsigned categoryBit =
        static_cast<unsigned>(_otherCategory);

    // 接触判定自体を取らない
    if ((m_CollisionBitMask & categoryBit) == 0)
    {
        return COLLISION_RESPONSE::RESPONSE_IGNORE;
    }
    // 接触判定は取るが、押し返さない
    if ((m_ResponseBitMask & categoryBit) == 0)
    {
        return COLLISION_RESPONSE::RESPONSE_OVERLAP;
    }

    // 接触判定を取り、物理的に押し返す
    return COLLISION_RESPONSE::RESPONSE_BLOCK;
}


