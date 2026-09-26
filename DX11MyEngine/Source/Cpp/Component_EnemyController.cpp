#include "pch.h"
#include "Component_EnemyController.h"
#include "GameObject.h"
#include "Component_SkinnedMeshAnimator.h"
#include "Component_DecalRenderer.h"
#include "Component_MoveLogic.h"
#include "Component_TimerDestruction.h"
#include "Component_Collider.h"
#include "Component_BoxCollider.h"
#include "Component_Health.h"
#include "Component_Physics.h"
#include "RendererEngine.h"
#include "CollisionInfo.h"
#include "MeshFactory.h"
#include "ResourceManager.h"


using namespace GIGA_Engine;
using namespace VECTOR3;
using namespace VECTOR2;
using namespace EnemyData;
using namespace UtilityData;

//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//* [引数]
//* pOwner : オーナーオブジェクト
//* updateRank : 更新レイヤー
//*----------------------------------------------------------------------------------------
EnemyController::EnemyController(std::weak_ptr<GameObject> pOwner, int updateRank)
    :IComponent(pOwner, updateRank),
	m_StateMachine(this),
	m_pHealthComp(nullptr),
	m_pAnimatorComp(nullptr),
	m_pColliderComp(nullptr),
	m_pMoveLogicComp(nullptr),
	m_pPhysicsComp(nullptr),
	m_pTransformComp(nullptr),
	m_pTarget(nullptr),
	m_IsAnim(false),
	m_IsGrounded(false),
	m_IsOnDamage(false),
	m_CrntAnimID(-1),
	m_MoveSpeed(0.0f),
	m_MoveVelocity(VECTOR3::VEC3()),
	m_StateTimer(0),
	m_GravityVelocity(0.0f),
	m_pEnemyData(nullptr),
	m_Gravity(18.0f),
	m_AnimSpeed(1.25f),
	m_MyGroupID(-1),
	m_MyID(-1),
	m_StaggerInfo()
{
    this->set_Tag("EnemyController");
}

//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
EnemyController::~EnemyController()
{

}

//*---------------------------------------------------------------------------------------
//*【?】初期化
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void EnemyController::Start(RendererEngine& renderer)
{
	auto ownerObj = m_pOwner.lock();;

    // アニメーションコンポーネントの取得
    m_pAnimatorComp = ownerObj->get_Component<SkinnedMeshAnimator>().get();

	// コライダーの取得
	m_pColliderComp = ownerObj->get_Component<BoxCollider>().get();

	// 移動コンポーネントの取得
	m_pMoveLogicComp = ownerObj->get_Component<MoveLogic>().get();

	// 物理コンポーネントの取得
	m_pPhysicsComp = ownerObj->get_Component<Physics>().get();

	// HP管理コンポーネントの取得
	m_pHealthComp = m_pOwner.lock()->get_Component<Health>().get();
	
	// トランスフォームコンポーネントの取得
	m_pTransformComp = ownerObj->get_Transform().lock().get();

	// *********************************************************
	// ステート内で処理を行うようにしたので要らないかも？
	// *********************************************************
	// TODO:処理関数を外から入れるようにする
	// 被弾時の処理登録
	m_pHealthComp->RegisterOnDamage(
		[this, &renderer](float _damage)
		{
			m_IsOnDamage = true;
			m_StaggerInfo._cumulativeValue += _damage;	// ダメージ蓄積
		}
	);
	// 死亡時の処理登録
	m_pHealthComp->RegisterOnDead(
		[this, &renderer]
		{
			// マネージャーから除外する
			Master::m_pEnemyManager->UnregisterEnemy(m_MyID);
		}
	);

	// 開始時の座標を入れる
	m_StartPos = m_pTransformComp->get_VEC3ToPos();

	m_pMoveLogicComp->Register(MOVE_BEHAVIOUR_TYPE::HOMING);
	m_pMoveLogicComp->Register(MOVE_BEHAVIOUR_TYPE::LINEAR);
	m_pMoveLogicComp->ChangeBehaviour(MOVE_BEHAVIOUR_TYPE::LINEAR);
}


//*---------------------------------------------------------------------------------------
//*【?】更新
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void EnemyController::Update(RendererEngine& renderer)
{
	m_IsGrounded = false;
}


//*---------------------------------------------------------------------------------------
//*【?】遅延更新
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void EnemyController::LateUpdate(RendererEngine& renderer)
{
	float deltaTime = Master::m_pTimeManager->get_DeltaTime();


	auto target = Master::m_pGameObjectManager->get_ObjectByTagConst("Player");
	m_pTarget = target;

	m_StateTimer += deltaTime;	// タイマー進める
	
	// ステートの実行
	m_StateMachine.Update();

	m_IsOnDamage = false;	//　ダメージフラグを初期化

	if (m_pAnimatorComp != nullptr)
	{
		m_pAnimatorComp->set_IsAnim(m_IsAnim);
		m_pAnimatorComp->PlayAnim(Master::m_pTimeManager->get_DeltaTime() * m_AnimSpeed);
	}

	auto transform = m_pOwner.lock()->get_Transform().lock();
	VEC3 newPos = transform->get_VEC3ToPos();

	VEC3 strPos = newPos;
	strPos.y += 5.0f;
	std::string stateName = EnemyData::g_AntStateNames[m_StateMachine.get_CrntStateIndex()];

	//Master::m_pDirectWriteManager->DrawString3D(stateName, strPos, "White_20_STD");

	// 世界の裏側に落下した場合
	if (newPos.y < -100.0f)
	{
		m_GravityVelocity = 0.0f;
		transform->set_Pos(VEC3(0.0f, 100.0f, 0.0f));
		m_pPhysicsComp->SetZeroVelocity();
	}
}


//*---------------------------------------------------------------------------------------
//*【?】描画
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//* [返値]なし
//*----------------------------------------------------------------------------------------
void EnemyController::Draw(RendererEngine &renderer)
{
	// ステートの描画
	m_StateMachine.Draw();
}

//*---------------------------------------------------------------------------------------
//*【?】衝突処理
//*
//* [引数]
//* & _other : 衝突相手の情報
//* [返値]なし
//*----------------------------------------------------------------------------------------
void EnemyController::OnCollisionEnter(const PhysicsData::CollisionInfo& _other)
{
	VEC3 normal = _other.hitNormal;

	// 法線のY成分が一定以上なら床とみなす
	if (normal.y < -0.7f)
	{
		// めり込み防止のため、下方向への速度をリセット
		if (m_GravityVelocity < 0.0f) {
			m_GravityVelocity = 0.0f;
		}
		m_IsGrounded = true;
	}
}

//*---------------------------------------------------------------------------------------
//*【?】ステートの変更
//*
//* [引数]
//*  _state : ステート番号
//* [返値]なし
//*----------------------------------------------------------------------------------------
void EnemyController::ChangeState(const int _state)
{
	m_StateTimer = 0;	// ステート時間をリセット
	m_StateMachine.ChangeState(_state);
}

//*---------------------------------------------------------------------------------------
//*【?】アニメーションの変更
//*
//* [引数]
//*  _id : アニメーション番号
//* [返値]なし
//*----------------------------------------------------------------------------------------
void EnemyController::ChangeAnimation(const int _newId)
{
	// 同じまたは停止中なら返す
	if (_newId == m_CrntAnimID || m_IsAnim == false)
	{
		return;
	}

	// ひとつ前のアニメーションIDセット
	m_pAnimatorComp->set_PrevAnimIndex(static_cast<int>(m_CrntAnimID));

	m_CrntAnimID = _newId;

	// 現在のアニメーションIDセット
	m_pAnimatorComp->set_AnimIndex(static_cast<int>(_newId));
	m_pAnimatorComp->set_AnimTime(0.0f);
}

//*---------------------------------------------------------------------------------------
//*【?】ターゲットのトランスフォームを取得
//*
//* [引数]
//*  _id : アニメーション番号
//* [返値]なし
//*----------------------------------------------------------------------------------------
std::shared_ptr<MyTransform> EnemyController::get_TargetTransform() const
{
	if (m_pTarget)
	{
		return m_pTarget->get_Transform().lock();
	}

	return nullptr;
}

//*---------------------------------------------------------------------------------------
//*【?】死亡フラグの取得
//*
//* [引数]なし
//* [返値]なし
//*----------------------------------------------------------------------------------------
bool EnemyController::get_IsDead()const
{
	if (m_pHealthComp != nullptr){
		return m_pHealthComp->get_IsDead();
	}
	else{
		assert(false);
	}
	return false;
}


//*---------------------------------------------------------------------------------------
//*【?】受けたダメージ量の取得
//*
//* [引数]なし
//* [返値]
//* ダメージ量 
//*----------------------------------------------------------------------------------------
float EnemyController::get_DamageAmount()const
{
	if (m_pHealthComp != nullptr){
		return m_pHealthComp->get_DamageAmount();
	}
	else{
		assert(false);
	}
	return 0.0f;
}



//*---------------------------------------------------------------------------------------
//*【?】移動ロジックの切り替え
//*
//* [引数]
//*  _moveType : 移動タイプ
//* [返値]なし
//*----------------------------------------------------------------------------------------
void EnemyController::set_MoveLogicState(UtilityData::MOVE_BEHAVIOUR_TYPE _moveType)
{
	m_pMoveLogicComp->ChangeBehaviour(_moveType);
}

//*---------------------------------------------------------------------------------------
//*【?】パラメータ取得
//*
//* [引数] なし
//* [返値] 書き換え不可のエネミーパラメータ
//*----------------------------------------------------------------------------------------
const EnemyData::BaseEnemyData* EnemyController::get_EnemyData()const
{
	return static_cast<const EnemyData::BaseEnemyData*>(m_pEnemyData);
}
