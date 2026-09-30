#include "pch.h"
#include "Component_MoveLogic.h"
#include "IMoveBehaviour.h"
#include "LinearMove_Behaviour.h"
#include "HormingMove_Behaviour.h"
#include "Component_RigidBody.h"
#include "GameObject.h"

using namespace GIGA_Engine;
using namespace VECTOR3;
using namespace UtilityData;

//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//* [引数]
//* pOwner : オーナーオブジェクト
//* updateRank : 更新レイヤー
//*----------------------------------------------------------------------------------------
MoveLogic::MoveLogic(std::weak_ptr<GameObject> pOwner, int updateRank)
    :IComponent(pOwner, updateRank),
    m_pMoveBehaviour(nullptr),
    m_GravityVelocity(0.0f),
    m_AccelerationSpeed(0.0f),
    m_CrntMoveVelocity(VEC3())
{
    this->set_Tag("MoveLogic");
}

//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
MoveLogic::~MoveLogic()
{
	m_pMoveBehaviour = nullptr;
}
//*---------------------------------------------------------------------------------------
//*【?】初期化
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//*
//* [返値] なし
//*----------------------------------------------------------------------------------------
void MoveLogic::Start(RendererEngine &renderer)
{
    // デフォルトは直線移動
    //ChangeBehaviour(MOVE_BEHAVIOUR_TYPE::LINEAR);
}

//*---------------------------------------------------------------------------------------
//*【?】更新
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//*
//* [返値] なし
//*----------------------------------------------------------------------------------------
void MoveLogic::Update(RendererEngine &renderer)
{
    Calculate(m_MoveParam);
}

//*---------------------------------------------------------------------------------------
//*【?】更新
//*
//* [引数]
//* &_param : 移動計算に必要なパラメータ
//*
//* [返値] なし
//*----------------------------------------------------------------------------------------
void MoveLogic::Calculate(const MoveParam& _param)
{
    float deltaTime = Master::m_pTimeManager->get_DeltaTime();
    // 移動挙動クラスがセットされていれば、移動計算をする
    if (auto pTransform = m_pOwner.lock()->get_Transform().lock())
    {
        if (m_pMoveBehaviour != nullptr)
        {
            ResultMove res;

            m_AccelerationSpeed += _param._acceleration * deltaTime;

            // 加速度を上書きしないよう、
            MoveParam effectiveParam = _param;
            effectiveParam._acceleration = m_AccelerationSpeed;

            // 移動計算を呼び出す
            res = m_pMoveBehaviour->MoveCalculate(deltaTime, effectiveParam, *pTransform);

            //****************************************************
            // 物理移動をするか（RIgidBodyで移動させるか）
            if (_param._isPhysicsMove)
            {
                PhysicsMovement(res, *pTransform, deltaTime);
            }
            else
            {
                NormalMovement(res, *pTransform, deltaTime);
            }
        }
    }
}

//*---------------------------------------------------------------------------------------
//*【?】物理移動
//*     RigidBodyで移動する 
//*
//* [引数] 
//* &_param : 移動計算に必要なパラメータ
//* [返値] なし
//*----------------------------------------------------------------------------------------
void MoveLogic::PhysicsMovement(const ResultMove& _param, const MyTransform& _transform, float deltaTime)
{
    auto rigidBody = m_pOwner.lock()->get_Component<RigidBody>();

    if (!rigidBody)
    {
        return;
    }
    // 地上の敵：Y方向の速度は重力・落下用に維持する
    auto velocity = rigidBody->GetLinearVelocity();
    velocity.x = _param._moveVelocity.x;
    velocity.z = _param._moveVelocity.z;
    rigidBody->SetLinearVelocity(velocity);

    DirectX::XMFLOAT4 rotation;
    DirectX::XMStoreFloat4(
        &rotation,
        DirectX::XMQuaternionNormalize(_param._RotQ));

    // 回転設定
    rigidBody->SetRotation(VECTOR4::VEC4(
        rotation.x,
        rotation.y,
        rotation.z,
        rotation.w
    ));
}

//*---------------------------------------------------------------------------------------
//*【?】通常移動
//*
//* [引数]
//* &_param : 移動計算に必要なパラメータ
//* [返値] なし
//*----------------------------------------------------------------------------------------
void MoveLogic::NormalMovement(const ResultMove& _param,  MyTransform& _transform, float deltaTime)
{
    // 加速度
    //float accelerationSpeed = _param._acceleration * deltaTime;

    // 目標速度に近づける
    //m_CrntMoveVelocity = VEC3::Lerp(m_CrntMoveVelocity, res._moveVelocity, accelerationSpeed);

    // 移動ベクトルと回転ベクトルをもとに、新しい位置と回転を計算する
    VEC3 crntPos = _transform.get_VEC3ToPos();
    VEC3 newPos = crntPos + (_param._moveVelocity * deltaTime);

    // 重力があるなら、重力処理を行う
    //if (_param._gravity > 0.0f)
    //{
    //    m_GravityVelocity -= _param._gravity * deltaTime;

    //    newPos.y += m_GravityVelocity * deltaTime;
    //    if (newPos.y < -100.0f)
    //    {
    //        newPos.y = 0.0f;
    //        m_GravityVelocity = 0.0f;
    //    }
    //}
    // 反映
    _transform.set_Pos(newPos);
    _transform.set_RotationQuaternion(_param._RotQ);
}



//*---------------------------------------------------------------------------------------
//*【?】パラメータのリセット
//*
//* [引数] なし
//* [返値] なし
//*----------------------------------------------------------------------------------------
void MoveLogic::ParamReset()
{
    m_GravityVelocity = 0.0f;
    m_AccelerationSpeed = 0.0f;
    m_CrntMoveVelocity = VEC3();
    m_MoveParam = MoveParam();
}


//*---------------------------------------------------------------------------------------
//*【?】移動挙動の登録
//*
//* [引数]
//* _type : 移動挙動の種類
//* 
//* [返値] なし
//*----------------------------------------------------------------------------------------
void MoveLogic::Register(MOVE_BEHAVIOUR_TYPE _type)
{
    int index = static_cast<int>(_type);

    switch (_type)
    {
    case MOVE_BEHAVIOUR_TYPE::LINEAR:
        m_pMoveBehaviourMap[index] = std::make_unique<LinearMove_Behaviour>();
        break;
    case MOVE_BEHAVIOUR_TYPE::HOMING:
        m_pMoveBehaviourMap[index] = std::make_unique<HormingMove_Behaviour>();
        break;
    default:
        break;
    }
}

//*---------------------------------------------------------------------------------------
//*【?】移動挙動の変更
//*
//* [引数]
//* _type : 変更する移動挙動の種類（事前に登録されていなかった場合、登録処理を行うため、newが走る）
//* 
//* [返値] なし
//*----------------------------------------------------------------------------------------
void MoveLogic::ChangeBehaviour(MOVE_BEHAVIOUR_TYPE _type)
{
	// NONEが指定された場合は、移動挙動を解除する
    if (_type == MOVE_BEHAVIOUR_TYPE::NONE)
    {
        m_pMoveBehaviour = nullptr;
		return;
    }

    // もし登録されていなければ登録する
    int index = static_cast<int>(_type);
    if (m_pMoveBehaviourMap[index] == nullptr)
    {
        Register(_type);
    }

    m_pMoveBehaviour = m_pMoveBehaviourMap[index].get();
}
