#include "pch.h"
#include "PhysicsEngine.h"
#include <btBulletDynamicsCommon.h>


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

    btConvexHullShape;

    btCollisionShape* groundShape =
        new btBoxShape(btVector3(
            50.0f,
            1.0f,
            50.0f
        ));
    
    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(btVector3(0.0f, -1.0f, 0.0f));
    btScalar mass = 0.0f;
    btDefaultMotionState* motionState =
        new btDefaultMotionState(transform);

    btRigidBody::btRigidBodyConstructionInfo info(
        mass,
        motionState,
        groundShape
    );
    btRigidBody* groundBody = new btRigidBody(info);

    // リジッドボディを追加
    // ポインタはこちらで削除する必要があるので、保持
    m_pWorld->addRigidBody(groundBody);

    m_RBPtrs.push_back(groundBody);

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
