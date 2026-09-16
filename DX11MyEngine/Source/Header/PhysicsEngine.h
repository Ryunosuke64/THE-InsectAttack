#pragma once

/// <summary>
/// コリジョンの形状
/// </summary>
enum class COLLISION_SHAPE
{
	BOX,				// ボックス
	SPHERE,				// 球
	CAPSULE,			// カプセル
	CYLINDER,			// 円柱
	CONE,				// 円錐
	PYRAMID,			// 三角錐
	TRIANGLE,			// 三角形
	LINE,				// 線
	POINT,				// 点
	CONVEX_HULL,		// 凸包
	CONVEX_TRIANGLE,	// 凸面三角形
	CONCAVE_TRIANGLE,	// 凹面三角形
	COMPOUND,			// 形状の組み合わせ
};

struct SetupRigidBodyDesc
{
	float mass = 0.0f;
	VECTOR3::VEC3 pos = VECTOR3::VEC3();
};

// ボックスシェイプセット用
struct BoxShapeDesc
{
	SetupRigidBodyDesc rdDesc;
	VECTOR3::VEC3 boxHalfExtents = VECTOR3::VEC3();
};

// 球シェイプセット用
struct SphereShapeDesc
{
	SetupRigidBodyDesc rdDesc;
	float radius = 0.0f;
};

// カプセルシェイプセット用
struct CapsuleShapeDesc
{
	SetupRigidBodyDesc rdDesc;
	float radius = 0.0f;
	float height = 0.0f;
};

// 円柱シェイプセット用
struct CylinderShapeDesc
{
	SetupRigidBodyDesc rdDesc;
	VECTOR3::VEC3 halfExtents = VECTOR3::VEC3();
};

// 円錐シェイプセット用
struct ConeShapeDesc
{
	SetupRigidBodyDesc rdDesc;
	float radius = 0.0f;
	float height = 0.0f;
};

// 三角錐シェイプセット用
struct PyramidShapeDesc
{
	SetupRigidBodyDesc rdDesc;
	std::array<VECTOR3::VEC3, 4> v4;
};

// 三角形シェイプセット用
struct TriangleShapeDesc
{
	SetupRigidBodyDesc rdDesc;
	std::array<VECTOR3::VEC3, 3> v3;
};

// 線シェイプセット用
struct LineShapeDesc
{
	SetupRigidBodyDesc rdDesc;
	std::array<VECTOR3::VEC3, 2> v2;
};

// 点シェイプセット用
struct PointShapeDesc
{
	SetupRigidBodyDesc rdDesc;
	VECTOR3::VEC3 v1 = VECTOR3::VEC3();
};

// 凸包シェイプセット用
struct ConvexHullShapeDesc
{
	SetupRigidBodyDesc rdDesc;
	const float* points;
	int numPoints;
	int stride = sizeof(VECTOR3::VEC3);
};
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
	std::unique_ptr<class btDefaultCollisionConfiguration> m_pConfig;
	std::unique_ptr<class btCollisionDispatcher> m_pDispatcher;
	std::unique_ptr<class btBroadphaseInterface> m_pBroadphase;
	std::unique_ptr<class btSequentialImpulseConstraintSolver> m_pSolver;
	std::unique_ptr<class btDiscreteDynamicsWorld> m_pWorld;

	std::vector<class  btRigidBody*>m_RBPtrs;	// リジッドボディのポインタを保持

public:
	PhysicsEngine();
	~PhysicsEngine();

	bool Setup();
	void Update(float deltaTime);
	bool Shutdown();

	void CreateRigidBody(class btCollisionShape* pShape, const VECTOR3::VEC3& pos, float mass);

	void RegisterShape(const BoxShapeDesc& desc);
	void RegisterShape(const SphereShapeDesc& desc);
	void RegisterShape(const CapsuleShapeDesc& desc);
	void RegisterShape(const CylinderShapeDesc& desc);
	void RegisterShape(const ConeShapeDesc& desc);
	void RegisterShape(const PyramidShapeDesc& desc);
	void RegisterShape(const TriangleShapeDesc& desc);
	void RegisterShape(const LineShapeDesc& desc);
	void RegisterShape(const PointShapeDesc& desc);
	void RegisterShape(const ConvexHullShapeDesc& desc);

private:
	class btBoxShape* CreateShapeBox(const VECTOR3::VEC3& boxHalfExtents);
	class btSphereShape* CreateShapeSphere(float radius);
	class btCapsuleShape* CreateShapeCapsule(float radius, float height);
	class btCylinderShape* CreateShapeCylinder(const VECTOR3::VEC3& halfExtents);
	class btConeShape* CreateShapeCone(float radius, float height);
	class btBU_Simplex1to4* CreateShapePyramid(const std::array<VECTOR3::VEC3, 4> v4);
	class btBU_Simplex1to4* CreateShapeTriangle(const std::array<VECTOR3::VEC3, 3> v3);
	class btBU_Simplex1to4* CreateShapeLine(const std::array<VECTOR3::VEC3, 2> v2);
	class btBU_Simplex1to4* CreateShapePoint(const VECTOR3::VEC3& v1);
	class btConvexHullShape* CreateShapeConvexHull(const float* points, int numPoints, int stride = sizeof(VECTOR3::VEC3));
	class btConvexTriangleMeshShape* CreateShapeConvexTriangleMesh();
	class btBvhTriangleMeshShape* CreateShapeBvhTriangleMesh();
};

