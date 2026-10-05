#include "pch.h"
#include <cmath>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/NarrowPhaseCollision/btRaycastCallback.h>

// Gimpact用
#include <BulletCollision/Gimpact/btGImpactShape.h>
#include <BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h>


#include "PhysicsEngine.h"
#include "CollisionInfo.h"
#include "Component_Collider.h"
#include "Component_BoxCollider.h"
#include "Component_SphereCollider.h"

using namespace VECTOR3;
using namespace VECTOR4;
using namespace PhysicsData;

//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//*----------------------------------------------------------------------------------------
PhysicsEngine::PhysicsEngine()
{

}

//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
PhysicsEngine::~PhysicsEngine()
{

}

//*---------------------------------------------------------------------------------------
//*【?】セットアップ
//*
//* [引数]
//* なし
//* 
//* [返値]
//* true  : 成功
//* false : 失敗
//*----------------------------------------------------------------------------------------
bool PhysicsEngine::Setup()
{    
    // 衝突判定の設定
    m_pConfig = std::make_unique<btDefaultCollisionConfiguration>();

    // 衝突判定を管理（自分で衝突定義したやつ）
    m_pDispatcher = std::make_unique <MyCollisionDispatcher>(m_pConfig.get());

    // GImpactの登録
    btGImpactCollisionAlgorithm::registerAlgorithm(
        m_pDispatcher.get()
    );

    // 広域衝突判定
    m_pBroadphase = std::make_unique <btDbvtBroadphase>();

    // 物理演算のソルバー
    m_pSolver = std::make_unique <btSequentialImpulseConstraintSolver>();

    // 物理ワールド
    m_pWorld = std::make_unique <btDiscreteDynamicsWorld>(
        m_pDispatcher.get(),
        m_pBroadphase.get(),
        m_pSolver.get(),
        m_pConfig.get()
    );

    // 重力
    m_pWorld->setGravity(btVector3(0.0f, -9.8f, 0.0f));
    return true;
}


//*---------------------------------------------------------------------------------------
//*【?】終了処理
//*
//* [引数]
//* なし
//* 
//* [返値]
//* true  : 成功
//* false : 失敗
//*----------------------------------------------------------------------------------------
bool PhysicsEngine::Shutdown()
{
    //
    // リジッドボディの削除
    //
    for (int i = m_pWorld->getNumCollisionObjects() - 1; i >= 0; i--)
    {
        btCollisionObject* obj = m_pWorld->getCollisionObjectArray()[i];
        btRigidBody* body = btRigidBody::upcast(obj);
        if (body && body->getMotionState())
        {
            delete body->getMotionState();
        }
        m_pWorld->removeCollisionObject(obj);
        delete obj;
    }

    ////
    //// コリジョンシェイプを削除
    ////
    //for (int j = 0; j < m_CollisionShapes.size(); j++)
    //{
    //    btCollisionShape* shape = m_CollisionShapes[j];
    //    m_CollisionShapes[j] = 0;
    //    delete shape;
    //}

    m_pWorld.reset();
    m_pSolver.reset();
    m_pBroadphase.reset();
    m_pDispatcher.reset();
    m_pConfig.reset();

    //m_CollisionShapes.clear();
    m_RigidBodies.clear();

    return true;
}

//*---------------------------------------------------------------------------------------
//*【?】更新処理
//*
//* [引数] なし
//* [返値] なし
//*----------------------------------------------------------------------------------------
void PhysicsEngine::Update(float deltaTime)
{
    m_pWorld->stepSimulation(deltaTime);

    CollectContacts();
    DispatchEvents();

    // 前フレームの情報として保持
    m_PrevCollisionPairs = std::move(m_CrntCollisionPairs);
    m_PrevTriggerPairs = std::move(m_CrntTriggerPairs);

    // 現在フレームの情報はクリア
    m_CrntCollisionPairs.clear();
    m_CrntTriggerPairs.clear();
}

//*---------------------------------------------------------------------------------------
//*【?】線形速度を設定
//*
//* [引数] 
//* & handle    : ハンドル
//* & velocity  : ベロシティ
//* 
//* [返値] なし
//*----------------------------------------------------------------------------------------
void PhysicsEngine::
SetLinearVelocity(const PhysicsData::PhysicsBodyHandle& handle, const VECTOR3::VEC3& velocity)
{
    // 有効状態でなければ返す
    if (!IsValidRigidBody(handle))
    {
        return;
    }

    auto& rb = m_RigidBodies[handle.index].rigidBody;

    rb->activate(true); // アクティブ状態にする

    rb->setLinearVelocity(
        btVector3(velocity.x, velocity.y, velocity.z)
    );
}

//*---------------------------------------------------------------------------------------
//*【?】力を加える
//*
//* [引数] 
//* & handle : ハンドル
//* & force  : 力
//* 
//* [返値] なし
//*----------------------------------------------------------------------------------------
void PhysicsEngine::
AddForce(const PhysicsData::PhysicsBodyHandle& handle, const VECTOR3::VEC3& force, const VECTOR3::VEC3& rel_pos)
{
    // 有効状態でなければ返す
    if (!IsValidRigidBody(handle))
    {
        return;
    }

    auto& rb = m_RigidBodies[handle.index].rigidBody;

    rb->activate(true); // アクティブ状態にする

    rb->applyForce(
        btVector3(force.x, force.y, force.z),
        btVector3(rel_pos.x, rel_pos.y, rel_pos.z)
    );
}

//*---------------------------------------------------------------------------------------
//*【?】重心に力を加える
//*     回転はなし
//*
//* [引数] 
//* & handle : ハンドル
//* & force  : 力
//* 
//* [返値] なし
//*----------------------------------------------------------------------------------------
void PhysicsEngine::
AddCentralForce(const PhysicsData::PhysicsBodyHandle& handle, const VECTOR3::VEC3& force)
{
    // 有効状態でなければ返す
    if (!IsValidRigidBody(handle))
    {
        return;
    }

    auto& rb = m_RigidBodies[handle.index].rigidBody;

    rb->activate(true); // アクティブ状態にする

    rb->applyCentralForce(
        btVector3(force.x, force.y, force.z)
    );
}

//*---------------------------------------------------------------------------------------
//*【?】衝撃を加える
//*
//* [引数] 
//* & handle : ハンドル
//* & force  : 衝撃力
//* 
//* [返値] なし
//*----------------------------------------------------------------------------------------
void PhysicsEngine::
AddImpulse(const PhysicsData::PhysicsBodyHandle& handle, const VECTOR3::VEC3& impulse, const VECTOR3::VEC3& rel_pos)
{
    // 有効状態でなければ返す
    if (!IsValidRigidBody(handle))
    {
        return;
    }

    auto& rb = m_RigidBodies[handle.index].rigidBody;

    rb->activate(true); // アクティブ状態にする

    rb->applyImpulse(
        btVector3(impulse.x, impulse.y, impulse.z),
        btVector3(rel_pos.x, rel_pos.y, rel_pos.z)
    );
}

//*---------------------------------------------------------------------------------------
//*【?】衝撃を加える
//*     回転はなし
//*
//* [引数] 
//* & handle : ハンドル
//* & force  : 衝撃力
//* 
//* [返値] なし
//*----------------------------------------------------------------------------------------
void PhysicsEngine::
AddCentralImpulse(const PhysicsData::PhysicsBodyHandle& handle, const VECTOR3::VEC3& impulse)
{
    // 有効状態でなければ返す
    if (!IsValidRigidBody(handle))
    {
        return;
    }

    auto& rb = m_RigidBodies[handle.index].rigidBody;

    rb->activate(true); // アクティブ状態にする

    rb->applyCentralImpulse(
        btVector3(impulse.x, impulse.y, impulse.z)
    );
}

//*---------------------------------------------------------------------------------------
//*【?】瞬間的な回転力を加える
//*     移動はしない 
//*
//* [引数]
//* &handle         : ハンドル
//* &angularImpulse : 衝撃ベクトル
//* 
//* [返値]なし
//*----------------------------------------------------------------------------------------
void PhysicsEngine::
AddAngularImpulse(const PhysicsData::PhysicsBodyHandle& handle, const VECTOR3::VEC3& angularImpulse)
{
    // 有効状態でなければ返す
    if (!IsValidRigidBody(handle))
    {
        return;
    }


    auto& rb = m_RigidBodies[handle.index].rigidBody;

    rb->activate(true); // アクティブ状態にする

    rb->applyTorqueImpulse(
        btVector3(angularImpulse.x, angularImpulse.y, angularImpulse.z)
    );
}

//*---------------------------------------------------------------------------------------
//*【?】トランスフォームを直接設定
//*
//* [引数]
//* &handle : ハンドル
//* &pos    : 位置
//* 
//* [返値]なし
//*----------------------------------------------------------------------------------------
void PhysicsEngine::
SetWorldTransform(const PhysicsData::PhysicsBodyHandle& handle, const VECTOR3::VEC3& pos, const VECTOR4::VEC4& rot)
{
    // 有効状態でなければ返す
    if (!IsValidRigidBody(handle))
    {
        return;
    }

    auto& rb = m_RigidBodies[handle.index].rigidBody;

    rb->activate(true); // アクティブ状態にする

    btTransform transform;
    transform.setOrigin(btVector3(pos.x, pos.y, pos.z));
    transform.setRotation(btQuaternion(rot.x, rot.y, rot.z, rot.w));

    rb->setWorldTransform(
        transform
    );
}

//*---------------------------------------------------------------------------------------
//*【?】質量を設定
//*
//* [引数] 
//* & handle : ハンドル
//* mass     : 質量
//* 
//* [返値] なし
//*----------------------------------------------------------------------------------------
void PhysicsEngine::
SetMass(const PhysicsData::PhysicsBodyHandle& handle, float mass)
{
    // 有効状態でなければ返す
    if (!IsValidRigidBody(handle))
    {
        return;
    }
    btVector3 inertia(0, 0, 0);
    auto body = m_RigidBodies[handle.index].rigidBody;
    auto shape = body->getCollisionShape();

    if (shape == nullptr) 
    {
        return;
    }

    if (mass > 0.0f)
    {
        shape->calculateLocalInertia(mass, inertia);
    }

    body->setMassProps(mass, inertia);

    // 慣性の更新
    body->updateInertiaTensor();
}

//*---------------------------------------------------------------------------------------
//*【?】反発係数を設定
//*
//* [引数] 
//* & handle    : ハンドル
//* restitution : 反発（0 = 跳ねない）
//* 
//* [返値] なし
//*----------------------------------------------------------------------------------------
void PhysicsEngine::
SetRestitution(const PhysicsData::PhysicsBodyHandle& handle, float restitution)
{
    // 有効状態でなければ返す
    if (!IsValidRigidBody(handle))
    {
        return;
    }
    auto body = m_RigidBodies[handle.index].rigidBody;

    body->setRestitution(btScalar(restitution));
}

//*---------------------------------------------------------------------------------------
//*【?】摩擦係数を設定
//*
//* [引数] 
//* & handle    : ハンドル
//* friction    : 摩擦（0 = 摩擦なし）
//* 
//* [返値] なし
//*----------------------------------------------------------------------------------------
void PhysicsEngine::
SetFriction(const PhysicsData::PhysicsBodyHandle& handle, float friction)
{
    // 有効状態でなければ返す
    if (!IsValidRigidBody(handle))
    {
        return;
    }
    auto body = m_RigidBodies[handle.index].rigidBody;

    body->setFriction(btScalar(friction));
}


//*---------------------------------------------------------------------------------------
//*【?】重力を設定
//*
//* [引数] 
//* & handle : ハンドル
//* scale    : 重力
//* 
//* [返値] なし
//*----------------------------------------------------------------------------------------
void PhysicsEngine::
SetGrivity(const PhysicsData::PhysicsBodyHandle& handle, const VECTOR3::VEC3& gravity)
{
    // 有効状態でなければ返す
    if (!IsValidRigidBody(handle))
    {
        return;
    }

    auto body = m_RigidBodies[handle.index].rigidBody;
    
    body->setGravity(btVector3(gravity.x, gravity.y, gravity.z));
}


//*---------------------------------------------------------------------------------------
//*【?】ワールド座標を設定
//*
//* [引数] 
//* & handle : ハンドル
//* pos      : 座標
//* 
//* [返値] なし
//*----------------------------------------------------------------------------------------
void PhysicsEngine::
SetWorldPosition(
    const PhysicsData::PhysicsBodyHandle& handle,
    const VECTOR3::VEC3& pos)
{
    // 有効状態でなければ返す
    if (!IsValidRigidBody(handle))
    {
        return;
    }
    auto body = m_RigidBodies[handle.index].rigidBody;
    btTransform transform = body->getWorldTransform();

    // トランスフォームに位置を設定
    transform.setOrigin(btVector3(pos.x, pos.y, pos.z));

    body->activate(true);   // アクティブ状態に

    // トランスフォーム再設定
    body->setWorldTransform(transform);

    // モーションステートがあるならそっちも変更
    if (body->getMotionState())
    {
        body->getMotionState()->setWorldTransform(transform);
    }
}


//*---------------------------------------------------------------------------------------
//*【?】ワールド座標を取得
//*
//* [引数] 
//* & handle : ハンドル
//* [返値] 
//* ワールド座標
//*----------------------------------------------------------------------------------------
VECTOR3::VEC3 PhysicsEngine::
GetWorldPosition(const PhysicsData::PhysicsBodyHandle& handle)
{
    // 有効状態でなければ返す
    if (!IsValidRigidBody(handle))
    {
        return VEC3();
    }
    auto body = m_RigidBodies[handle.index].rigidBody;

    // トランスフォームを取得し、位置を取得
    btTransform transform = body->getWorldTransform();
    btVector3 pos = transform.getOrigin();

    return VEC3(
        FLOAT_CAST(pos.getX()),
        FLOAT_CAST(pos.getY()),
        FLOAT_CAST(pos.getZ())
    );
}


//*---------------------------------------------------------------------------------------
//*【?】回転を取得
//*
//* [引数] 
//* & handle : ハンドル
//* [返値] 
//* 回転
//*----------------------------------------------------------------------------------------
VECTOR4::VEC4 PhysicsEngine::
GetRotation(const PhysicsData::PhysicsBodyHandle& handle)
{
    // 有効状態でなければ返す
    if (!IsValidRigidBody(handle))
    {
        return VEC4();
    }
    auto body = m_RigidBodies[handle.index].rigidBody;

    // トランスフォームを取得し、回転を取得
    btTransform transform = body->getWorldTransform();
    btQuaternion rot = transform.getRotation();

    return VEC4(
        FLOAT_CAST(rot.getX()),
        FLOAT_CAST(rot.getY()),
        FLOAT_CAST(rot.getZ()),
        FLOAT_CAST(rot.getW())
    );
}

//*---------------------------------------------------------------------------------------
//*【?】線形速度を取得
//*
//* [引数] 
//* & handle : ハンドル
//* [返値] 
//* 線形速度
//*----------------------------------------------------------------------------------------
VECTOR3::VEC3 PhysicsEngine::
GetLinearVelocity(const PhysicsData::PhysicsBodyHandle& handle)const
{
    // 有効状態でなければ返す
    if (!IsValidRigidBody(handle))
    {
        return VEC3();
    }
    auto body = m_RigidBodies[handle.index].rigidBody;

    btVector3 vel = body->getLinearVelocity();

    return VEC3(
        FLOAT_CAST(vel.getX()),
        FLOAT_CAST(vel.getY()),
        FLOAT_CAST(vel.getZ())
    );
}


//*---------------------------------------------------------------------------------------
//*【?】マスクの設定
//*
//* [引数] 
//* & handle : ハンドル
//*  group   : 自身のグループ
//*  mask    : 衝突マスク
//* 
//* [返値] なし
//*----------------------------------------------------------------------------------------
void PhysicsEngine::
SetMask(const PhysicsData::PhysicsBodyHandle& handle, unsigned group, unsigned mask)
{
    // 有効状態でなければ返す
    if (!IsValidRigidBody(handle))
    {
        return;
    }
    auto* body = m_RigidBodies[handle.index].rigidBody;

    // 再登録時にワールド重力で上書きされる場合があるため保存
    const btVector3 gravity = body->getGravity();

    // 一旦worldから除外し追加しなおす
    // 除外しない場合、衝突相手の組み合わせが更新できないため
    m_pWorld->removeRigidBody(body);
    m_pWorld->addRigidBody(
        body,
        static_cast<int>(group),
        static_cast<int>(mask)
    );

    body->setGravity(gravity);
}


//*---------------------------------------------------------------------------------------
//*【?】リジッドボディへ、ユーザー設定用のポインタを設定する
//*
//* [引数] 
//* & handle : ハンドル
//* userP    : 格納するポインタ
//* 
//* [返値] なし
//*----------------------------------------------------------------------------------------
void PhysicsEngine::
SetUserPointer(const PhysicsData::PhysicsBodyHandle& handle, void* userP)
{
    // 有効状態でなければ返す
    if (!IsValidRigidBody(handle))
    {
        return;
    }

    auto* body = m_RigidBodies[handle.index].rigidBody;
    
    // ポインタ設定
    body->setUserPointer(userP);
}


//*---------------------------------------------------------------------------------------
//*【?】リジッドボディの作成
//*
//* [引数] なし
//* [返値] なし
//*----------------------------------------------------------------------------------------
PhysicsBodyHandle PhysicsEngine::
CreateRigidBody(const RigidBodyDesc& desc)
{
    PhysicsBodyHandle resultHandle;

    // コライダー取得
    auto collider = desc.collider.lock();

    if (!collider)
    {
        assert(false);
        MessageBox(NULL, L"コライダーが設定されていないため、シェイプの作成ができません", L"PhysicsEngine", MB_OK);
        return resultHandle;
    }
    
    VEC3 center = collider->get_Center();

    // シェイプ作成
    btCollisionShape* pShape = CreateShape(collider->GetShapeDesc(), center);

    if (pShape == nullptr)
    {
        MessageBox(NULL, L"シェイプが作成できませんでした", L"PhysicsEngine", MB_OK);
        assert(false);
        return resultHandle;
    }

    // トランスフォーム設定
    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(btVector3(desc.pos.x, desc.pos.y, desc.pos.z));

    // MotionState
    btDefaultMotionState* motionState = new btDefaultMotionState(transform);

    btVector3 localInertia = btVector3(0.0f, 0.0f, 0.0f);
    if (desc.mass > 0.0f) {
        pShape->calculateLocalInertia(desc.mass, localInertia);
    }

    // Info
    btRigidBody::btRigidBodyConstructionInfo info(
        (btScalar)desc.mass,
        motionState,
        pShape,
        localInertia
    );

    info.m_restitution = desc.restitution;  // 反発係数（大きいほど跳ねる）
    info.m_friction = desc.friction;        // 摩擦（大きいほど滑りにくい）

    // 
    m_CollisionShapes.push_back(pShape);

    // RD
    btRigidBody* rigidBody = new btRigidBody(info);

    // 自身の衝突マスクの種類
    const int group =
        static_cast<int>(collider->get_CollisionCategory());

    // 衝突マスク
    const int mask =
        static_cast<int>(collider->get_CollisionBitMask());

    // リジッドボディを追加
    m_pWorld->addRigidBody(rigidBody, group, mask);

    // 重力
    btVector3 gravity = btVector3(desc.gravity.x, desc.gravity.y, desc.gravity.z);
    rigidBody->setGravity(gravity);
    rigidBody->setRestitution(desc.restitution);
    rigidBody->setFriction(desc.friction);
    rigidBody->setAngularFactor(btVector3(desc.angularFactor.x, desc.angularFactor.y, desc.angularFactor.z));

    // ************************************************************
    // 
    // ポインタはこちらで削除する必要があるので、保持
    // 
    // ************************************************************
    
    
    //
    // 空いている場所を探し再利用
    //
    for (uint32_t i = 0; i < m_RigidBodies.size(); i++)
    {
        RigidBodySlot& slot = m_RigidBodies[i];

        if (!slot.active)
        {
            rigidBody->setUserIndex(i);

            slot.rigidBody = rigidBody;
            slot.active = true;
            slot.owner = desc.owner;
            slot.collider = desc.collider;

            resultHandle.index = i;
            resultHandle.generation = slot.generation;

            return resultHandle;
        }
    }

    //
    // 空いていないなら追加
    //
    uint32_t index = static_cast<uint32_t>(m_RigidBodies.size());

    RigidBodySlot rdSlot;
    rdSlot.active = true;
    rdSlot.rigidBody = rigidBody;
    rdSlot.owner = desc.owner;
    rdSlot.collider = desc.collider;
    rigidBody->setUserIndex(index);

    // 配列に追加
    m_RigidBodies.push_back(rdSlot);

    resultHandle.index = index;
    resultHandle.generation = rdSlot.generation;


    return resultHandle;
}


//*---------------------------------------------------------------------------------------
//*【?】リジッドボディの登録解除
//*
//* [引数]
//* handle : 登録解除するリジッドボディのID
//*
//* [返値]
//* なし
//*----------------------------------------------------------------------------------------
void PhysicsEngine::UnregisterRigidBody(const PhysicsData::PhysicsBodyHandle& handle)
{
    if (!IsValidRigidBody(handle)) {
        return;
    }

    // 状態をリセット
    RigidBodySlot& slot = m_RigidBodies[handle.index];
    btRigidBody* rigidBody = slot.rigidBody;
    btMotionState* motionState = rigidBody->getMotionState();
    btCollisionShape* shape = rigidBody->getCollisionShape();
    
    rigidBody->setUserPointer(nullptr);

    // リジッドボディをワールドから除外し削除
    m_pWorld->removeRigidBody(rigidBody);
    delete rigidBody;

    // モーションステート削除
    if (motionState != nullptr)
    {
        delete motionState;
    }
    // シェイプ削除
    if (shape != nullptr)
    {
        delete shape;
    }


    slot.rigidBody = nullptr;
    slot.active = false;

    // 次にこのindexが使用された際に、
    // 古いEnemyIDと区別するため
    slot.generation++;
}


//*---------------------------------------------------------------------------------------
//*【?】指定ハンドルのリジッドボディが有効状態か
//*
//* [引数]
//* handle : ハンドル
//*
//* [返値]
//* 有効かどうか
//*----------------------------------------------------------------------------------------
bool PhysicsEngine::IsValidRigidBody(const PhysicsBodyHandle& handle)const
{
    // インデックス範囲
    if (handle.index >= m_RigidBodies.size()) {
        return false;
    }

    const RigidBodySlot& slot = m_RigidBodies[handle.index];

    // 非アクティブ
    if (!slot.active) {
        return false;
    }

    if (slot.generation != handle.generation) {
        return false;
    }

    // ぬるぽチェック
    if (slot.rigidBody == nullptr) {
        return false;
    }


    // 有効状態
    return true;
}


//=========================================================================================
//
//						シェイプ登録関数群
//
//=========================================================================================

//*-----------------------------------------------------------------------------------------
//*【?】シェイプ登録  共用体ver
//*-----------------------------------------------------------------------------------------
btCollisionShape* PhysicsEngine::CreateShape(const PhysicsShapeDesc& descVariant, const VECTOR3::VEC3& center)
{
    btCollisionShape* childShape = nullptr;
    btCollisionShape* resultShape = nullptr;

    // 対応したシェイプ登録関数呼び出し
    std::visit([&](const auto& value)
        {
            childShape = CreateShape(value);
        }, descVariant
    );

    //
    // 中心オフセットがあるなら、子形状として中心を設定
    //
    if (center.x != 0.0f || center.y != 0.0f || center.z != 0.0f)
    {
        btTransform childTransform;
        childTransform.setIdentity();
        childTransform.setOrigin(
            btVector3(center.x, center.y, center.z));

        auto* compound = new btCompoundShape();
        compound->addChildShape(childTransform, childShape);

        // この形状を剛体に渡す
        resultShape = compound;
    }
    //
    //  中心オフセットがなければ、作成したシェイプをそのまま返すようにする
    //
    else
    {
        resultShape = childShape;
    }


    return resultShape;
}


//*-----------------------------------------------------------------------------------------
//*【?】ボックスシェイプ登録
//*-----------------------------------------------------------------------------------------
btCollisionShape* PhysicsEngine::CreateShape(const PhysicsData::ErrorShapeDesc& desc)
{
    return nullptr;
}

//*-----------------------------------------------------------------------------------------
//*【?】ボックスシェイプ登録
//*-----------------------------------------------------------------------------------------
btCollisionShape* PhysicsEngine::CreateShape(const BoxShapeDesc& desc)
{
    return CreateShapeBox(desc.boxHalfExtents);
}


//*-----------------------------------------------------------------------------------------
//*【?】球シェイプ登録
//*-----------------------------------------------------------------------------------------
btCollisionShape* PhysicsEngine::CreateShape(const SphereShapeDesc& desc)
{
    return CreateShapeSphere(desc.radius);
}


//*-----------------------------------------------------------------------------------------
//*【?】カプセルシェイプ登録
//*-----------------------------------------------------------------------------------------
btCollisionShape* PhysicsEngine::CreateShape(const CapsuleShapeDesc& desc)
{
    return CreateShapeCapsule(desc.radius, desc.height, desc.capsuleAxis);
}

//*-----------------------------------------------------------------------------------------
//*【?】円柱シェイプ登録
//*-----------------------------------------------------------------------------------------
btCollisionShape* PhysicsEngine::CreateShape(const CylinderShapeDesc& desc)
{
    return CreateShapeCylinder(desc.halfExtents);
}

//*-----------------------------------------------------------------------------------------
//*【?】円錐シェイプ登録
//*-----------------------------------------------------------------------------------------
btCollisionShape* PhysicsEngine::CreateShape(const ConeShapeDesc& desc)
{
    return CreateShapeCone(desc.radius, desc.height);
}

//*-----------------------------------------------------------------------------------------
//*【?】三角錐シェイプ登録
//*-----------------------------------------------------------------------------------------
btCollisionShape* PhysicsEngine::CreateShape(const PyramidShapeDesc& desc)
{
    return CreateShapePyramid(desc.v4);
}

//*-----------------------------------------------------------------------------------------
//*【?】三角形シェイプ登録
//*-----------------------------------------------------------------------------------------
btCollisionShape* PhysicsEngine::CreateShape(const TriangleShapeDesc& desc)
{
    return CreateShapeTriangle(desc.v3);
}

//*-----------------------------------------------------------------------------------------
//*【?】線シェイプ登録
//*-----------------------------------------------------------------------------------------
btCollisionShape* PhysicsEngine::CreateShape(const LineShapeDesc& desc)
{
    return CreateShapeLine(desc.v2);
}

//*-----------------------------------------------------------------------------------------
//*【?】点シェイプ登録
//*-----------------------------------------------------------------------------------------
btCollisionShape* PhysicsEngine::CreateShape(const PointShapeDesc& desc)
{
    return CreateShapePoint(desc.v1);
}

//*-----------------------------------------------------------------------------------------
//*【?】凸包シェイプ登録
//*-----------------------------------------------------------------------------------------
btCollisionShape* PhysicsEngine::CreateShape(const ConvexHullShapeDesc& desc)
{
    return CreateShapeConvexHull(desc.vertexPositions);
}

//*-----------------------------------------------------------------------------------------
//*【?】GImpactシェイプ登録
//*-----------------------------------------------------------------------------------------
btCollisionShape* PhysicsEngine::CreateShape(const GImpactShapeDesc& desc)
{
    return CreateGImpactMeshShape(desc.vertexPositions, desc.indices);
}

//*-----------------------------------------------------------------------------------------
//*【?】BVH三角形フルメッシュシェイプ登録
//*-----------------------------------------------------------------------------------------
btCollisionShape* PhysicsEngine::CreateShape(const BvhTriangleShapeDesc& desc)
{
    return CreateShapeBvhTriangleMesh(desc.vertexPositions, desc.indices);
}

//=========================================================================================
//
//						シェイプ作成関数群
//
//=========================================================================================

//*-----------------------------------------------------------------------------------------
//*【?】ボックスシェイプ作成
//*-----------------------------------------------------------------------------------------
btBoxShape* PhysicsEngine::CreateShapeBox(const VECTOR3::VEC3 & boxHalfExtents)
{
    return new btBoxShape(btVector3(boxHalfExtents.x, boxHalfExtents.y, boxHalfExtents.z));
}

//*-----------------------------------------------------------------------------------------
//*【?】球シェイプ作成
//*-----------------------------------------------------------------------------------------
btSphereShape* PhysicsEngine::CreateShapeSphere(float radius)
{
    return new btSphereShape(btScalar(radius));
}

//*-----------------------------------------------------------------------------------------
//*【?】カプセルシェイプ作成
//*-----------------------------------------------------------------------------------------
btCapsuleShape* PhysicsEngine::CreateShapeCapsule(float radius, float height, PhysicsData::CAPSULE_AXIS capsuleAxis)
{
    switch (capsuleAxis)
    {
    case PhysicsData::CAPSULE_AXIS::X:
        return new btCapsuleShapeX(btScalar(radius), btScalar(height));

    case PhysicsData::CAPSULE_AXIS::Y:
        return new btCapsuleShape(btScalar(radius), btScalar(height));

    case PhysicsData::CAPSULE_AXIS::Z:
        return new btCapsuleShapeZ(btScalar(radius), btScalar(height));

    default:
        assert(false && "Invalid capsule axis");
        break;
    }
}

//*-----------------------------------------------------------------------------------------
//*【?】円柱シェイプ作成
//*-----------------------------------------------------------------------------------------
btCylinderShape* PhysicsEngine::CreateShapeCylinder(const VECTOR3::VEC3 & halfExtents)
{
    return new btCylinderShape(btVector3(halfExtents.x, halfExtents.y, halfExtents.z));
}

//*-----------------------------------------------------------------------------------------
//*【?】円錐シェイプ作成
//*-----------------------------------------------------------------------------------------
btConeShape* PhysicsEngine::CreateShapeCone(float radius, float height)
{
    return new btConeShape(btScalar(radius), btScalar(height));
}

//*-----------------------------------------------------------------------------------------
//*【?】三角錐シェイプ作成
//*-----------------------------------------------------------------------------------------
btBU_Simplex1to4* PhysicsEngine::CreateShapePyramid(const std::array<VECTOR3::VEC3, 4> v4)
{
    btVector3 vertex[4] =
    {
        {v4[0].x,v4[0].y,v4[0].z},
        {v4[1].x,v4[1].y,v4[1].z},
        {v4[2].x,v4[2].y,v4[2].z},
        {v4[3].x,v4[3].y,v4[3].z},
    };
    return new btBU_Simplex1to4(vertex[0], vertex[1], vertex[2], vertex[3]);
}

//*-----------------------------------------------------------------------------------------
//*【?】三角形シェイプ作成
//*-----------------------------------------------------------------------------------------
btBU_Simplex1to4* PhysicsEngine::CreateShapeTriangle(const std::array<VECTOR3::VEC3, 3> v3)
{
    btVector3 vertex[3] =
    {
        {v3[0].x,v3[0].y,v3[0].z},
        {v3[1].x,v3[1].y,v3[1].z},
        {v3[2].x,v3[2].y,v3[2].z},
    };
    return new btBU_Simplex1to4(vertex[0], vertex[1], vertex[2]);
}

//*-----------------------------------------------------------------------------------------
//*【?】線シェイプ作成
//*-----------------------------------------------------------------------------------------
btBU_Simplex1to4* PhysicsEngine::CreateShapeLine(const std::array<VECTOR3::VEC3, 2> v2)
{
    btVector3 vertex[2] =
    {
        {v2[0].x,v2[0].y,v2[0].z},
        {v2[1].x,v2[1].y,v2[1].z},
    };
    return new btBU_Simplex1to4(vertex[0], vertex[1]);
}

//*-----------------------------------------------------------------------------------------
//*【?】点シェイプ作成
//*-----------------------------------------------------------------------------------------
btBU_Simplex1to4* PhysicsEngine::CreateShapePoint(const VECTOR3::VEC3 & v1)
{
    return new btBU_Simplex1to4(btVector3(v1.x, v1.y, v1.z));
}

//*-----------------------------------------------------------------------------------------
//*【?】凸包シェイプ作成
//*-----------------------------------------------------------------------------------------
btConvexHullShape* PhysicsEngine::CreateShapeConvexHull(const std::vector<VERTEX::CollisionVertex>& vertices)
{
    auto* shape = new btConvexHullShape();

    for (const auto& vertex : vertices)
    {
        const VEC3& p = vertex.position;

        shape->addPoint(
            btVector3(p.x, p.y, p.z),
            false   // ここではAABBを更新しない
        );
    }

    shape->recalcLocalAabb();

    return shape;
}

//*-----------------------------------------------------------------------------------------
//*【?】動的オブジェクト向けの凹メッシュ作成
//*-----------------------------------------------------------------------------------------
btGImpactMeshShape* PhysicsEngine::
CreateGImpactMeshShape(
    const std::vector<VERTEX::CollisionVertex>& vertices,
    const std::vector<uint32_t>& indices)
{
    btTriangleMesh* triangleMesh = new btTriangleMesh();

    // 三角形メッシュ追加していく
    for (int i = 0; i < indices.size(); i += 3)
    {
        const VEC3 p0 = vertices[indices[i]].position;
        const VEC3 p1 = vertices[indices[i + 1]].position;
        const VEC3 p2 = vertices[indices[i + 2]].position;

        triangleMesh->addTriangle(
            btVector3(p0.x, p0.y, p0.z),
            btVector3(p1.x, p1.y, p1.z),
            btVector3(p2.x, p2.y, p2.z)
        );
    }
    
    // GImpactメッシュシェイプ
    btGImpactMeshShape* shape =
        new btGImpactMeshShape(
            triangleMesh
        );

    // 境界情報の更新
    shape->updateBound();

    return shape;
}

//*-----------------------------------------------------------------------------------------
//*【?】BVH三角形フルメッシュシェイプ作成
//*-----------------------------------------------------------------------------------------
btBvhTriangleMeshShape* PhysicsEngine::
    CreateShapeBvhTriangleMesh(
        const std::vector<VERTEX::CollisionVertex>& vertices,
        const std::vector<uint32_t>& indices)
{
    btTriangleMesh* triangleMesh = new btTriangleMesh();

    // 三角形メッシュ追加していく
    for (int i = 0; i < indices.size(); i += 3)
    {
        const VEC3 p0 = vertices[indices[i]].position;
        const VEC3 p1 = vertices[indices[i + 1]].position;
        const VEC3 p2 = vertices[indices[i + 2]].position;

        triangleMesh->addTriangle(
            btVector3(p0.x, p0.y, p0.z),
            btVector3(p1.x, p1.y, p1.z),
            btVector3(p2.x, p2.y, p2.z)
        );
    }

    btBvhTriangleMeshShape* shape = 
        new btBvhTriangleMeshShape(
            triangleMesh, 
            true            // BVHのAABB圧縮を使用
        );

    return shape;
}


//*-----------------------------------------------------------------------------------------
//*【?】レイ判定
//*-----------------------------------------------------------------------------------------
bool PhysicsEngine::
Raycast(
    const CollInData_Ray& ray, 
    unsigned group, unsigned mask, 
    CollisionInfo* _hitInfo)
{
    btVector3 start_bt = btVector3(
        ray._point.x,
        ray._point.y, 
        ray._point.z
    );
    btVector3 end_bt = btVector3(
        ray._point.x + ray._dir.x, 
        ray._point.y + ray._dir.y, 
        ray._point.z + ray._dir.z
    );


    btCollisionWorld::ClosestRayResultCallback callback(
        start_bt,
        end_bt
    );

    // 衝突マスクの設定
    callback.m_collisionFilterGroup = static_cast<int>(group);
    callback.m_collisionFilterMask = static_cast<int>(mask);

    //    判定方式をGJKに変更
    // ※ 初期の方法では弾のすり抜けが発生してしまったため
    callback.m_flags |=
        btTriangleRaycastCallback::kF_UseGjkConvexCastRaytest;

    m_pWorld->rayTest(start_bt,end_bt, callback);

    // 衝突
    if (callback.hasHit())
    {
        // 衝突点
        VEC3 hitPoint = VEC3(
            callback.m_hitPointWorld.getX(),
            callback.m_hitPointWorld.getY(),
            callback.m_hitPointWorld.getZ()
        );

        // 衝突法線
        VEC3 hitNormal = VEC3(
            callback.m_hitNormalWorld.getX(),
            callback.m_hitNormalWorld.getY(),
            callback.m_hitNormalWorld.getZ()
        );

        // リジッドボディの生成時に設定したインデックスからリジッドボディスロットを検索
        const auto* hitBody = callback.m_collisionObject;
        int index = hitBody->getUserIndex();

        if (index >= m_RigidBodies.size())
        {
            assert(false);
            return false;
        }
        auto &hitObj = m_RigidBodies[index].owner;
        auto &hitCollider = m_RigidBodies[index].collider;

        // ヒット情報に入れる
        _hitInfo->hitPoint = hitPoint;
        _hitInfo->hitNormal = hitNormal;
        _hitInfo->hitObject = hitObj;
        _hitInfo->hitCollider = hitCollider;

        return true;
    }

    return false;
}

//*-----------------------------------------------------------------------------------------
//*【?】球を移動させたときの、経路上の最初の衝突を調べる
//*-----------------------------------------------------------------------------------------
bool PhysicsEngine::SphereCast(
    const VEC3& start,
    const VEC3& end,
    float radius,
    unsigned group,
    unsigned mask,
    SweepHitInfo* hitInfo,
    const GameObject* ignoreObject) const
{
    if (hitInfo)
    {
        *hitInfo = SweepHitInfo{};
    }

    const auto isFinite = [](const VEC3& position)
    {
        return std::isfinite(position.x) &&
            std::isfinite(position.y) && std::isfinite(position.z);
    };
    if (!m_pWorld || !std::isfinite(radius) || radius <= 0.0f ||
        !isFinite(start) || !isFinite(end) || group == 0 || mask == 0)
    {
        return false;
    }

    const btVector3 start_bt(start.x, start.y, start.z);
    const btVector3 end_bt(end.x, end.y, end.z);
    const btScalar distanceSq = (end_bt - start_bt).length2();
    if (!std::isfinite(distanceSq) || distanceSq <= SIMD_EPSILON * SIMD_EPSILON)
    {
        return false;
    }

    // 無効な剛体・Colliderと、指定されたオブジェクトを候補から除外する。
    // ヒット後の除外では、その奥にある有効な障害物を見逃してしまう。
    struct ClosestSphereResultCallback final
        : btCollisionWorld::ClosestConvexResultCallback
    {
        const btAlignedObjectArray<RigidBodySlot>& bodies;
        const GameObject* ignoredObject;

        ClosestSphereResultCallback(
            const btVector3& from,
            const btVector3& to,
            const btAlignedObjectArray<RigidBodySlot>& bodySlots,
            const GameObject* ignored)
            : btCollisionWorld::ClosestConvexResultCallback(from, to),
            bodies(bodySlots), ignoredObject(ignored)
        {
        }

        bool needsCollision(btBroadphaseProxy* proxy) const override
        {
            if (!proxy || !btCollisionWorld::ClosestConvexResultCallback::needsCollision(proxy))
            {
                return false;
            }

            const auto* object = static_cast<const btCollisionObject*>(proxy->m_clientObject);
            if (!object)
            {
                return false;
            }
            const int index = object->getUserIndex();
            if (index < 0 || index >= bodies.size())
            {
                return false;
            }

            const auto& slot = bodies[index];
            if (!slot.active || slot.rigidBody != object)
            {
                return false;
            }
            const auto owner = slot.owner.lock();
            const auto collider = slot.collider.lock();
            return owner && owner.get() != ignoredObject &&
                collider && collider->get_IsEnable();
        }
    };

    ClosestSphereResultCallback callback(start_bt, end_bt, m_RigidBodies, ignoreObject);
    callback.m_collisionFilterGroup = static_cast<int>(group);
    callback.m_collisionFilterMask = static_cast<int>(mask);

    // 一時的な判定形状。物理ワールドへの登録や剛体の移動は行わない。
    btSphereShape sphere(radius);
    btTransform from;
    btTransform to;
    from.setIdentity();
    to.setIdentity();
    from.setOrigin(start_bt);
    to.setOrigin(end_bt);
    m_pWorld->convexSweepTest(&sphere, from, to, callback);

    if (!callback.hasHit())
    {
        return false;
    }

    const auto handle = GetBodyHandle(callback.m_hitCollisionObject);
    if (!IsValidRigidBody(handle))
    {
        return false;
    }

    if (hitInfo)
    {
        const auto& slot = m_RigidBodies[handle.index];
        const btVector3 center = start_bt.lerp(end_bt, callback.m_closestHitFraction);
        hitInfo->hitFraction = static_cast<float>(callback.m_closestHitFraction);
        hitInfo->castPosition = VEC3(center.getX(), center.getY(), center.getZ());

        auto& collision = hitInfo->hitInfo;
        collision.hitObject = slot.owner;
        collision.hitCollider = slot.collider;
        if (const auto owner = slot.owner.lock())
        {
            collision.hitTransform = owner->get_Transform();
        }
        const auto& point = callback.m_hitPointWorld;
        const auto& normal = callback.m_hitNormalWorld;
        collision.hitPoint = VEC3(point.getX(), point.getY(), point.getZ());
        collision.hitNormal = VEC3(normal.getX(), normal.getY(), normal.getZ());
        collision.contactCount = 1;
        collision.contacts[0].position = collision.hitPoint;
        collision.contacts[0].normal = collision.hitNormal;
    }
    return true;
}

//*-----------------------------------------------------------------------------------------
//*【?】スフィア判定
//*-----------------------------------------------------------------------------------------
std::vector<std::weak_ptr<GameObject>> PhysicsEngine::
CheckSphere(
    const VECTOR3::VEC3& position,
    float radius,
    int mask)
{
    // 一時判定用シェイプを作る
    btSphereShape sphere(radius);

    btCollisionObject queryObject;
    queryObject.setCollisionShape(&sphere);

    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(
        btVector3(position.x, position.y, position.z)
    );

    queryObject.setWorldTransform(transform);

    // コールバック
    SphereContactCallback callback;
    callback.queryObject = &queryObject;
    callback.m_collisionFilterMask = mask;

    // 判定
    m_pWorld->contactTest(
        &queryObject,
        callback
    );

    std::vector<std::weak_ptr<GameObject>>resultGameObjects;

    // 範囲内のコリジョンオブジェクトからインデックスを取り出し、
    // ゲームオブジェクトを取り出す
    for (auto obj : callback.objects)
    {
        int index = obj->getUserIndex();

        if (index < 0 || index >= m_RigidBodies.size())
        {
            MessageBox(NULL, L"リジッドボディへのインデックスが範囲外です", L"PhysicsEngine", MB_OK);
            assert(false);
            continue;
        }
        auto& hitGameObj = m_RigidBodies[index].owner;

        if (!hitGameObj.expired())
        {
            resultGameObjects.push_back(hitGameObj);
        }
    }

    return resultGameObjects;
}



//*-----------------------------------------------------------------------------------------
//*【?】接触ペアを作る
//*-----------------------------------------------------------------------------------------
CollisionPair PhysicsEngine::
MakePair(const PhysicsBodyHandle& a, const PhysicsBodyHandle& b)
{
    // [b] が [a]より小さいなら順番を入れ替える
    // 同じペア同士で判定をさせない
    // 例:(A, B) と (B, A)は同じオブジェクト同士なのに、
    // 別のペアとして扱われてしまうため、順番をそろえる
    if (b.index < a.index ||
        (b.index == a.index && b.generation < a.generation))
    {
        return { b, a };
    }

    return { a, b };
}


//*-----------------------------------------------------------------------------------------
//*【?】指定オブジェクトに対応するコライダーを取得する
//*-----------------------------------------------------------------------------------------
Collider* PhysicsEngine::
GetCollider(const btCollisionObject* object)
{
    if (object == nullptr) { 
        return nullptr; 
    }

    int index = object->getUserIndex();

    // 範囲外チェック
    if (index < 0 || index >= m_RigidBodies.size())
    {
        return nullptr;
    }

    // コライダー取得
    if (auto collider = m_RigidBodies[index].collider.lock())
    {
        return collider.get();
    }

    return nullptr;
}

//*-----------------------------------------------------------------------------------------
//*【?】指定オブジェクトに対応する剛体ハンドルを取得する
//*-----------------------------------------------------------------------------------------
PhysicsBodyHandle PhysicsEngine::GetBodyHandle(
    const btCollisionObject* object) const
{
    if (!object)
        return {}; // 無効なハンドル

    const int index = object->getUserIndex();

    if (index < 0 || index >= m_RigidBodies.size())
        return {};

    const auto& slot = m_RigidBodies[index];

    // 登録中か、このオブジェクトのスロットかを確認
    if (!slot.active || slot.rigidBody != object)
        return {};

    return {
        static_cast<uint32_t>(index),
        slot.generation
    };
}


//*-----------------------------------------------------------------------------------------
//*【?】現在フレームの接触ペアを収集
//* 
//* [***** btPersistentManifold *****]
//*  接触情報のキャッシュで、最大4つの衝突点情報を持っている
//*  4つ以上、衝突点がある場合は、めり込み具合などで絞られる
//*  [btManifoldPoint]に衝突点情報が入っている
//* 
//*-----------------------------------------------------------------------------------------
void PhysicsEngine::CollectContacts()
{
    // 接触情報の数を取得
    int manifoldCount = m_pDispatcher->getNumManifolds();

    for (int i = 0; i < manifoldCount; i++)
    {
        // Manifoldの取得
        btPersistentManifold* manifold = m_pDispatcher->getManifoldByIndexInternal(i);

        const btCollisionObject* objectA = manifold->getBody0();
        const btCollisionObject* objectB = manifold->getBody1();

        PairCollisionInfo info;
        info.contactCount = 0;

        // 衝突点の数分
        for (int j = 0; j < manifold->getNumContacts(); j++)
        {
            // 衝突点情報の取得
            const btManifoldPoint& point = manifold->getContactPoint(j);

            // 2つのオブジェクトが、どれくらいめり込んでいるか
            // 古いキャッシュが残っており、接触していない可能性があるため、念のため確かめる
            // 有効な接触点があれば、ブレイクする
            if (point.getDistance() <= 0.0f)
            {
                PairContactPoint& contact = info.contacts[info.contactCount];

                // 接触位置 A側とB側の両方を保持
                const btVector3& posA = point.getPositionWorldOnA();
                const btVector3& posB = point.getPositionWorldOnB();

                contact.positionA = VEC3(
                    posA.x(), posA.y(), posA.z()
                );

                contact.positionB = VEC3(
                    posB.x(), posB.y(), posB.z()
                );

                // 法線
                const btVector3& normal = point.m_normalWorldOnB;

                contact.normalOnB = VEC3(
                    normal.x(),
                    normal.y(),
                    normal.z()
                );

                // getDistance()はどれだけ、離れているかを表しているので、
                // 負の値ならめり込んでいることになる
                // それを反転して、めり込み量にする
                contact.penetrationDepth =-point.getDistance();

                info.contactCount++;

            }
        }

        // 接触点が一つもなかった場合は飛ばす
        if (info.contactCount <= 0)
        {
            continue;
        }

        // ペアを作る
        const auto handleA = GetBodyHandle(objectA);
        const auto handleB = GetBodyHandle(objectB);

        if (!IsValidRigidBody(handleA) ||
            !IsValidRigidBody(handleB))
        {
            continue;
        }

        const CollisionPair pair = MakePair(handleA, handleB);

        // ペアの順序に、接触情報のA/Bも合わせる
        if (pair.a.index != handleA.index ||
            pair.a.generation != handleA.generation)
        {
            for (uint32_t i = 0; i < info.contactCount; ++i)
            {
                auto& contact = info.contacts[i];

                std::swap(contact.positionA, contact.positionB);
                contact.normalOnB = -contact.normalOnB;
            }
        }

        // オブジェクトからコライダーを取得する
        Collider* colliderA = GetCollider(objectA);
        Collider* colliderB = GetCollider(objectB);

        // コライダーがなければ飛ばす
        if (!colliderA || !colliderB)
        {
            continue;
        }

        //
        // トリガーとコリジョン、それぞれ衝突しているペアを記録
        //
        if (colliderA->get_IsTrigger() || colliderB->get_IsTrigger())
        {
            m_CrntTriggerPairs[pair] = info;
        }
        else
        {
            m_CrntCollisionPairs[pair] = info;
        }
    }
}

//*-----------------------------------------------------------------------------------------
//*【?】イベントの発行をする
//*-----------------------------------------------------------------------------------------
void PhysicsEngine::DispatchEvents()
{
    //==================================================
    // Collision：Enter / Stay
    // 今回接触しているペアを調べる
    //==================================================
    for (const auto& [pair, pairInfo] : m_CrntCollisionPairs)
    {
        const bool isEnter = !m_PrevCollisionPairs.contains(pair);

        // A側、B側の順に通知する
        for (bool notifyA : { true, false })
        {
            const auto& receiver = notifyA ? pair.a : pair.b;

            // 通知先の剛体が削除済みなら飛ばす
            if (!IsValidRigidBody(receiver))
                continue;

            auto owner = m_RigidBodies[receiver.index].owner.lock();
            if (!owner)
                continue;

            const CollisionInfo info =
                BuildCollisionInfo(pair, pairInfo, notifyA);

            if (isEnter)
            {
                owner->OnCollisionEnter(info);
            }
            else
            {
                owner->OnCollisionStay(info);
            }
        }
    }

    //==================================================
    // Collision：Exit
    // 前回は接触していたが、今回は接触していないペア
    //==================================================
    for (const auto& [pair, pairInfo] : m_PrevCollisionPairs)
    {
        if (m_CrntCollisionPairs.contains(pair))
            continue;

        for (bool notifyA : { true, false })
        {
            const auto& receiver = notifyA ? pair.a : pair.b;

            if (!IsValidRigidBody(receiver))
                continue;

            auto owner = m_RigidBodies[receiver.index].owner.lock();
            if (!owner)
                continue;

            // Exitには、最後に接触していた時点の情報を渡す
            const CollisionInfo info =
                BuildCollisionInfo(pair, pairInfo, notifyA);

            owner->OnCollisionExit(info);
        }
    }

    //==================================================
    // Trigger：Enter / Stay
    // 今回重なっているペアを調べる
    //==================================================
    for (const auto& [pair, pairInfo] : m_CrntTriggerPairs)
    {
        const bool isEnter = !m_PrevTriggerPairs.contains(pair);

        for (bool notifyA : { true, false })
        {
            const auto& receiver = notifyA ? pair.a : pair.b;

            if (!IsValidRigidBody(receiver))
                continue;

            auto owner = m_RigidBodies[receiver.index].owner.lock();
            if (!owner)
                continue;

            const CollisionInfo info =
                BuildCollisionInfo(pair, pairInfo, notifyA);

            if (isEnter)
            {
                owner->OnTriggerEnter(info);
            }
            else
            {
                owner->OnTriggerStay(info);
            }
        }
    }

    //==================================================
    // Trigger：Exit
    // 前回は重なっていたが、今回は重なっていないペア
    //==================================================
    for (const auto& [pair, pairInfo] : m_PrevTriggerPairs)
    {
        if (m_CrntTriggerPairs.contains(pair))
            continue;

        for (bool notifyA : { true, false })
        {
            const auto& receiver = notifyA ? pair.a : pair.b;

            if (!IsValidRigidBody(receiver))
                continue;

            auto owner = m_RigidBodies[receiver.index].owner.lock();
            if (!owner)
                continue;

            const CollisionInfo info =
                BuildCollisionInfo(pair, pairInfo, notifyA);

            owner->OnTriggerExit(info);
        }
    }
}

CollisionInfo PhysicsEngine::BuildCollisionInfo(
    const CollisionPair& pair,
    const PairCollisionInfo& pairInfo,
    bool notifyA) const
{
    CollisionInfo result{};

    // Aに通知するなら、衝突相手はB
    // Bに通知するなら、衝突相手はA
    const auto& other = notifyA ? pair.b : pair.a;

    if (IsValidRigidBody(other))
    {
        const auto& otherSlot = m_RigidBodies[other.index];

        result.hitObject = otherSlot.owner;
        result.hitCollider = otherSlot.collider;

        if (auto otherOwner = otherSlot.owner.lock())
        {
            result.hitTransform = otherOwner->get_Transform();
        }
    }

    // 保存されている接触点を、通知先の視点へ変換する
    for (uint32_t i = 0;
        i < pairInfo.contactCount &&
        i < pairInfo.contacts.size() &&
        i < result.contacts.size();
        i++)
    {
        const auto& source = pairInfo.contacts[i];
        auto& destination = result.contacts[i];

        // 接触位置：相手側の表面上の位置
        // 法線：相手から自分へ向かう方向
        if (notifyA)
        {
            destination.position = source.positionB;
            destination.normal = source.normalOnB;
        }
        else
        {
            destination.position = source.positionA;
            destination.normal = -source.normalOnB;
        }

        destination.penetrationDepth = source.penetrationDepth;
        result.contactCount++;
    }

    // 単一の接触位置を使う処理向けに、最初の点を代表値にする
    if (result.contactCount > 0)
    {
        const auto& firstContact = result.contacts[0];

        result.hitPoint = firstContact.position;
        result.hitNormal = firstContact.normal;
        result.penetrationDepth = firstContact.penetrationDepth;
    }

    return result;
}