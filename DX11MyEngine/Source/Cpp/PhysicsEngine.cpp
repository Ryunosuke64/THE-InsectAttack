#include "pch.h"
#include "PhysicsEngine.h"
#include <btBulletDynamicsCommon.h>


//*---------------------------------------------------------------------------------------
//*y?zƒRƒ“ƒXƒgƒ‰ƒNƒ^
//*----------------------------------------------------------------------------------------
PhysicsEngine::PhysicsEngine()
{

}

//*---------------------------------------------------------------------------------------
//*y?zƒfƒXƒgƒ‰ƒNƒ^
//*----------------------------------------------------------------------------------------
PhysicsEngine::~PhysicsEngine()
{

}

//*---------------------------------------------------------------------------------------
//*y?zƒZƒbƒgƒAƒbƒv
//*
//* [ˆø”]
//* ‚È‚µ
//* 
//* [•Ô’l]
//* true  : ¬Œ÷
//* false : Ž¸”s
//*----------------------------------------------------------------------------------------
bool PhysicsEngine::Setup()
{    
    // Õ“Ë”»’è‚ÌÝ’è
    m_pConfig = new btDefaultCollisionConfiguration();

    // Õ“Ë”»’è‚ðŠÇ—
    m_pDispatcher = new btCollisionDispatcher(m_pConfig);

    // LˆæÕ“Ë”»’è
    m_pBroadphase = new btDbvtBroadphase();

    // •¨—‰‰ŽZ‚Ìƒ\ƒ‹ƒo[
    m_pSolver = new btSequentialImpulseConstraintSolver();

    // •¨—ƒ[ƒ‹ƒh
    m_pWorld = new btDiscreteDynamicsWorld(
        m_pDispatcher,
        m_pBroadphase,
        m_pSolver,
        m_pConfig
    );

    // d—Í
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

    btRigidBody* groundBody =
        new btRigidBody(info);

    m_pWorld->addRigidBody(groundBody);

    return true;
}


//*---------------------------------------------------------------------------------------
//*y?zI—¹ˆ—
//*
//* [ˆø”]
//* ‚È‚µ
//* 
//* [•Ô’l]
//* true  : ¬Œ÷
//* false : Ž¸”s
//*----------------------------------------------------------------------------------------
bool PhysicsEngine::Shutdown()
{
    return true;
}

//*---------------------------------------------------------------------------------------
//*y?zXVˆ—
//*
//* [ˆø”] ‚È‚µ
//* [•Ô’l] ‚È‚µ
//*----------------------------------------------------------------------------------------
void PhysicsEngine::Update(float deltaTime)
{
    m_pWorld->stepSimulation(deltaTime);
}
