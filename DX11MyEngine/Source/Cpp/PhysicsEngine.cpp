#include "pch.h"
#include "PhysicsEngine.h"

using namespace VECTOR3;
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

    // 衝突判定を管理
    m_pDispatcher = std::make_unique < btCollisionDispatcher>(m_pConfig.get());

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

    BoxShapeDesc desc;
    desc.boxHalfExtents = VEC3(10.0f, 10.0f, 10.0f);
    desc.rdDesc.mass = 1.0f;
    desc.rdDesc.pos = VEC3(0.0f, 0.0f, 0.0f);
    RegisterShape(desc);
    
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

    //
    // コリジョンシェイプを削除
    //
    for (int j = 0; j < m_CollisionShapes.size(); j++)
    {
        btCollisionShape* shape = m_CollisionShapes[j];
        m_CollisionShapes[j] = 0;
        delete shape;
    }

    m_pWorld.reset();
    m_pSolver.reset();
    m_pBroadphase.reset();
    m_pDispatcher.reset();
    m_pConfig.reset();

    m_CollisionShapes.clear();
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

    m_RigidBodies[handle.index].rigidBody->applyForce(
        btVector3(force.x, force.y, force.z),
        btVector3(rel_pos.x, rel_pos.y, rel_pos.z)
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

    m_RigidBodies[handle.index].rigidBody->applyImpulse(
        btVector3(impulse.x, impulse.y, impulse.z),
        btVector3(rel_pos.x, rel_pos.y, rel_pos.z)
    );
}

//*---------------------------------------------------------------------------------------
//*【?】間的な回転力を加える
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

    m_RigidBodies[handle.index].rigidBody->applyTorqueImpulse(
        btVector3(angularImpulse.x, angularImpulse.y, angularImpulse.z)
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
}

//*---------------------------------------------------------------------------------------
//*【?】リジッドボディの作成
//*
//* [引数] なし
//* [返値] なし
//*----------------------------------------------------------------------------------------
PhysicsBodyHandle PhysicsEngine::CreateRigidBody(btCollisionShape* pShape, const VECTOR3::VEC3& pos, float mass)
{
    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(btVector3(pos.x, pos.y, pos.z));

    // MotionState
    btDefaultMotionState* motionState = new btDefaultMotionState(transform);

    // Info
    btRigidBody::btRigidBodyConstructionInfo info(
        (btScalar)mass,
        motionState,
        pShape
    );

    // 
    m_CollisionShapes.push_back(pShape);

    // RD
    btRigidBody* rigidBody = new btRigidBody(info);

    // リジッドボディを追加
    m_pWorld->addRigidBody(rigidBody);

    // ************************************************************
    // 
    // ポインタはこちらで削除する必要があるので、保持
    // 
    // ************************************************************
    
    PhysicsBodyHandle resultHandle;
    
    //
    // 空いている場所を探し再利用
    //
    for (uint32_t i = 0; i < m_RigidBodies.size(); i++)
    {
        RigidBodySlot& slot = m_RigidBodies[i];

        if (!slot.active)
        {
            slot.rigidBody = rigidBody;
            slot.active = true;

            resultHandle.index = i;
            resultHandle.generation = slot.generation;

            return resultHandle;
        }
    }

    //
    // 空いていないなら追加
    //
    RigidBodySlot rdSlot;
    rdSlot.active = true;
    rdSlot.rigidBody = rigidBody;

    // 配列に追加
    m_RigidBodies.push_back(rdSlot);

    resultHandle.index = static_cast<uint32_t>(m_RigidBodies.size() - 1);
    resultHandle.generation = rdSlot.generation;

    return resultHandle;
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
//*【?】ボックスシェイプ登録
//*-----------------------------------------------------------------------------------------
PhysicsBodyHandle PhysicsEngine::RegisterShape(const BoxShapeDesc& desc)
{
    btCollisionShape* shape = CreateShapeBox(desc.boxHalfExtents);
    return CreateRigidBody(shape, desc.rdDesc.pos, desc.rdDesc.mass);
}


//*-----------------------------------------------------------------------------------------
//*【?】球シェイプ登録
//*-----------------------------------------------------------------------------------------
PhysicsBodyHandle PhysicsEngine::RegisterShape(const SphereShapeDesc& desc)
{
    btCollisionShape* shape = CreateShapeSphere(desc.radius);
    return CreateRigidBody(shape, desc.rdDesc.pos, desc.rdDesc.mass);
}


//*-----------------------------------------------------------------------------------------
//*【?】カプセルシェイプ登録
//*-----------------------------------------------------------------------------------------
PhysicsBodyHandle PhysicsEngine::RegisterShape(const CapsuleShapeDesc& desc)
{
    btCollisionShape* shape = CreateShapeCapsule(desc.radius, desc.height);
    return CreateRigidBody(shape, desc.rdDesc.pos, desc.rdDesc.mass);
}

//*-----------------------------------------------------------------------------------------
//*【?】円柱シェイプ登録
//*-----------------------------------------------------------------------------------------
PhysicsBodyHandle PhysicsEngine::RegisterShape(const CylinderShapeDesc& desc)
{
    btCollisionShape* shape = CreateShapeCylinder(desc.halfExtents);
    return CreateRigidBody(shape, desc.rdDesc.pos, desc.rdDesc.mass);
}

//*-----------------------------------------------------------------------------------------
//*【?】円錐シェイプ登録
//*-----------------------------------------------------------------------------------------
PhysicsBodyHandle PhysicsEngine::RegisterShape(const ConeShapeDesc& desc)
{
    btCollisionShape* shape = CreateShapeCone(desc.radius, desc.height);
    return CreateRigidBody(shape, desc.rdDesc.pos, desc.rdDesc.mass);
}

//*-----------------------------------------------------------------------------------------
//*【?】三角錐シェイプ登録
//*-----------------------------------------------------------------------------------------
PhysicsBodyHandle PhysicsEngine::RegisterShape(const PyramidShapeDesc& desc)
{
    btCollisionShape* shape = CreateShapePyramid(desc.v4);
    return CreateRigidBody(shape, desc.rdDesc.pos, desc.rdDesc.mass);
}

//*-----------------------------------------------------------------------------------------
//*【?】三角形シェイプ登録
//*-----------------------------------------------------------------------------------------
PhysicsBodyHandle PhysicsEngine::RegisterShape(const TriangleShapeDesc& desc)
{
    btCollisionShape* shape = CreateShapeTriangle(desc.v3);
    return CreateRigidBody(shape, desc.rdDesc.pos, desc.rdDesc.mass);
}

//*-----------------------------------------------------------------------------------------
//*【?】線シェイプ登録
//*-----------------------------------------------------------------------------------------
PhysicsBodyHandle PhysicsEngine::RegisterShape(const LineShapeDesc& desc)
{
    btCollisionShape* shape = CreateShapeLine(desc.v2);
    return CreateRigidBody(shape, desc.rdDesc.pos, desc.rdDesc.mass);
}

//*-----------------------------------------------------------------------------------------
//*【?】点シェイプ登録
//*-----------------------------------------------------------------------------------------
PhysicsBodyHandle PhysicsEngine::RegisterShape(const PointShapeDesc& desc)
{
    btCollisionShape* shape = CreateShapePoint(desc.v1);
    return CreateRigidBody(shape, desc.rdDesc.pos, desc.rdDesc.mass);
}

//*-----------------------------------------------------------------------------------------
//*【?】凸包シェイプ登録
//*-----------------------------------------------------------------------------------------
PhysicsBodyHandle PhysicsEngine::RegisterShape(const ConvexHullShapeDesc& desc)
{
    btCollisionShape* shape = CreateShapeConvexHull(desc.points, desc.numPoints, desc.stride);
    return CreateRigidBody(shape, desc.rdDesc.pos, desc.rdDesc.mass);
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
btCapsuleShape* PhysicsEngine::CreateShapeCapsule(float radius, float height)
{
    return new btCapsuleShape(btScalar(radius), btScalar(height));
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
btConvexHullShape* PhysicsEngine::CreateShapeConvexHull(const float* points, int numPoints, int stride)
{
    return new btConvexHullShape(points, numPoints, stride);
}
