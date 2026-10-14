#include "pch.h"
#include "DestructionBehavior_Header.h"
#include "Component_RigidBody.h"

//*---------------------------------------------------------------------------------------
//*【?】開始
//*
//* [引数]
//* *object  :
//* &hitPos  :
//* &hitDir  :
//*
//* [返値]
//* なし
//*----------------------------------------------------------------------------------------
void PropDestructionBehavior::Start(
    GameObject* object,
    const VECTOR3::VEC3& hitPos,
    const VECTOR3::VEC3& hitDir)
{
    auto rigidBody =
        object->get_Component<RigidBody>();

    if (rigidBody)
    {
        rigidBody->SetEnabled(true);    // 物理挙動ON
        rigidBody->SetMass(1.0f);       // 質量設定

        // 吹っ飛ばす
        rigidBody->AddImpulse(          
            hitDir * IMPULSE_POWER);
    }

    // 破壊開始
    m_IsDestructionStart = true;
}    


//*---------------------------------------------------------------------------------------
//*【?】更新
//*
//* [引数]
//* deltaTime : デルタタイム
//*
//* [返値]
//* なし
//*----------------------------------------------------------------------------------------
void PropDestructionBehavior::Update(float deltaTime)
{
    if (!m_IsDestructionStart)
    {
        return;
    }

    m_DestroyTimer += deltaTime;

    // デストロイタイムを過ぎたら、終了
    if (m_DestroyTimer >= DESTROY_TIME)
    {
        m_IsFinished = true;
    }
}

//*---------------------------------------------------------------------------------------
//*【?】破壊が終了したか
//*
//* [引数] 
//* なし 
//* [返値]
//* 終了したか
//*----------------------------------------------------------------------------------------
bool PropDestructionBehavior::IsFinished() const
{
    return m_IsFinished;
}
