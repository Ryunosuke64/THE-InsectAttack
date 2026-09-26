#pragma once

/// <summary>
/// 衝突の判定分岐
/// </summary>
enum class COLLISION_CALC_TYPE
{
    BOX_BOX,        // 箱 と 箱
    SPHRERE_SPHRERE,// 球 と 球
    BOX_SPHRERE,    // 箱 と 球
    RAY_RAY,        // 線分 と 線分
    BOX_RAY,        // 箱 と 線分
    SPHERE_RAY,     // 球 と 線分

    NUM,
};

/// <summary>
/// 3Dレイ判定用
/// </summary>
struct CollInData_Ray
{
    VECTOR3::VEC3 _point;    // 開始点 
    VECTOR3::VEC3 _dir;      // 方向
};

/// <summary>
/// 3D線分判定用
/// </summary>
struct CollInData_Segment
{
    VECTOR3::VEC3 _start;   // 開始点 
    VECTOR3::VEC3 _end;     // 終了点
};

/// <summary>
/// 三角形判定用
/// </summary>
struct CollInData_Triangle
{
	VECTOR3::VEC3 _v0;
	VECTOR3::VEC3 _v1;
	VECTOR3::VEC3 _v2;
};

/// <summary>
/// 3D球判定
/// </summary>
struct CollInData_Sphere
{
    VECTOR3::VEC3 _pos;
    float _radius;
};

/// <summary>
/// 3DボックスAABB判定用
/// </summary>
struct CollInData_AABB
{
    VECTOR3::VEC3 _min; 
    VECTOR3::VEC3 _max;
};

/// <summary>
/// 3D平面判定用
/// </summary>
struct CollInData_Plane
{
    VECTOR3::VEC3 _point;   // 任意の点
    VECTOR3::VEC3 _norm;    // 法線
};

/// <summary>
/// 2DボックスAABB判定用
/// </summary>
struct CollInData2D_AABB
{
    VECTOR2::VEC2 _min;
    VECTOR2::VEC2 _max;
};

// Oriented Bounding Box
struct CollInData_OBB
{
    VECTOR3::VEC3 _center;      // 中心位置
    VECTOR3::VEC3 _axis[3];     // 各座標軸の傾き
    VECTOR3::VEC3 _harfLength;  //ローカルのx, y, z軸に沿ったハーフサイズ
};

struct CollisionProxy
{
    uint32_t colliderIndex;

    class Collider* collider;
    std::shared_ptr<MyTransform> transform;

    CollInData_AABB worldBounds;

    unsigned category;
    unsigned collisionMask;

    bool isStatic;
};

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:CollisionManager Class --- */
//
//  ★★★シングルトン★★★
//
// 【?】衝突判定の管理
//   [※ BulletPhysicsによる衝突判定へ移行するため、廃止予定]
// 
// 参考サイト：https://yutateno.hatenablog.jp/entry/2019/11/27/001801 
//           ：https://developer.mozilla.org/ja/docs/Games/Techniques/3D_collision_detection
//           ：https://qiita.com/Aqua-218/items/a432cf0410bff57202c5
//
// ***************************************************************************************
class CollisionManager
{
private:
    struct CollisionQueryDebugMetrics
    {
        int RaycastQueryCount = 0;
        int RaycastColliderScanCount = 0;
        int RaycastNarrowPhaseCount = 0;
        int RaycastHitCount = 0;
        double RaycastTotalTimeMs = 0.0;

        int SphereQueryCount = 0;
        int SphereColliderScanCount = 0;
        int SphereNarrowPhaseCount = 0;
        int SphereHitCount = 0;
        double SphereTotalTimeMs = 0.0;
    };

    // 衝突計算をするコライダーの配列
    std::vector<std::shared_ptr<class Collider>> m_pCollidersList;

    // 前回のCollisionProcess終了後から今回開始までの問い合わせ計測
    CollisionQueryDebugMetrics m_QueryDebugMetrics;

    // 直近フレームの処理時間履歴
    std::vector<double> m_CollisionProcessTimeHistory;
    std::vector<double> m_RaycastTimeHistory;
    std::vector<double> m_SphereQueryTimeHistory;

public:
    CollisionManager();
    ~CollisionManager();


    /// <summary>
    /// 衝突判定の更新
    /// </summary>
    void CollisionProcess();

    /// <summary>
    /// コライダーの登録
    /// </summary>
    /// <param name="pCol"></param>
    void RegisterCollider(std::shared_ptr<class Collider> pCol);

    /// <summary>
    /// 衝突判定
    /// </summary>
    /// <param name="_colA">Aコライダー</param>
    /// <param name="_colB">Bコライダー</param>
    /// <param name="_transA">Aトランスフォーム</param>
    /// <param name="_transB">Bトランスフォーム</param>
    /// <param name="info">data格納先</param>
    /// <returns>衝突したか</returns>
    bool HitCheck(std::shared_ptr<class Collider> _colA,std::shared_ptr<class Collider> _colB, std::shared_ptr<class MyTransform> _transA,std::shared_ptr<class MyTransform> _transB, PhysicsData::CollisionInfo* info);
    
    /// <summary>
    /// レイキャスト判定
    /// </summary>
    /// <param name="_collider">コライダー</param>
    /// <param name="_transform">トランスフォーム</param>
    /// <param name="_ray">レイ情報</param>
    /// <param name="_outHitInfo">data格納先</param>
    /// <returns>衝突したか</returns>
    bool HitCheck_Raycast(std::shared_ptr<class Collider> _collider,  std::shared_ptr<class MyTransform> _transform, const CollInData_Ray& _ray, float *_outDist, PhysicsData::CollisionInfo* _outHitInfo);

    /// <summary>
    /// 範囲内のオブジェクトを取得する
    /// </summary>
    std::vector<std::shared_ptr<class Collider>> CheckSphere(const VECTOR3::VEC3& _center, float _radius, unsigned _mask);


    /// <summary>
    /// レイキャスト判定
    /// </summary>
    /// <param name="_ray"></param>
    /// <param name="_outHitInfo"></param>
    /// <returns></returns>
    bool CheckRaycast(const CollInData_Ray& _ray,int _mask, PhysicsData::CollisionInfo* _outHitInfo);

    //*****************************************************************************************
    //						 3D 
    //*****************************************************************************************
    // 箱と箱 物理的判定
    bool HitCheck_BoxVsBox_Physics(const CollInData_AABB &_src, const CollInData_AABB &_dst, PhysicsData::CollisionInfo *info);

    // 箱と箱
    bool HitCheck_BoxVsBox(const CollInData_AABB &_src, const CollInData_AABB &_dst);
    
    // 箱と箱（OBB）
    bool HitCheck_OBBVsOBB(const CollInData_OBB &_src, const CollInData_OBB&_dst, PhysicsData::CollisionInfo* _hitInfo);

    // 箱と点
    bool HitCheck_BoxVsPoint(const CollInData_AABB &box, const VECTOR3::VEC3& _p);    
    

    // 箱と球
    bool HitCheck_BoxVsSphere(const CollInData_AABB &_box, const CollInData_Sphere &_sphere);

    // 球と球
    bool HitCheck_SphereVsSphere(const CollInData_Sphere &_src, const CollInData_Sphere &_dst);

    //*****************************************************************************************
    //						 レイキャスト 
    //*****************************************************************************************
    // 平面とレイ
    bool HitCheck_PlaneVsRay(const CollInData_Plane& _plane, const CollInData_Ray& _ray, PhysicsData::CollisionInfo* _hitInfo );
    
    // 箱とレイ
    bool HitCheck_BoxVsRay(const CollInData_AABB& _box, const CollInData_Ray& _ray, PhysicsData::CollisionInfo* _hitInfo);
    
	// 三角形とレイ
	bool HitCheck_TraiangleVsRay(const CollInData_Triangle& _triangle, const CollInData_Ray& _ray, float& u, float& v, float& t);

    // 球とレイ
    bool HitCheck_SphereVsRay(const CollInData_Sphere& _sphere, const CollInData_Ray& _ray, PhysicsData::CollisionInfo* _hitInfo );

    // 平面と線分
    bool HitCheck_PlaneVsSegment(const CollInData_Plane& _plane, const CollInData_Segment& _segment);

    //*****************************************************************************************
    //						 2D 
    //*****************************************************************************************
    // 箱と点
    bool HitCheck2D_BoxVsPoint(const CollInData2D_AABB& _box, const VECTOR2::VEC2& _p);



private:
    // コピー禁止
    CollisionManager(const CollisionManager &) = delete;
    CollisionManager &operator=(const CollisionManager &) = delete;
    // ------------------------------------------------------

    /// <summary>
    /// 最終的な衝突応答を求める（応答がそれぞれ違った場合にどれを優先するかなど）
    /// </summary>
    /// <param name="responseA">応答A</param>
    /// <param name="responseB">応答B</param>
    /// <returns>最終的な衝突応答</returns>
    UtilityData::COLLISION_RESPONSE CombineResponse(
        UtilityData::COLLISION_RESPONSE responseA,
        UtilityData::COLLISION_RESPONSE responseB);
};

