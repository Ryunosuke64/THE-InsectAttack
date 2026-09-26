#include "pch.h"
#include "GameObject.h"
#include "CollisionInfo.h"
#include "RendererEngine.h"

//*---------------------------------------------------------------------------------------
//* @:GameObject3D Class 
//*【?】コンストラクタ
//* 引数：なし
//*----------------------------------------------------------------------------------------
GameObject::GameObject():
	m_IsCalcUpdate(false),
	//m_IsShadow(false),
	m_IsStatic(true),
	m_IsUpdateAllowedDuringPause(true),
	m_IsDrawAllowedDuringPause(true)
{

}

//*---------------------------------------------------------------------------------------
//* @:GameObject3D Class 
//*【?】デストラクタ
//* 引数：なし
//*----------------------------------------------------------------------------------------
GameObject::~GameObject()
{
	m_pComponentMap.clear();	// 解放
	m_pTransform.reset();
}


//*---------------------------------------------------------------------------------------
//* @:Object Class 
//*【?】コンポーネントの更新
//* 引数：描画エンジンの参照
//* 戻値：void
//*----------------------------------------------------------------------------------------
void GameObject::ComponentUpdate(RendererEngine& renderer)
{
	// 初めての更新の場合はコンポーネントのStartメソッドを呼ぶようにする
	if (m_IsCalcUpdate == false)
	{
		for (auto &comp : m_pComponentMap)
		{
			comp.second->Start(renderer);
		}
	}

	for (auto& comp : m_pComponentMap)
	{
		comp.second->Update(renderer);
	}

	m_IsCalcUpdate = true;
}

//*---------------------------------------------------------------------------------------
//* @:Object Class 
//*【?】コンポーネントの更新後の更新
//* 引数：描画エンジンの参照
//* 戻値：void
//*----------------------------------------------------------------------------------------
void GameObject::ComponentLateUpdate(RendererEngine& renderer)
{
	for (auto& comp : m_pComponentMap)
	{
		comp.second->LateUpdate(renderer);
	}
}


//*---------------------------------------------------------------------------------------
//* @:Object Class 
//*【?】コンポーネントの描画
//* 引数：描画エンジンの参照
//* 戻値：void
//*----------------------------------------------------------------------------------------
void GameObject::ComponentRender(RendererEngine &renderer)
{
	for (auto &comp : m_pComponentMap)
	{
		// 表示するか
		// シャドウパスの場合は表示させる
		if(!comp.second->IsVisible(renderer.get_MainCameraFrustum()) && 
			renderer.get_CrntRenderPass() !=  RenderData::RENDER_PASS::SHADOW)
		{
			continue;
		}
		comp.second->Draw(renderer);
	}
}

//*---------------------------------------------------------------------------------------
//*【?】衝突判定を受け取る
//*
//* [引数]
//* &info : 衝突情報
//*
//* [返値] なし
//*----------------------------------------------------------------------------------------
void GameObject::OnCollisionEnter(const PhysicsData::CollisionInfo &info)
{
	// 自身が持っている全てのコンポーネントに対してループ処理を行う
	for (auto & comp : m_pComponentMap)
	{
		// アクティブなもののみ
		if (comp.second->get_IsStatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE))
		{
			comp.second->OnCollisionEnter(info);
		}
	}
}

//*---------------------------------------------------------------------------------------
//*【?】衝突判定を受け取る
//*
//* [引数]
//* &info : 衝突情報
//*
//* [返値] なし
//*----------------------------------------------------------------------------------------
void GameObject::OnTriggerEnter(const PhysicsData::CollisionInfo &info)
{
	// 自身が持っている全てのコンポーネントに対してループ処理を行う
	for (auto & comp : m_pComponentMap)
	{
		// アクティブなもののみ
		if (comp.second->get_IsStatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE))
		{
			comp.second->OnTriggerEnter(info);
		}
	}
}

//*---------------------------------------------------------------------------------------
//* @:Object Class 
//*【?】Transformの取得
//* 引数：なし
//* 戻値：弱参照ポインタ
//*----------------------------------------------------------------------------------------
std::weak_ptr<MyTransform> GameObject::get_Transform() const
{
	return m_pTransform;
}

//*---------------------------------------------------------------------------------------
//* @:Object Class 
//*【?】RectTransformの取得
//* 引数：なし
//* 戻値：弱参照ポインタ
//*----------------------------------------------------------------------------------------
std::weak_ptr<RectTransform> GameObject::get_RectTransform()const
{
	if (m_pTransform->get_IsRectTransform())
	{
		return std::static_pointer_cast<RectTransform>(m_pTransform);
	}

	return {};
}

//*---------------------------------------------------------------------------------------
//* @:Object Class 
//*【?】読み取り専用トランスフォーム取得
//* 引数：なし
//* 戻値：読み取り専用ポインタ
//*----------------------------------------------------------------------------------------
const MyTransform* GameObject::get_TransformConst()const
{
	return m_pTransform.get();
}

//*---------------------------------------------------------------------------------------
//* @:Object Class 
//*【?】読み取り専用トランスフォーム取得
//* 引数：なし
//* 戻値：読み取り専用ポインタ
//*----------------------------------------------------------------------------------------
const RectTransform* GameObject::get_RectTransformConst()const
{
	if (m_pTransform->get_IsRectTransform())
	{
		return std::static_pointer_cast<RectTransform>(m_pTransform).get();
	}

	return {};
}



//Transform* GameObject::get_Transform() const
//{
//	return m_pTransform.get();
//}

//*---------------------------------------------------------------------------------------
//* @:Object Class 
//*【?】Transformの取得
//* 引数：なし
//* 戻値：弱参照ポインタ
//*----------------------------------------------------------------------------------------
std::unordered_map<std::type_index, std::shared_ptr<IComponent>> GameObject::get_ComponentMap()const
{
	return m_pComponentMap;
}


