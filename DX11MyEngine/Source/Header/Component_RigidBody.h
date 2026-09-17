#pragma once
#include "IComponent.h"
#include "ConstantPhysicsData.h"

class PhysicsEngine;

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:RigidBody Class --- */
//
//  ★継承：Component ★
//
// 【?】剛体コンポーネント
//		BulletPhysicsを使用した、剛体処理を行う
// 
// ***************************************************************************************
class RigidBody : public IComponent
{
private:
    PhysicsEngine* m_pEngine = nullptr; // 非所有
    PhysicsData::PhysicsBodyHandle m_Handle;
    PhysicsData::RigidBodyDesc m_Desc;

public:
    RigidBody(std::weak_ptr<GameObject> pOwner, int updateRank = 100);
    ~RigidBody();

    // Colliderなどの構築完了後に呼ぶ。
    bool Setup(PhysicsEngine& engine, const PhysicsData::RigidBodyDesc& desc);
    void Release();

    void AddForce(const VECTOR3::VEC3& force);
    void AddImpulse(const VECTOR3::VEC3& impulse);

    VECTOR3::VEC3 GetLinearVelocity() const;
    void SetLinearVelocity(const VECTOR3::VEC3& velocity);

    void SetMass(float mass);
    void SetGravityScale(float scale);

    void SetEnabled(bool enabled);
    void Teleport(const VECTOR3::VEC3& position);

    // Kinematic用。瞬間移動とは区別する。
    void MovePosition(const VECTOR3::VEC3& position);
};

