#pragma once
#include <BulletDynamics\Dynamics\btRigidBody.h>
namespace PhysicsData
{
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

	// ボックスシェイプセット用
	struct BoxShapeDesc
	{
		VECTOR3::VEC3 boxHalfExtents = VECTOR3::VEC3();
	};

	// 球シェイプセット用
	struct SphereShapeDesc
	{
		float radius = 0.0f;
	};

	// カプセルシェイプセット用
	struct CapsuleShapeDesc
	{
		float radius = 0.0f;
		float height = 0.0f;
	};

	// 円柱シェイプセット用
	struct CylinderShapeDesc
	{
		VECTOR3::VEC3 halfExtents = VECTOR3::VEC3();
	};

	// 円錐シェイプセット用
	struct ConeShapeDesc
	{
		float radius = 0.0f;
		float height = 0.0f;
	};

	// 三角錐シェイプセット用
	struct PyramidShapeDesc
	{
		std::array<VECTOR3::VEC3, 4> v4;
	};

	// 三角形シェイプセット用
	struct TriangleShapeDesc
	{
		std::array<VECTOR3::VEC3, 3> v3;
	};

	// 線シェイプセット用
	struct LineShapeDesc
	{
		std::array<VECTOR3::VEC3, 2> v2;
	};

	// 点シェイプセット用
	struct PointShapeDesc
	{
		VECTOR3::VEC3 v1 = VECTOR3::VEC3();
	};

	// 凸包シェイプセット用
	struct ConvexHullShapeDesc
	{
		const float* points;
		int numPoints;
		int stride = sizeof(VECTOR3::VEC3);
	};

	// シェイプのセットアップvariant
	using PhysicsShapeDesc = std::variant<
		BoxShapeDesc,
		SphereShapeDesc,
		CapsuleShapeDesc,
		CylinderShapeDesc,
		ConeShapeDesc,
		PyramidShapeDesc,
		TriangleShapeDesc,
		LineShapeDesc,
		PointShapeDesc,
		ConvexHullShapeDesc
	>;


	// PhysicsEngine内のスロットを識別する。
	// generationで削除・再利用後の古い参照を検出する。
	struct PhysicsBodyHandle
	{
		uint32_t index = UINT32_MAX;
		uint32_t generation = 0;
	};

	// 剛体スロット
	struct RigidBodySlot
	{
		 btRigidBody* rigidBody = nullptr;

		uint32_t generation = 0;
		bool active = false;
	};
	enum class BodyType
	{
		Static,
		Dynamic,
		Kinematic
	};

	/// <summary>
	/// リジッドボディのセットアップデータ
	/// </summary>
	struct RigidBodyDesc
	{
		BodyType type = BodyType::Dynamic;
		float mass = 0.0f;
		float friction = 0.5f;
		float restitution = 0.0f;
		VECTOR3::VEC3 gravityScale = VECTOR3::VEC3(0.0f, -9.8f, 0.0f); // ワールド重力に対する倍率
		VECTOR3::VEC3 pos = VECTOR3::VEC3();

		PhysicsShapeDesc shapeDesc; // シェイプセットアップ用
	};


};