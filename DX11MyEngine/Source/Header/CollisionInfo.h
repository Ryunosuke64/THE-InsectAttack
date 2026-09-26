#pragma once
//constexpr int MAX_CONTACT_POINTS_SIZE = 4;  // 接触点情報の最大数
//
//namespace PhysicsData
//{
//    // ***************************************************************************************
//    // ---------------------------------------------------------------------------------------
//    /* --- @:CollisionInfo Class --- */
//    //
//    // 【?】衝突時の情報をまとめたクラス
//    //
//    // ***************************************************************************************
//    struct CollisionInfo
//    {
//        std::weak_ptr<class GameObject> hitObject;     // 衝突相手
//        std::weak_ptr<class MyTransform> hitTransform; // 衝突相手のトランスフォーム
//        std::weak_ptr<class Collider> hitCollider;     // 衝突相手のコライダー
//        VECTOR3::VEC3 hitPoint;                         // 衝突位置
//        VECTOR3::VEC3 hitNormal;                        // 衝突面の向き
//        VECTOR3::VEC3 relativeVelocity;                 // 衝突した物体の相対速度
//        float penetrationDepth;                         // めり込み量
//
//        std::array<ContactPoint, MAX_CONTACT_POINTS_SIZE> contacts; // 接触点情報
//        int contactCount;



        //public:
        //    ~CollisionInfo();
        //
        //
        //    //*---------------------------------------------------------------------------------------
        //    //*【?】衝突点情報
        //    //*     設定は必ずPhysicsEngine側から設定する 
        //    //*----------------------------------------------------------------------------------------
        //    void set_ContactPoints(const PhysicsData::ContactPoint& _contactPoint, int _index);
        //    void set_ContactCount(int _count) { m_ContactCount = _count; }
        //
        //    std::array<PhysicsData::ContactPoint, MAX_CONTACT_POINTS_SIZE> 
        //        get_ContactPoints()const { return m_ContactPoints; };    
        //    int get_ContactCount() const{ return m_ContactCount; };
        //
        //    //*---------------------------------------------------------------------------------------
        //    //*【?】衝突相手のオブジェクト
        //    //*----------------------------------------------------------------------------------------
        //    void set_HitObject(std::weak_ptr<GameObject> _pObj) { m_pHitObject = _pObj; }
        //    std::weak_ptr<GameObject> get_HitObject()const { return m_pHitObject; };
        //    
        //    //*---------------------------------------------------------------------------------------
        //    //*【?】衝突相手のトランスフォーム
        //    //*----------------------------------------------------------------------------------------
        //    void set_HitTransform(std::weak_ptr<MyTransform> _pTrans) { m_pHitTransform = _pTrans; }
        //    std::weak_ptr<MyTransform> get_HitTransform()const { return m_pHitTransform; };
        //
        //    //*---------------------------------------------------------------------------------------
        //    //*【?】衝突したコライダー
        //    //*----------------------------------------------------------------------------------------
        //    void set_HitCollider(std::weak_ptr<class Collider> _pColl) { m_pHitCollider = _pColl; }
        //    std::weak_ptr<class Collider> get_HitCollider()const { return m_pHitCollider; };
        //
        //    //*---------------------------------------------------------------------------------------
        //    //*【?】衝突した場所
        //    //*----------------------------------------------------------------------------------------
        //    void set_HitPoint(const VECTOR3::VEC3 &_vIn) { m_HitPoint = _vIn; }
        //    VECTOR3::VEC3 get_HitPoint()const { return m_HitPoint; }
        //
        //    //*---------------------------------------------------------------------------------------
        //    //*【?】衝突面の法線
        //    //*----------------------------------------------------------------------------------------
        //    void set_HitNormal(const VECTOR3::VEC3 &_vIn) { m_HitNormal = _vIn; }
        //    VECTOR3::VEC3 get_HitNormal()const { return m_HitNormal; }
        //    
        //    //*---------------------------------------------------------------------------------------
        //    //*【?】衝突した物体の相対速度
        //    //*----------------------------------------------------------------------------------------
        //    void set_RelativeVelocity(const VECTOR3::VEC3 &_vIn) { m_RelativeVelocity = _vIn; }
        //    VECTOR3::VEC3 get_RelativeVelocity()const { return m_HitPoint; }
        //    
        //    //*---------------------------------------------------------------------------------------
        //    //*【?】めり込み深度
        //    //*----------------------------------------------------------------------------------------
        //    void set_PenetrationDepth(const float &_depth) { m_PenetrationDepth = _depth; }
//        //    float get_PenetrationDepth()const { return m_PenetrationDepth; }
//
//    };
//
//}