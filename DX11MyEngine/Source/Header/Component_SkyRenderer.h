#pragma once
#include "IComponent.h"


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:SkyRenderer Class --- */
//
//  ★継承：IComponent ★
//
// 【?】IMeshResourceから受け取った情報から、メッシュの描画をする
//		MeshRendererと似た設計
//		深度テストで毎回奥になるようにする
//		SkyboxかSkydorm選択できるようにする
//		基本、一つのシーンに一つ存在する
//		
// ***************************************************************************************
class SkyRenderer : public IComponent
{
private:
	std::weak_ptr<class IMeshResource> m_pMeshResource;	// メッシュ情報の参照
	
	// 形状タイプ (分ける必要あまり無いかも)
	enum class SKY_GEOMETRY_TYPE{
		BOX,
		DORM,
	}m_GeometryType;

	// RendererEngine側で深度テストを行わない用のものを用意しておく
	// Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_pDSS;

public:
	SkyRenderer(std::weak_ptr<GameObject> pOwner, int updateRank = 100);
	~SkyRenderer();

	void Start(RendererEngine& renderer) override;		// 初期化
	void Update(RendererEngine& renderer) override;		// 更新処理
	void Draw(RendererEngine& renderer) override;		// 描画処理
	void set_MeshResource(std::weak_ptr<class IMeshResource> meshResource);

    std::shared_ptr<class Material> get_Material() const;
    bool set_Material(const std::shared_ptr<class Material>& material);

};

