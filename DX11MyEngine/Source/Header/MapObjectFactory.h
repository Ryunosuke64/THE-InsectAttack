#pragma once


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:EnemyFactory Class --- */
//
// 【?】マップオブジェクトの生成を行う
//
// ***************************************************************************************
class MapObjectFactory
{
public:
	static bool CreateMapObject(const Map::MapObjectData& data);

private:
	static std::shared_ptr<class Collider> SetupColliderBox(const Map::MO_ColliderData& data, std::shared_ptr<GameObject> obj);
	static std::shared_ptr<class Collider> SetupColliderSphere(const Map::MO_ColliderData& data, std::shared_ptr<GameObject> obj);
	static std::shared_ptr<class Collider> SetupColliderCapsule(const Map::MO_ColliderData& data, std::shared_ptr<GameObject> obj);
	static std::shared_ptr<class Collider> SetupColliderCylinder(const Map::MO_ColliderData& data, std::shared_ptr<GameObject> obj);
	static std::shared_ptr<class Collider> SetupColliderConvexHull(const Map::MO_ColliderData& data, std::shared_ptr<GameObject> obj, std::weak_ptr<class ModelData> modelData);
	static std::shared_ptr<class Collider> SetupColliderConcave(const Map::MO_ColliderData& data, std::shared_ptr<GameObject> obj, std::weak_ptr<class ModelData> modelData);
};

