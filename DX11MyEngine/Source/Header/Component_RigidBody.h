#pragma once
#include "IComponent.h"

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
    VECTOR3::VEC3 m_Velocity;
    VECTOR3::VEC3 m_ForceAccumulator; // 1フレームに蓄積された力
    float m_Mass;			// 重量
    float m_GravityScale;   // 重力の強さ
    float m_MaxSpeed;       // 速度の上限
    float m_Restitution;    // 反発係数(0.0f：跳ねない)
    float m_MoveDrag;	    // 移動の減衰（0.0～1.0、1.0なら減衰なし、0.0なら完全に止まる）
    float m_AirDrag;	    // 空中抵抗の減衰（0.0～1.0、1.0なら減衰なし、0.0なら完全に止まる）
    bool m_IsEnable;
    class btRigidBody *m_pRigidBodyBT;

public:
    RigidBody(std::weak_ptr<GameObject> pOwner, int updateRank = 100);
    ~RigidBody();

    void Start(RendererEngine& renderer) override;		// 初期化
    void Update(RendererEngine& renderer) override;
};

