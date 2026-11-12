#include "pch.h"
#include "Component_ModelChange.h"
#include "Component_ModelMeshResource.h"

using namespace UtilityData;
using namespace VECTOR4;
using namespace VECTOR3;

//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//*----------------------------------------------------------------------------------------
ModelChange::ModelChange(std::weak_ptr<GameObject> pOwner, int updateRank) :
	IComponent(pOwner, updateRank)
{

}

//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
ModelChange::~ModelChange()
{

}

//*---------------------------------------------------------------------------------------
//*【?】初期化
//*
//* [引数]
//* RendererEngine& : 描画エンジンの参照
//*
//* [返値]
//* void
//*----------------------------------------------------------------------------------------
void ModelChange::Start(RendererEngine& renderer)
{
}

//*---------------------------------------------------------------------------------------
//*【?】更新
//*
//* [引数]
//* RendererEngine& : 描画エンジンの参照
//*
//* [返値]
//* void
//*----------------------------------------------------------------------------------------
void ModelChange::Update(RendererEngine& renderer)
{
}


//*---------------------------------------------------------------------------------------
//*【?】モデルデータの登録
//*     そのモデルに対応させるスロット番号を入れ、ChangeModelで使う
//*     スロット番号は0番から指定すること
//*
//* [引数]
//* _pData	   : モデルデータ
//*
//* [返値]
//* スロット番号 
//*----------------------------------------------------------------------------------------
size_t  ModelChange::RegisterModelData(const ModelLODSet& models)
{
	size_t slot = m_ModelList.size();
	m_ModelList.push_back(models);
	return slot;
}

//*---------------------------------------------------------------------------------------
//*【?】モデルの切り替え
//*
//* [引数]
//* _slotIndex : 切り替え先のモデルのスロット番号
//*
//* [返値] なし
//*----------------------------------------------------------------------------------------
bool ModelChange::ChangeModel(int _slotIndex)
{
	// 範囲外チェック
	if (_slotIndex >= m_ModelList.size())
	{
		Tool::ErrorMessage(L"範囲外インデックスです", L"ModelChange");
		assert(false);
		return false;
	}

	// メッシュリソースコンポーネントの生存チェック
	auto resource = m_pModelResource.lock();
	if (!resource)
	{
		Tool::ErrorMessage(L"リソースコンポーネントエラー", L"ModelChange");
		assert(false);
		return false;
	}    
	
	// 切り替え
	resource->set_LODModelData(m_ModelList[_slotIndex]);

	return true;
}