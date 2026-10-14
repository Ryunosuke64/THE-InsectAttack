#include "pch.h"
#include "Component_Destructible.h"
#include "Component_MapObject.h"
#include "Component_Health.h"
#include "Component_RigidBody.h"
#include "ConstantMapData.h"
#include "DestructionBehavior_Header.h"

using namespace Map;

//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//*----------------------------------------------------------------------------------------
Destructible::Destructible(std::weak_ptr<GameObject> pOwner, int updateRank) :
	IComponent(pOwner, updateRank),
    m_State(STATE::ALIVE)
{
}
//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
Destructible::~Destructible()
{
}

//*---------------------------------------------------------------------------------------
//*【?】初期化
//* 
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void Destructible::Start(RendererEngine& renderer)
{

}

//*---------------------------------------------------------------------------------------
//*【?】更新
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void Destructible::Update(RendererEngine& renderer)
{
    // 生存中なら返す
    if (m_State == STATE::ALIVE)
    {
        return;
    }
    // 破壊開始状態なら
    else if (m_State == STATE::DESTROYSTART)
    {
        if (m_pHealthComp.expired())
        {
            assert(false);
            return;
        }

        // 破壊を開始する
        auto health = m_pHealthComp.lock();
        const auto& collisionInfo = health->get_CollisionInfo();
        StartDestruction(collisionInfo.hitPoint, -collisionInfo.hitNormal);
    }

    if (!m_pBehavior)
        return;

    float deltaTime = Master::m_pTimeManager->get_DeltaTime();

    // ビヘイビアの更新
    m_pBehavior->Update(deltaTime);


    if (m_pBehavior->IsFinished())
    {
        // 破壊完了
        m_State = STATE::DESTROYED;

        // オブジェクトを削除する
        m_pOwner.lock()->set_StatusFlag(OBJECT_STATUS_BITFLAG::IS_DELETE);
    }
}

//*---------------------------------------------------------------------------------------
//*【?】更新
//*
//* [引数]
//* *behavior : 破壊のふるまいを定義したクラスのポインタ
//* [返値]
//* 成功したかどうか 
//*----------------------------------------------------------------------------------------
bool Destructible::Setup(std::unique_ptr<IDestructionBehavior> behavior)
{
    // 振る舞いを受け取る
    m_pBehavior = std::move(behavior);

    auto ownerObj = m_pOwner.lock();

    // 体力コンポーネントの取得
    m_pHealthComp = ownerObj->get_Component<Health>();
    if (m_pHealthComp.expired())
    {
        assert(false);
        return false;
    }
    auto health = m_pHealthComp.lock();

    //
    // 死んだ際のコールバック
    //
    health->RegisterOnDead(
        [&]()
        {
            // 破壊開始
            m_State = STATE::DESTROYSTART;
        }
    );

    // マップオブジェクトコンポーネントの取得
    m_pMapObjectComp = ownerObj->get_Component<MapObject>();
    if (m_pMapObjectComp.expired())
    {
        assert(false);
        return false;
    }

    return true;
}

//*---------------------------------------------------------------------------------------
//*【?】破壊の開始
//*
//* [引数]
//* hitPos : ダメージを受けた位置
//* hitDir : ダメージを与えた物体の方向
//* [返値]
//* 成功したかどうか 
//*----------------------------------------------------------------------------------------
void Destructible::StartDestruction(
	const VECTOR3::VEC3& hitPos,
	const VECTOR3::VEC3& hitDir)
{
    if (m_pBehavior)
    {
        if (m_pOwner.expired())
        {
            assert(false);
            return;
        }
        GameObject* ownerObj = m_pOwner.lock().get();

        // 破壊中状態へ
        m_State = STATE::DESTROYING;

        m_pBehavior->Start(
            ownerObj,
            hitPos,
            hitDir);
    }
}