#pragma once


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:PhysicsEngine Class --- */
//
// 【?】物理エンジン
//		BulletPhysicsの管理
// 
// [参考サイト]https://note.com/zerogram0g/n/n26a5a1b8c157
//
// ***************************************************************************************
class PhysicsEngine
{
private:
	class btDefaultCollisionConfiguration* m_pConfig;
	class btCollisionDispatcher* m_pDispatcher;
	class btBroadphaseInterface* m_pBroadphase;
	class btSequentialImpulseConstraintSolver* m_pSolver;
	class btDiscreteDynamicsWorld* m_pWorld;

public:
	PhysicsEngine();
	~PhysicsEngine();

	bool Setup();
	void Update(float deltaTime);
	bool Shutdown();
};

