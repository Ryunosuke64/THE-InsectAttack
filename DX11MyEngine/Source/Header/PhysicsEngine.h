#pragma once
#include <LinearMath/btAlignedObjectArray.h>
#include "ConstantPhysicsData.h"


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:PhysicsEngine Class --- */
//
// 【?】物理エンジン
//		BulletPhysicsの管理
// 
// [参考サイト]
// https://note.com/zerogram0g/n/n26a5a1b8c157
// http://bulletjpn.web.fc2.com/07_RigidBodyDynamics.html
//
// ***************************************************************************************
class PhysicsEngine
{
private:
	std::unique_ptr<class btDefaultCollisionConfiguration> m_pConfig;
	std::unique_ptr<class btCollisionDispatcher> m_pDispatcher;
	std::unique_ptr<class btBroadphaseInterface> m_pBroadphase;
	std::unique_ptr<class btSequentialImpulseConstraintSolver> m_pSolver;
	std::unique_ptr<class btDiscreteDynamicsWorld> m_pWorld;

	btAlignedObjectArray<PhysicsData::RigidBodySlot>m_RigidBodies;	// リジッドボディのポインタを保持
	btAlignedObjectArray <class btCollisionShape* > m_CollisionShapes;	// リジッドボディのポインタを保持
	
public:
	PhysicsEngine();
	~PhysicsEngine();

	bool Setup();
	void Update(float deltaTime);
	bool Shutdown();

	PhysicsData::PhysicsBodyHandle CreateRigidBody(const PhysicsData::RigidBodyDesc& desc);
	void UnregisterRigidBody(const PhysicsData::PhysicsBodyHandle& handle);

	bool IsValidRigidBody(const PhysicsData::PhysicsBodyHandle& handle)const;
	void SetLinearVelocity(const PhysicsData::PhysicsBodyHandle& handle, const VECTOR3::VEC3& velocity);
	void AddForce(const PhysicsData::PhysicsBodyHandle& handle, const VECTOR3::VEC3& force, const VECTOR3::VEC3& rel_pos);
	void AddCentralForce(const PhysicsData::PhysicsBodyHandle& handle, const VECTOR3::VEC3& force);
	void AddImpulse(const PhysicsData::PhysicsBodyHandle& handle, const VECTOR3::VEC3& impulse, const VECTOR3::VEC3& rel_pos);
	void AddCentralImpulse(const PhysicsData::PhysicsBodyHandle& handle, const VECTOR3::VEC3& impulse);
	void AddAngularImpulse(const PhysicsData::PhysicsBodyHandle& handle, const VECTOR3::VEC3& angularImpulse);
	void SetWorldTransform(const PhysicsData::PhysicsBodyHandle& handle, const VECTOR3::VEC3& pos, const VECTOR4::VEC4& rot);
	
	void SetMass(const PhysicsData::PhysicsBodyHandle& handle, float mass);
	void SetGrivity(const PhysicsData::PhysicsBodyHandle& handle, const VECTOR3::VEC3& gravity);
	void SetWorldPosition(
		const PhysicsData::PhysicsBodyHandle& handle,
		const VECTOR3::VEC3& pos);

	void SetMask(const PhysicsData::PhysicsBodyHandle& handle, unsigned group, unsigned mask);
	void SetUserPointer(const PhysicsData::PhysicsBodyHandle& handle, void* userP);


	VECTOR3::VEC3 GetWorldPosition(const PhysicsData::PhysicsBodyHandle& handle);
	VECTOR4::VEC4 GetRotation(const PhysicsData::PhysicsBodyHandle& handle);


	class btCollisionShape* CreateShape(const PhysicsData::PhysicsShapeDesc& descVariant,const VECTOR3::VEC3& center);
	class btCollisionShape* CreateShape(const PhysicsData::ErrorShapeDesc& desc);
	class btCollisionShape* CreateShape(const PhysicsData::BoxShapeDesc& desc);
	class btCollisionShape* CreateShape(const PhysicsData::SphereShapeDesc& desc);
	class btCollisionShape* CreateShape(const PhysicsData::CapsuleShapeDesc& desc);
	class btCollisionShape* CreateShape(const PhysicsData::CylinderShapeDesc& desc);
	class btCollisionShape* CreateShape(const PhysicsData::ConeShapeDesc& desc);
	class btCollisionShape* CreateShape(const PhysicsData::PyramidShapeDesc& desc);
	class btCollisionShape* CreateShape(const PhysicsData::TriangleShapeDesc& desc);
	class btCollisionShape* CreateShape(const PhysicsData::LineShapeDesc& desc);
	class btCollisionShape* CreateShape(const PhysicsData::PointShapeDesc& desc);
	class btCollisionShape* CreateShape(const PhysicsData::ConvexHullShapeDesc& desc);
	class btCollisionShape* CreateShape(const PhysicsData::GImpactShapeDesc& desc);
	class btCollisionShape* CreateShape(const PhysicsData::BvhTriangleShapeDesc& desc);

	bool Raycast(const CollInData_Ray& ray, unsigned group, unsigned mask, class CollisionInfo* _hitInfo);
	std::vector<std::weak_ptr<GameObject>> CheckSphere(
		const VECTOR3::VEC3& position,
		float radius,
		int mask);

private:
	PhysicsEngine(const PhysicsEngine&) = delete;
	PhysicsEngine& operator=(const PhysicsEngine&) = delete;
	// ------------------------------------------------------



	class btBoxShape* CreateShapeBox(const VECTOR3::VEC3& boxHalfExtents);
	class btSphereShape* CreateShapeSphere(float radius);
	class btCapsuleShape* CreateShapeCapsule(float radius, float height);
	class btCylinderShape* CreateShapeCylinder(const VECTOR3::VEC3& halfExtents);
	class btConeShape* CreateShapeCone(float radius, float height);
	class btBU_Simplex1to4* CreateShapePyramid(const std::array<VECTOR3::VEC3, 4> v4);
	class btBU_Simplex1to4* CreateShapeTriangle(const std::array<VECTOR3::VEC3, 3> v3);
	class btBU_Simplex1to4* CreateShapeLine(const std::array<VECTOR3::VEC3, 2> v2);
	class btBU_Simplex1to4* CreateShapePoint(const VECTOR3::VEC3& v1);
	class btConvexHullShape* CreateShapeConvexHull(const std::vector<VERTEX::CollisionVertex>& vertexPositions);
	class btGImpactMeshShape* CreateGImpactMeshShape(const std::vector<VERTEX::CollisionVertex>& vertices, const std::vector<uint32_t>& indices);
	class btConvexTriangleMeshShape* CreateShapeConvexTriangleMesh();
	class btBvhTriangleMeshShape* CreateShapeBvhTriangleMesh(const std::vector<VERTEX::CollisionVertex>& verticesPos, const std::vector<uint32_t>& indices);
};

