#include "pch.h"
#include "Component_Physics.h"
#include "CollisionInfo.h"

using namespace GIGA_Engine;
using namespace VECTOR3;
using namespace UtilityData;

//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//* [引数]
//* pOwner : オーナーオブジェクト
//* updateRank : 更新レイヤー
//*----------------------------------------------------------------------------------------
Physics::Physics(std::weak_ptr<GameObject> pOwner, int updateRank)
    :IComponent(pOwner, updateRank),
    m_Velocity(VEC3()),
    m_ForceAccumulator(VEC3()),
    m_Mass(1.0f),
    m_GravityScale(9.8f),
    m_MaxSpeed(7.0f),
    m_Restitution(0.3f),
    m_MoveDrag(0.8f),
    m_AirDrag(0.95f),
    m_IsEnable(true),
    m_IsGrounded(false)
{
    this->set_Tag("Physics");
}

//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
Physics::~Physics()
{
}

//*---------------------------------------------------------------------------------------
//*【?】開始
//*
//* [引数]
//* &renderer : 描画エンジン
//* [返値]なし
//*----------------------------------------------------------------------------------------
void Physics::Start(RendererEngine& renderer)
{

}


//*---------------------------------------------------------------------------------------
//*【?】更新
//*
//* [引数]
//* &renderer : 描画エンジン
//* [返値]なし
//*----------------------------------------------------------------------------------------
void Physics::Update(RendererEngine& renderer)
{
    if (m_IsEnable == false) return;
    float deltaTime = Master::m_pTimeManager->get_DeltaTime();
    auto transform = m_pOwner.lock()->get_Transform().lock();
    VEC3 position = transform->get_VEC3ToPos();

    // 重力の適用
    VEC3 gravity = { 0.0f, -m_GravityScale * m_Mass, 0.0f };
    AddForce(gravity);

    // 加速度の計算 (a = F/m)
    VEC3 acceleration = m_ForceAccumulator / m_Mass;

    // 速度と位置の更新 (オイラー積分)
    m_Velocity += acceleration * deltaTime;
    
    // =========================================================
    // 空気抵抗（空中にいる間の水平ブレーキ）
    // =========================================================
    if (m_IsGrounded == false)
    {
        m_Velocity.x *= m_AirDrag;
        m_Velocity.z *= m_AirDrag;
    }
    
    VEC3 horizontalVel = VEC3(m_Velocity.x, 0.0f, m_Velocity.z); // 水平方向だけ取り出す
    
    // 現在のスピードが限界を超えていたら
    if (horizontalVel.Length() > m_MaxSpeed)
    {
        // 限界の長さに縮める（向きはそのままに、長さをmaxSpeedにする）
        horizontalVel = horizontalVel.Normalize() * m_MaxSpeed;
        m_Velocity.x = horizontalVel.x;
        m_Velocity.z = horizontalVel.z;
    }
    
    position += m_Velocity * deltaTime;

    transform->set_Pos(position);

    // 次のフレームのために力をリセット
    m_ForceAccumulator = { 0, 0, 0 };

    // === 回転の物理演算 ===
    if (m_AngularVelocity.LengthSq() > 0.001f)
    {
        // 現在の回転（クォータニオンとして保持している XMVECTOR）を取得
        DirectX::XMVECTOR crntRotQ = transform->get_RotationQuaternion();

        // このフレーム分の微小回転クォータニオンを作成
        // 角速度ベクトルに deltaTime をかけて、このフレームの回転量を計算
        VEC3 deltaRotation = m_AngularVelocity * deltaTime;

        // 微小回転の近似公式： q = (x*0.5, y*0.5, z*0.5, 1.0)
        DirectX::XMVECTOR dq = DirectX::XMVectorSet(
            deltaRotation.x * 0.5f,
            deltaRotation.y * 0.5f,
            deltaRotation.z * 0.5f,
            1.0f
        );

        // クォータニオンの掛け算（新しい回転 ＝ 微小回転 * 現在の回転）
        DirectX::XMVECTOR newRotQ = DirectX::XMQuaternionMultiply(dq, crntRotQ);

        // 誤差修正のために正規化（単位クォータニオンにする）
        newRotQ = DirectX::XMQuaternionNormalize(newRotQ);

        // トランスフォームに反映
        transform->set_RotationQuaternion(newRotQ);

        // 空気抵抗による回転の減衰
        m_AngularVelocity *= m_AngularDrag;
    }

    m_IsGrounded = false;
}

//*---------------------------------------------------------------------------------------
//*【?】衝突時の処理
//*
//* [引数]
//* &info : 衝突情報
//* [返値]なし
//*----------------------------------------------------------------------------------------
void Physics::OnCollisionEnter(const PhysicsData::CollisionInfo& info)
{
    VEC3 hitNorm = -info.hitNormal;

    // 現在の進行方向と、ぶつかった面の法線の内積
    float dot_VelocityAndNormal = VEC3::Dot(m_Velocity, hitNorm);

    // 内積がマイナス ＝ 面に向かって突っ込んでいる状態
    if (dot_VelocityAndNormal < 0.0f)
    {
        // 壁に向かって進もうとしている分の速度ベクトル
        VEC3 penetrationVelocity = hitNorm * dot_VelocityAndNormal;

        // 反発係数 (0.0fなら完全に滑る/壁に張り付く、0.3fなら少し跳ねる)
        //float restitution = 0.0f;

        // 公式： V_new = V - (1 + e) * V_p に基づいて速度を補正
        m_Velocity = m_Velocity - (penetrationVelocity * (1.0f + m_Restitution));

        // 摩擦処理: 床にぶつかった時だけ、XとZの速度を減衰させる
        // HitNormal.y が 0.5f より大きい（だいたい上を向いている面）なら床とみなす
        if (hitNorm.y > 0.5f)
        {
            m_IsGrounded = true;
            m_Velocity.x *= m_MoveDrag; // 摩擦係数
            m_Velocity.z *= m_MoveDrag;
        }
    }
}


//*---------------------------------------------------------------------------------------
//*【?】継続的な力を加える
//*
//* [引数]
//* &_force : 力ベクトル
//* [返値]なし
//*----------------------------------------------------------------------------------------
void Physics::AddForce(const VECTOR3::VEC3& _force)
{
    m_ForceAccumulator += _force;
}

//*---------------------------------------------------------------------------------------
//*【?】瞬間的な衝撃を加える
//*
//* [引数]
//* &_impulse : 衝撃ベクトル
//* [返値]なし
//*----------------------------------------------------------------------------------------
void Physics::AddImpulse(const VECTOR3::VEC3& _impulse)
{
    m_Velocity += _impulse / m_Mass;
}

//*---------------------------------------------------------------------------------------
//*【?】間的な回転力を加える
//*
//* [引数]
//* &_impulse : 衝撃ベクトル
//* [返値]なし
//*----------------------------------------------------------------------------------------
void Physics::AddAngularImpulse(const VEC3& _angularImpulse)
{
    // 本来は慣性モーメント（回転のしにくさ）で割りますが、
    // 簡易的にはそのまま角速度（Velocity）に加算してしまってOKです
    m_AngularVelocity += _angularImpulse;
}

//*---------------------------------------------------------------------------------------
//*【?】移動ベクトルをクリア
//*
//* [引数]なし
//* [返値]なし
//*----------------------------------------------------------------------------------------
void Physics::SetZeroVelocity()
{
    m_Velocity = 0.0f;
    m_AngularVelocity = 0.0f;
}
