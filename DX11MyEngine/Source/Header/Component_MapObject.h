#pragma once
#include "IComponent.h"


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:MapObject Class --- */
//
//  ★継承：IComponent ★
//
// 【?】マップ内に配置されるすべてのオブジェクトに取り付けるコンポーネント
//		
// ***************************************************************************************
class MapObject : public IComponent
{
private:
	std::string m_MyAssetID;			// 自身のデータ参照用ID
	Map::MAP_OBJECT_TYPE m_ObjectType;

public:
	MapObject(std::weak_ptr<GameObject> pOwner, int updateRank);
	~MapObject();

	void Start(RendererEngine& renderer) override;	// 初期化
	void Update(RendererEngine& renderer) override;	// 更新
	void OnCollisionEnter(const PhysicsData::CollisionInfo& _other)override;

	void set_AssetID(const std::string& _assetID) { m_MyAssetID = _assetID; };
	std::string get_AssetID()const { return m_MyAssetID; }
};

