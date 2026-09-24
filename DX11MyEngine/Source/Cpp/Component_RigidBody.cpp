#include "pch.h"
#include "Component_RigidBody.h"
#include "Component_Collider.h"
#include "PhysicsEngine.h"

using namespace GIGA_Engine;
using namespace VECTOR3;
using namespace VECTOR4;
using namespace UtilityData;
using namespace PhysicsData;

//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//* [引数]
//* pOwner : オーナーオブジェクト
//* updateRank : 更新レイヤー
//*----------------------------------------------------------------------------------------
RigidBody::RigidBody(std::weak_ptr<GameObject> pOwner, int updateRank)
    :IComponent(pOwner, updateRank)
{
    this->set_Tag("RigidBody");
}

//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
RigidBody::~RigidBody()
{
    Release();
}

//*---------------------------------------------------------------------------------------
//*【?】更新
//*
//* [引数]
//* &renderer : 描画エンジン
//* [返値]なし
//*----------------------------------------------------------------------------------------
void RigidBody::LateUpdate(RendererEngine& renderer)
{
    auto transform = m_pOwner.lock()->get_Transform().lock();
    const auto pos = m_pEngine->GetWorldPosition(m_Handle);
    const auto rot = m_pEngine->GetRotation(m_Handle);
    const auto quaternion = DirectX::XMVectorSet(
        rot.x, rot.y, rot.z, rot.w
    );

    transform->set_Pos(pos);
    transform->set_RotationQuaternion(DirectX::XMQuaternionNormalize(quaternion));
}

//*---------------------------------------------------------------------------------------
//*【?】セットアップ
//*
//* [引数]
//* &engine : 物理エンジン
//* &desc   : 剛体セットアップデータ
//* [返値]なし
//*----------------------------------------------------------------------------------------
bool RigidBody::Setup(PhysicsEngine& engine, const RigidBodyDesc& desc)
{
    m_pEngine = &engine;
    m_Desc = desc;


    // エンジンに登録し、ハンドルを受け取る
    m_Handle = m_pEngine->CreateRigidBody(desc);


    auto owner = m_pOwner.lock();
    auto collider = desc.collider.lock();

    if (!owner || !collider) {
        return false;
    }

    auto self = owner->get_Component<RigidBody>();
    if (!self || self.get() != this) {
        return false;
    }

    // shared_ptr<RigidBody> → weak_ptr<RigidBody> に自動変換
    collider->set_RigidBody(self);

    // ユーザーデータとして保持
    m_UserData.collider = collider;
    m_UserData.gameoOject = owner;

    // ポインタを設定
    m_pEngine->SetUserPointer(m_Handle, &m_UserData);

    return true;
}


//*---------------------------------------------------------------------------------------
//*【?】解放
//*
//* [引数] なし
//* [返値] なし
//*----------------------------------------------------------------------------------------
void RigidBody::Release()
{
    if (m_pEngine != nullptr)
    {
        // エンジンから登録解除
        m_pEngine->UnregisterRigidBody(m_Handle);
        
        m_pEngine = nullptr;
    }
}

//*---------------------------------------------------------------------------------------
//*【?】マスクを設定しなおす
//*
//* [引数] なし
//* [返値] なし
//*----------------------------------------------------------------------------------------
void RigidBody::RefreshCollisionFilter()
{
    if (auto collider = m_Desc.collider.lock())
    {
        unsigned mask = collider->get_CollisionBitMask();
        unsigned group = UINT_CAST(collider->get_CollisionCategory());
        m_pEngine->SetMask(m_Handle, group, mask);
    }
}

//*---------------------------------------------------------------------------------------
//*【?】継続的な力を加える
//*
//* [引数]
//* &_force : 力ベクトル
//* 
//* [返値] なし
//*----------------------------------------------------------------------------------------
void RigidBody::AddForce(const VECTOR3::VEC3& force)
{
    m_pEngine->AddForce(m_Handle, force, VEC3());
}

//*---------------------------------------------------------------------------------------
//*【?】瞬間的な衝撃を加える
//*
//* [引数]
//* &_impulse : 衝撃ベクトル
//* 
//* [返値] なし
//*----------------------------------------------------------------------------------------
void RigidBody::AddImpulse(const VECTOR3::VEC3& impulse)
{
    m_pEngine->AddImpulse(m_Handle, impulse, VEC3());
}

//*---------------------------------------------------------------------------------------
//*【?】線形速度を設定
//*
//* [引数]
//* &velocity : 速度
//* 
//* [返値]なし
//*----------------------------------------------------------------------------------------
void RigidBody::SetLinearVelocity(const VECTOR3::VEC3& velocity)
{
    m_pEngine->SetLinearVelocity(m_Handle, velocity);
}

//*---------------------------------------------------------------------------------------
//*【?】重心に継続的な力を加える
//*     回転なし
//*
//* [引数]
//* &_force : 力ベクトル
//* 
//* [返値]なし
//*----------------------------------------------------------------------------------------
void RigidBody::AddCentralForce(const VECTOR3::VEC3& force)
{
    m_pEngine->AddCentralForce(m_Handle, force);
}

//*---------------------------------------------------------------------------------------
//*【?】瞬間的な衝撃を加える
//*     回転なし
//*
//* [引数]
//* &_impulse : 衝撃ベクトル
//* 
//* [返値]なし
//*----------------------------------------------------------------------------------------
void RigidBody::AddCentralImpulse(const VECTOR3::VEC3& impulse)
{
    m_pEngine->AddCentralImpulse(m_Handle, impulse);
}

//*---------------------------------------------------------------------------------------
//*【?】直接位置を設定する
//*     物理的な挙動は考慮しない 
//*
//* [引数]
//* &position : 位置
//* 
//* [返値]なし
//*----------------------------------------------------------------------------------------
void RigidBody::Teleport(const VECTOR3::VEC3& position)
{
    VEC4 rot = m_pEngine->GetRotation(m_Handle);
    m_pEngine->SetWorldTransform(m_Handle, position, rot);
}

//*---------------------------------------------------------------------------------------
//*【?】直接位置を設定する
//*     物理的な挙動は考慮しない 
//*
//* [引数]
//* &position : 位置
//* 
//* [返値]なし
//*----------------------------------------------------------------------------------------
void RigidBody::SetWorldTransform(const VECTOR3::VEC3& position, const VECTOR4::VEC4& rotation)
{
    m_pEngine->SetWorldTransform(m_Handle, position, rotation);
}

//*---------------------------------------------------------------------------------------
//*【?】ワールド座標の取得
//*----------------------------------------------------------------------------------------
VECTOR3::VEC3 RigidBody::GetWorldPotision()const
{
    return m_pEngine->GetWorldPosition(m_Handle);
}

//*---------------------------------------------------------------------------------------
//*【?】回転の取得
//*----------------------------------------------------------------------------------------
VECTOR4::VEC4 RigidBody::GetRotation()const
{
    return m_pEngine->GetRotation(m_Handle);
}

//*---------------------------------------------------------------------------------------
//*【?】重量の設定
//*----------------------------------------------------------------------------------------
void RigidBody::SetMass(float mass)
{
    m_pEngine->SetMass(m_Handle, mass);
}

//*---------------------------------------------------------------------------------------
//*【?】重力の設定の設定
//*----------------------------------------------------------------------------------------
void RigidBody::SetGravity(const VECTOR3::VEC3& gravity)
{
    m_pEngine->SetGrivity(m_Handle, gravity);
}
