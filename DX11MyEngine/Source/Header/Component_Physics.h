#pragma once
#include "IComponent.h"

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:Physics Class --- */
//
//  ★継承：Component ★
//
// 【?】物理コンポーネント
//		リジッドボディ的な役割     
// 
// ***************************************************************************************
class Physics : public IComponent
{
private:
    friend class PhysicsEditor;

    VECTOR3::VEC3 m_Velocity;
    VECTOR3::VEC3 m_ForceAccumulator; // 1フレームに蓄積された力
	float m_Mass;			// 重量
    float m_GravityScale;   // 重力の強さ
	float m_MaxSpeed;       // 速度の上限
    float m_Restitution;    // 反発係数(0.0f：跳ねない)
	float m_MoveDrag;	    // 移動の減衰（0.0～1.0、1.0なら減衰なし、0.0なら完全に止まる）
	float m_AirDrag;	    // 空中抵抗の減衰（0.0～1.0、1.0なら減衰なし、0.0なら完全に止まる）
    bool m_IsEnable;    
    bool m_IsGrounded;

    // --- 回転用の物理パラメータ ---
    VECTOR3::VEC3 m_AngularVelocity = { 0.0f, 0.0f, 0.0f };      // 角速度（1秒間にどの軸でどれくらい回転するか）
    float m_AngularDrag = 0.95f;                        // 空気抵抗による回転の減衰（0.0～1.0）

public:
    Physics(std::weak_ptr<GameObject> pOwner, int updateRank = 100);
    ~Physics();

    void Start(RendererEngine& renderer) override;		// 初期化
    void Update(RendererEngine& renderer) override;
    void OnCollisionEnter(const PhysicsData::CollisionInfo& info) override;

    /// <summary> 力を加える </summary>
    void AddForce(const VECTOR3::VEC3& _force);
    /// <summary> 衝撃を加える </summary>
    void AddImpulse(const VECTOR3::VEC3& _impulse);
    /// <summary> 瞬間的な回転力を加える </summary>
    void AddAngularImpulse(const VECTOR3::VEC3& angularImpulse);
    /// <summary> 移動ベクトルをクリア </summary>
    void SetZeroVelocity();

    const bool get_IsEnable()const { return m_IsEnable; }
    const bool get_IsGrounded()const { return m_IsGrounded; }

    const VECTOR3::VEC3& get_Velocity()const { return m_Velocity; }
    const VECTOR3::VEC3& get_ForceAccumulator()const { return m_ForceAccumulator; }
    const float get_Mass()const { return m_Mass; }
    const float get_GravityScale()const { return m_GravityScale; }
	const float get_MaxSpeed()const { return m_MaxSpeed; }
	const float get_Restitution()const { return m_Restitution; }
	const float get_MoveDrag()const { return m_MoveDrag; }
	const float get_AirDrag()const { return m_AirDrag; }
	const float get_AngularDrag()const { return m_AngularDrag; }



    /// <summary> 使用フラグ </summary>
    void set_IsEnable(bool _flag) { m_IsEnable = _flag; }
    /// <summary> 重量 </summary>
    void set_Mass(float _mass) { m_Mass = _mass; }
    /// <summary> 重力の強さ </summary>
    void set_GravityScale(float _g) { m_GravityScale = _g; }
    /// <summary> 速度の上限 </summary>
    void set_MaxSpeed(float _max) { m_MaxSpeed = _max; }
    /// <summary> 反発係数(0.0f：跳ねない) </summary>
    void set_Restitution(float _restitution) { m_Restitution = _restitution; }
    /// <summary> 移動の減衰（0.0～1.0、1.0なら減衰なし、0.0なら完全に止まる） </summary>
    void set_MoveDrag(float _drag) { m_MoveDrag = _drag; }
    /// <summary> 空中抵抗の減衰（0.0～1.0、1.0なら減衰なし、0.0なら完全に止まる） </summary>
    void set_AirDrag(float _drag) { m_AirDrag = _drag; }
    /// <summary> 回転時の抵抗の減衰（0.0～1.0、1.0なら減衰なし、0.0なら完全に止まる） </summary>
	void set_AngularDrag(float _drag) { m_AngularDrag = _drag; }
};

