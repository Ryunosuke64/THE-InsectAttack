#include "pch.h"
#include "ConstantPhysicsData.h"
#include "Component_Collider.h"


bool PhysicsData::MyCollisionDispatcher::
needsResponse(const btCollisionObject* body0, const btCollisionObject* body1)
{
	// Bullet標準で物理応答しない組み合わせならfalse
	if (!btCollisionDispatcher::needsResponse(body0, body1))
	{
		return false;
	}

	// ユーザー設定ポインタ取り出し
	PhysicsUserData* userDataA = static_cast<PhysicsUserData*>(body0->getUserPointer());
	PhysicsUserData* userDataB = static_cast<PhysicsUserData*>(body1->getUserPointer());

	// UserDataが設定されていない場合は
	// Bullet標準の衝突判定を許可
	if (!userDataA || !userDataB)
	{
		return true;
	}

	auto colliderA = userDataA->collider.lock();
	auto colliderB = userDataB->collider.lock();

	if (!colliderA || colliderB)
	{
		return true;
	}

	UtilityData::COLLISION_RESPONSE responseA = colliderA->get_Response(colliderB->get_CollisionCategory());
	UtilityData::COLLISION_RESPONSE responseB = colliderB->get_Response(colliderA->get_CollisionCategory());

	// どちらか一方でもOVERLAPなら判定自体をしない
	if (responseA == UtilityData::COLLISION_RESPONSE::RESPONSE_OVERLAP ||
		responseB == UtilityData::COLLISION_RESPONSE::RESPONSE_OVERLAP)
	{
		return false;
	}

	return true;
}


bool PhysicsData::MyCollisionDispatcher::
needsCollision(const btCollisionObject* body0, const btCollisionObject* body1)
{
	if (!btCollisionDispatcher::needsCollision(body0, body1))
		return false;

	// ユーザー設定ポインタ取り出し
	PhysicsUserData* userDataA = static_cast<PhysicsUserData*>(body0->getUserPointer());
	PhysicsUserData* userDataB = static_cast<PhysicsUserData*>(body1->getUserPointer());

	// UserDataが設定されていない場合は
	// Bullet標準の衝突判定を許可
	if (!userDataA || !userDataB)
	{
		return true;
	}

	auto colliderA = userDataA->collider.lock();
	auto colliderB = userDataB->collider.lock();

	if (!colliderA || colliderB)
	{
		return true;
	}

	UtilityData::COLLISION_RESPONSE responseA = colliderA->get_Response(colliderB->get_CollisionCategory());
	UtilityData::COLLISION_RESPONSE responseB = colliderB->get_Response(colliderA->get_CollisionCategory());

	// どちらか一方でもIGNOREなら判定自体をしない
	if (responseA == UtilityData::COLLISION_RESPONSE::RESPONSE_IGNORE ||
		responseB == UtilityData::COLLISION_RESPONSE::RESPONSE_IGNORE)
	{
		return false;
	}

	return true;
}