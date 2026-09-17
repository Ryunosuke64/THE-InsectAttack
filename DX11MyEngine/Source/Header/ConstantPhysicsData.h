#pragma once

namespace PhysicsData
{
	// PhysicsEngine内のスロットを識別する。
	// generationで削除・再利用後の古い参照を検出する。
	struct PhysicsBodyHandle
	{
		uint32_t index = UINT32_MAX;
		uint32_t generation = 0;
	};

	enum class BodyType
	{
		Static,
		Dynamic,
		Kinematic
	};

	struct RigidBodyDesc
	{
		BodyType type = BodyType::Dynamic;
		float mass = 1.0f;
		float gravityScale = 1.0f; // ワールド重力に対する倍率
		float friction = 0.5f;
		float restitution = 0.0f;
	};
};