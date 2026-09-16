#include "pch.h"
#include "PhysicsEngine.h"
#include <btBulletDynamicsCommon.h>

using namespace VECTOR3;

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
//*【?】リジッドボディの作成
//*
//* [引数] なし
//* [返値] なし
//*----------------------------------------------------------------------------------------
void PhysicsEngine::CreateRigidBody(btCollisionShape* pShape, const VECTOR3::VEC3& pos, float mass)
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

    // RD
    btRigidBody* rigidBody = new btRigidBody(info);

    // リジッドボディを追加
    m_pWorld->addRigidBody(rigidBody);

    // ポインタはこちらで削除する必要があるので、保持
    m_RBPtrs.push_back(rigidBody);
}


//=========================================================================================
//
//						シェイプ登録関数群
//
//=========================================================================================

//*-----------------------------------------------------------------------------------------
//*【?】ボックスシェイプ登録
//*-----------------------------------------------------------------------------------------
void PhysicsEngine::RegisterShape(const BoxShapeDesc& desc)
{
    btCollisionShape* shape = CreateShapeBox(desc.boxHalfExtents);
    CreateRigidBody(shape, desc.rdDesc.pos, desc.rdDesc.mass);
}


//*-----------------------------------------------------------------------------------------
//*【?】球シェイプ登録
//*-----------------------------------------------------------------------------------------
void PhysicsEngine::RegisterShape(const SphereShapeDesc& desc)
{
    btCollisionShape* shape = CreateShapeSphere(desc.radius);
    CreateRigidBody(shape, desc.rdDesc.pos, desc.rdDesc.mass);
}


//*-----------------------------------------------------------------------------------------
//*【?】カプセルシェイプ登録
//*-----------------------------------------------------------------------------------------
void PhysicsEngine::RegisterShape(const CapsuleShapeDesc& desc)
{
    btCollisionShape* shape = CreateShapeCapsule(desc.radius, desc.height);
    CreateRigidBody(shape, desc.rdDesc.pos, desc.rdDesc.mass);
}

//*-----------------------------------------------------------------------------------------
//*【?】円柱シェイプ登録
//*-----------------------------------------------------------------------------------------
void PhysicsEngine::RegisterShape(const CylinderShapeDesc& desc)
{
    btCollisionShape* shape = CreateShapeCylinder(desc.halfExtents);
    CreateRigidBody(shape, desc.rdDesc.pos, desc.rdDesc.mass);
}

//*-----------------------------------------------------------------------------------------
//*【?】円錐シェイプ登録
//*-----------------------------------------------------------------------------------------
void PhysicsEngine::RegisterShape(const ConeShapeDesc& desc)
{
    btCollisionShape* shape = CreateShapeCone(desc.radius, desc.height);
    CreateRigidBody(shape, desc.rdDesc.pos, desc.rdDesc.mass);
}

//*-----------------------------------------------------------------------------------------
//*【?】三角錐シェイプ登録
//*-----------------------------------------------------------------------------------------
void PhysicsEngine::RegisterShape(const PyramidShapeDesc& desc)
{
    btCollisionShape* shape = CreateShapePyramid(desc.v4);
    CreateRigidBody(shape, desc.rdDesc.pos, desc.rdDesc.mass);
}

//*-----------------------------------------------------------------------------------------
//*【?】三角形シェイプ登録
//*-----------------------------------------------------------------------------------------
void PhysicsEngine::RegisterShape(const TriangleShapeDesc& desc)
{
    btCollisionShape* shape = CreateShapeTriangle(desc.v3);
    CreateRigidBody(shape, desc.rdDesc.pos, desc.rdDesc.mass);
}

//*-----------------------------------------------------------------------------------------
//*【?】線シェイプ登録
//*-----------------------------------------------------------------------------------------
void PhysicsEngine::RegisterShape(const LineShapeDesc& desc)
{
    btCollisionShape* shape = CreateShapeLine(desc.v2);
    CreateRigidBody(shape, desc.rdDesc.pos, desc.rdDesc.mass);
}

//*-----------------------------------------------------------------------------------------
//*【?】点シェイプ登録
//*-----------------------------------------------------------------------------------------
void PhysicsEngine::RegisterShape(const PointShapeDesc& desc)
{
    btCollisionShape* shape = CreateShapePoint(desc.v1);
    CreateRigidBody(shape, desc.rdDesc.pos, desc.rdDesc.mass);
}

//*-----------------------------------------------------------------------------------------
//*【?】凸包シェイプ登録
//*-----------------------------------------------------------------------------------------
void PhysicsEngine::RegisterShape(const ConvexHullShapeDesc& desc)
{
    btCollisionShape* shape = CreateShapeConvexHull(desc.points, desc.numPoints, desc.stride);
    CreateRigidBody(shape, desc.rdDesc.pos, desc.rdDesc.mass);
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
