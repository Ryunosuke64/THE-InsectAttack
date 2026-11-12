#pragma once
#include "IComponent.h"

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:ModelChange Class --- */
//
//  ★継承：IComponent ★
//
// 【?】事前に複数のモデルデータの参照を入れておき、
//		要求があればモデルの差し替えをしてくれるようにするためのもの
//		
// ***************************************************************************************
class ModelChange : public IComponent
{
private:
	std::vector<ModelLODSet> m_ModelList;						// モデル切り替えを行うモデルデータ
	std::weak_ptr<class ModelMeshResource> m_pModelResource;	// 変更を適用するモデルメッシュリソースコンポーネント

public:
	ModelChange(std::weak_ptr<GameObject> pOwner, int updateRank = 100);
	~ModelChange();

	void Start(RendererEngine& renderer) override;		// 初期化
	void Update(RendererEngine& renderer) override;		// 初期化

	// モデルメッシュリソースコンポーネントの設定
	void set_ModelMeshResource(std::shared_ptr<class ModelMeshResource> _pResource) { m_pModelResource = _pResource; }

	// モデルデータの登録
	// そのモデルに対応させるスロット番号を入れ、ChangeModelで使う
	// スロット番号は0番から指定すること
	size_t  RegisterModelData(const ModelLODSet& models);

	// モデルの切り替え
	bool  ChangeModel(int _slotIndex);
};

