#include "pch.h"
#include "CollisionManager.h"
#include "Component_Collider.h"
#include "Component_BoxCollider.h"
#include "Component_SphereCollider.h"
#include "CollisionInfo.h"
#include "Component_Transform.h"
#include "GameObject.h"

#include <chrono>

using namespace UtilityData;
using namespace PhysicsData;
using namespace VECTOR3;

namespace
{
    constexpr size_t COLLISION_DEBUG_HISTORY_SIZE = 300;

    struct CollisionDebugTimeSummary
    {
        double AverageMs = 0.0;
        double MaxMs = 0.0;
        double P95Ms = 0.0;
        double P99Ms = 0.0;
    };

    void AddCollisionDebugTimeSample(std::vector<double>& _history, double _elapsedMs)
    {
        if (_history.size() >= COLLISION_DEBUG_HISTORY_SIZE)
        {
            _history.erase(_history.begin());
        }

        _history.push_back(_elapsedMs);
    }

    CollisionDebugTimeSummary CalculateCollisionDebugTimeSummary(const std::vector<double>& _history)
    {
        CollisionDebugTimeSummary summary;
        if (_history.empty())
        {
            return summary;
        }

        std::vector<double> sortedHistory = _history;
        std::sort(sortedHistory.begin(), sortedHistory.end());

        double totalMs = 0.0;
        for (double elapsedMs : sortedHistory)
        {
            totalMs += elapsedMs;
        }

        const size_t sampleCount = sortedHistory.size();
        const size_t p95Index = ((sampleCount * 95 + 99) / 100) - 1;
        const size_t p99Index = ((sampleCount * 99 + 99) / 100) - 1;

        summary.AverageMs = totalMs / static_cast<double>(sampleCount);
        summary.MaxMs = sortedHistory.back();
        summary.P95Ms = sortedHistory[p95Index];
        summary.P99Ms = sortedHistory[p99Index];
        return summary;
    }
}

CollisionManager::CollisionManager()
{
}

CollisionManager::~CollisionManager()
{
    m_pCollidersList.clear();
}

//*---------------------------------------------------------------------------------------
//*【?】衝突判定を行う
//* [引数] pCol : 登録するコライダー
//* [返値] なし
//*----------------------------------------------------------------------------------------
void CollisionManager::RegisterCollider(std::shared_ptr<Collider> pCol)
{
    m_pCollidersList.push_back(pCol);
}


//*---------------------------------------------------------------------------------------
//*【?】衝突判定を行う
//* [引数] なし
//* [返値] なし
//*----------------------------------------------------------------------------------------
void CollisionManager::CollisionProcess()
{
    // RaycastとCheckSphereはCollisionProcessの前後から呼ばれるため、
    // 前回のCollisionProcess終了後から今回開始までを1区間として表示する。
    const CollisionQueryDebugMetrics queryMetrics = m_QueryDebugMetrics;
    m_QueryDebugMetrics = {};

    const auto collisionProcessStart = std::chrono::steady_clock::now();

    const int colliderCountBeforeCleanup = static_cast<int>(m_pCollidersList.size());

    // コライダーがないまたは、オブジェクトがない場合は削除
    m_pCollidersList.erase(
        std::remove_if(
            m_pCollidersList.begin(),
            m_pCollidersList.end(),
            [](const std::shared_ptr<Collider> pIn)
            {
                return (pIn == nullptr || pIn->get_OwnerObj().expired());
            }),
        m_pCollidersList.end()
    );

    const int registeredColliderCount = static_cast<int>(m_pCollidersList.size());
    const int removedColliderCount = colliderCountBeforeCleanup - registeredColliderCount;

    // 全てのコライダーのヒットフラグを一旦falseに
    int enabledColliderCount = 0;
    for (auto &comp : m_pCollidersList)
    {
        comp->set_IsHit(false);
        if (comp->get_IsEnable())
        {
            enabledColliderCount++;
        }
    }

    int processAllCount = 0;
    int processCount = 0;
    int staticPairSkipCount = 0;
    int collisionMaskSkipCount = 0;
    int missingTransformCount = 0;
    int hitCount = 0;
    int blockCount = 0;
    int overlapCount = 0;
    double narrowPhaseTimeMs = 0.0;
    double eventNotificationTimeMs = 0.0;

    // 判定ループ処理
    for (int i = 0; i < m_pCollidersList.size(); i++)
    {
        auto &colA = m_pCollidersList[i];
        bool isStaticA = false;
        bool isStaticB = false;
        auto transA = colA->get_OwnerObj().lock()->get_Component<MyTransform>();
        if (transA == nullptr) {
            missingTransformCount++;
            MessageBoxA(NULL, "Aトランスフォームコンポーネントがありません", "衝突判定", MB_OK);
            continue;
        }

        // 使用フラグチェック
        if (colA->get_IsEnable() == false) {
            continue;
        }
        
        for (int j = i + 1; j < m_pCollidersList.size(); j++)
        {
            // 処理回数カウント
            processAllCount++;

            auto& colB = m_pCollidersList[j];

            // 使用フラグチェック
            if (colB->get_IsEnable() == false){
                continue;
            }

            isStaticA = colA->get_IsStatic();
            isStaticB = colB->get_IsStatic();

            // 両方とも静的なら判定しない
            if (isStaticA && isStaticB)
            {
                staticPairSkipCount++;
                continue;
            }
            
            bool collisionMaskA = GIGA_Engine::BitFlag::CheckAny(colA->get_CollisionBitMask(), UINT_CAST(colB->get_CollisionCategory()));
            bool collisionMaskB = GIGA_Engine::BitFlag::CheckAny(colB->get_CollisionBitMask(), UINT_CAST(colA->get_CollisionCategory()));

            // 衝突マスクのチェック
            if (collisionMaskA == 0 ||
                collisionMaskB == 0)
            {
                collisionMaskSkipCount++;
                continue;
            }

            // トランスフォームの取得
            auto transB = colB->get_OwnerObj().lock()->get_Component<MyTransform>();

            if (transB == nullptr) {
                missingTransformCount++;
                MessageBoxA(NULL, "Bトランスフォームコンポーネントがありません", "衝突判定", MB_OK);
                continue;
            }


            CollisionInfo info; // 衝突情報格納用

            processCount++;

            // 衝突チェック
            const auto narrowPhaseStart = std::chrono::steady_clock::now();
            const bool isHit = HitCheck(colA, colB, transA, transB, &info);
            const auto narrowPhaseEnd = std::chrono::steady_clock::now();
            narrowPhaseTimeMs += std::chrono::duration<double, std::milli>(
                narrowPhaseEnd - narrowPhaseStart).count();

            if (isHit)
            {
                hitCount++;

                // 衝突
                colA->set_IsHit(true);
                colB->set_IsHit(true);

                //// トリガー判定（responseを追加したのでトリガーが要らないかも）
                //bool isTriggerHit_A = colA->get_IsTrigger();
                //bool isTriggerHit_B = colB->get_IsTrigger();

                //--------------------------------
                // 衝突応答
                //--------------------------------
                COLLISION_RESPONSE responseA =
                    colA->get_Response(colB->get_CollisionCategory());

                COLLISION_RESPONSE responseB =
                    colB->get_Response(colA->get_CollisionCategory());

                COLLISION_RESPONSE finalResponse =
                    CombineResponse(responseA, responseB);


                // Blockの場合だけ押し出す
                if (GIGA_Engine::BitFlag::CheckAny(
                    static_cast<unsigned>(finalResponse),
                    static_cast<unsigned>(COLLISION_RESPONSE::RESPONSE_BLOCK))
                    )
                {
                    VEC3 pushVector;    // 押し出しベクトル
                    VEC3 currentPos;    // 押し出し反映用

                    // 押し出すベクトル = 法線 * めり込み量
                    pushVector = info.hitNormal * info.penetrationDepth;

                    // 静的オブジェクトの場合は0.0にする（押し出さない）
                    // 押し出し比率
                    float ratioA = 0.5f;
                    float ratioB = 0.5f;

                    if (isStaticA && !isStaticB) {
                        // Aが静的、Bが動的
                        ratioA = 0.0f;
                        ratioB = 1.0f;
                    }
                    else if (!isStaticA && isStaticB) {
                        // Aが動的、Bが静的
                        ratioA = 1.0f;
                        ratioB = 0.0f;
                    }


                    // Aは法線方向に押し出す *******************************************
                    currentPos = transA->get_VEC3ToPos();
                    transA->set_Pos(currentPos + (pushVector * ratioA));

                    // Bは衝突された側なので法線とは逆方向に押し出す ********************
                    currentPos = transB->get_VEC3ToPos();
                    transB->set_Pos(currentPos + (-pushVector * ratioB));
                }

                // Bと衝突したことをAオブジェクト側に伝える
                CollisionInfo infoA = info;
                infoA.hitObject = colB->get_OwnerObj();
                infoA.hitCollider = colB;
                infoA.hitNormal = -info.hitNormal; // そのままだとぶつかった側の押し出し法線なので、法線を反転させる


                // Aと衝突したことをBオブジェクト側に伝える
                CollisionInfo infoB = info;
                infoB.hitObject  = colA->get_OwnerObj();
                infoB.hitCollider = colA;

                // Responseに応じてイベントを通知
                const auto eventNotificationStart = std::chrono::steady_clock::now();
                switch (finalResponse)
                {
                case COLLISION_RESPONSE::RESPONSE_OVERLAP:
                    overlapCount++;
                    colA->get_OwnerObj().lock()->OnTriggerEnter(infoA);
                    colB->get_OwnerObj().lock()->OnTriggerEnter(infoB);
                    break;

                case COLLISION_RESPONSE::RESPONSE_BLOCK:
                    blockCount++;
                    colA->get_OwnerObj().lock()->OnCollisionEnter(infoA);
                    colB->get_OwnerObj().lock()->OnCollisionEnter(infoB);
                    break;

                case COLLISION_RESPONSE::RESPONSE_IGNORE:
                    break;
                }
                const auto eventNotificationEnd = std::chrono::steady_clock::now();
                eventNotificationTimeMs += std::chrono::duration<double, std::milli>(
                    eventNotificationEnd - eventNotificationStart).count();
            }
        }
    }


    //////////////////////////////////////////////////////////////////////////////////////////
    //						デバッグ用
    //              ※ デバッグモードが有効の際に表示
    //////////////////////////////////////////////////////////////////////////////////////////
    if (Master::m_pDataManager->get_IsDebugMode() == false)return;

    const auto collisionProcessEnd = std::chrono::steady_clock::now();
    const double collisionProcessTimeMs = std::chrono::duration<double, std::milli>(
        collisionProcessEnd - collisionProcessStart).count();

    AddCollisionDebugTimeSample(m_CollisionProcessTimeHistory, collisionProcessTimeMs);
    AddCollisionDebugTimeSample(m_RaycastTimeHistory, queryMetrics.RaycastTotalTimeMs);
    AddCollisionDebugTimeSample(m_SphereQueryTimeHistory, queryMetrics.SphereTotalTimeMs);

    const CollisionDebugTimeSummary collisionTimeSummary =
        CalculateCollisionDebugTimeSummary(m_CollisionProcessTimeHistory);
    const CollisionDebugTimeSummary raycastTimeSummary =
        CalculateCollisionDebugTimeSummary(m_RaycastTimeHistory);
    const CollisionDebugTimeSummary sphereTimeSummary =
        CalculateCollisionDebugTimeSummary(m_SphereQueryTimeHistory);

    Master::m_pDebugger->BeginDebugWindow("CollisionProcess",0);
    Master::m_pDebugger->DG_TextValue(
        Tool::U8ToChar(u8"登録コライダー: %d / 有効: %d / 今回削除: %d"),
        registeredColliderCount, enabledColliderCount, removedColliderCount);
    Master::m_pDebugger->DG_TextValue(
        Tool::U8ToChar(u8"ペアループ: %d / Static除外: %d / マスク除外: %d"),
        processAllCount, staticPairSkipCount, collisionMaskSkipCount);
    Master::m_pDebugger->DG_TextValue(
        Tool::U8ToChar(u8"形状判定: %d / ヒット: %d / Transformなし: %d"),
        processCount, hitCount, missingTransformCount);
    Master::m_pDebugger->DG_TextValue(
        Tool::U8ToChar(u8"Block: %d / Overlap: %d"), blockCount, overlapCount);
    Master::m_pDebugger->DG_TextValue(
        Tool::U8ToChar(u8"形状判定時間: %.3f ms / イベント通知時間: %.3f ms"),
        narrowPhaseTimeMs, eventNotificationTimeMs);
    Master::m_pDebugger->DG_TextValue(
        Tool::U8ToChar(u8"CollisionProcess: %.3f ms"), collisionProcessTimeMs);
    Master::m_pDebugger->DG_TextValue(
        Tool::U8ToChar(u8"直近%zuフレーム  平均: %.3f / 最大: %.3f / p95: %.3f / p99: %.3f ms"),
        m_CollisionProcessTimeHistory.size(),
        collisionTimeSummary.AverageMs,
        collisionTimeSummary.MaxMs,
        collisionTimeSummary.P95Ms,
        collisionTimeSummary.P99Ms);

    Master::m_pDebugger->DG_TextValue(
        Tool::U8ToChar(u8"--- 前回CollisionProcess以降の問い合わせ ---"));
    Master::m_pDebugger->DG_TextValue(
        Tool::U8ToChar(u8"Raycast 回数: %d / 走査: %d / 形状判定: %d / ヒット: %d"),
        queryMetrics.RaycastQueryCount,
        queryMetrics.RaycastColliderScanCount,
        queryMetrics.RaycastNarrowPhaseCount,
        queryMetrics.RaycastHitCount);
    Master::m_pDebugger->DG_TextValue(
        Tool::U8ToChar(u8"Raycast 合計: %.3f ms / 1回平均: %.3f ms"),
        queryMetrics.RaycastTotalTimeMs,
        queryMetrics.RaycastQueryCount > 0
            ? queryMetrics.RaycastTotalTimeMs / queryMetrics.RaycastQueryCount
            : 0.0);
    Master::m_pDebugger->DG_TextValue(
        Tool::U8ToChar(u8"Raycast直近%zuフレーム  平均: %.3f / 最大: %.3f / p95: %.3f / p99: %.3f ms"),
        m_RaycastTimeHistory.size(),
        raycastTimeSummary.AverageMs,
        raycastTimeSummary.MaxMs,
        raycastTimeSummary.P95Ms,
        raycastTimeSummary.P99Ms);

    Master::m_pDebugger->DG_TextValue(
        Tool::U8ToChar(u8"CheckSphere 回数: %d / 走査: %d / 形状判定: %d / ヒット: %d"),
        queryMetrics.SphereQueryCount,
        queryMetrics.SphereColliderScanCount,
        queryMetrics.SphereNarrowPhaseCount,
        queryMetrics.SphereHitCount);
    Master::m_pDebugger->DG_TextValue(
        Tool::U8ToChar(u8"CheckSphere 合計: %.3f ms / 1回平均: %.3f ms"),
        queryMetrics.SphereTotalTimeMs,
        queryMetrics.SphereQueryCount > 0
            ? queryMetrics.SphereTotalTimeMs / queryMetrics.SphereQueryCount
            : 0.0);
    Master::m_pDebugger->DG_TextValue(
        Tool::U8ToChar(u8"CheckSphere直近%zuフレーム  平均: %.3f / 最大: %.3f / p95: %.3f / p99: %.3f ms"),
        m_SphereQueryTimeHistory.size(),
        sphereTimeSummary.AverageMs,
        sphereTimeSummary.MaxMs,
        sphereTimeSummary.P95Ms,
        sphereTimeSummary.P99Ms);
    Master::m_pDebugger->EndDebugWindow();
}


//*---------------------------------------------------------------------------------------
//*【?】衝突したかどうかを確かめる
//*     コライダーごとに処理を分ける 
//*     参考サイト：https://qiita.com/Aqua-218/items/a432cf0410bff57202c5
// 
//* [引数]
//* _colA   : コライダーA
//* _colB   : コライダーB
//* _transA : トランスフォームA
//* _transB : トランスフォームB
//* info    : 衝突情報の保存先
//* [返値]
//* true : 成功
//* false : 失敗
//*----------------------------------------------------------------------------------------
bool CollisionManager::HitCheck(
    std::shared_ptr<Collider> _colA, 
    std::shared_ptr<Collider> _colB, 
    std::shared_ptr<MyTransform> _transA, 
    std::shared_ptr<MyTransform> _transB, 
    CollisionInfo *info
)
{
    COLLIDER_TYPE colA_Type = _colA->get_ColliderType();
    COLLIDER_TYPE colB_Type = _colB->get_ColliderType();

    //=========================================================================================
    //
    //						 ボックス同士
    //
    //=========================================================================================
    if (colA_Type == COLLIDER_TYPE::BOX && colB_Type == COLLIDER_TYPE::BOX)
    {
        // 一旦キャストして実装
        auto boxA = std::static_pointer_cast<BoxCollider>(_colA);
        auto boxB = std::static_pointer_cast<BoxCollider>(_colB);

        // コライダーの位置と、オブジェクトの位置を合わせる。
        VEC3 centerA = boxA->get_Center() + _transA->get_VEC3ToPos();
        VEC3 centerB = boxB->get_Center() + _transB->get_VEC3ToPos();

        // サイズを反映させる
        CollInData_AABB data_A;
        data_A._min = centerA - boxA->get_Size();
        data_A._max = centerA + boxA->get_Size();

        CollInData_AABB data_B;
        data_B._min = centerB - boxB->get_Size();
        data_B._max = centerB + boxB->get_Size();

        // BoxとBoxの判定を行う
        if (HitCheck_BoxVsBox(data_A, data_B))
        {
            // 参考サイト

            // 各軸でのオーバーラップ量を計算
            float overlapX = std::min(data_A._max.x - data_B._min.x, data_B._max.x - data_A._min.x);
            float overlapY = std::min(data_A._max.y - data_B._min.y, data_B._max.y - data_A._min.y);
            float overlapZ = std::min(data_A._max.z - data_B._min.z, data_B._max.z - data_A._min.z);

            VEC3 normal = VEC3();

            if (overlapX <= overlapY && overlapX <= overlapZ)
            {
                info->penetrationDepth = overlapX;
                normal = (centerA.x < centerB.x) ? VEC3(-1.0f, 0.0f, 0.0f) : VEC3(1.0f, 0.0f, 0.0f);
            }
            else if (overlapY <= overlapZ)
            {
                info->penetrationDepth = overlapY;
                normal = (centerA.y < centerB.y) ? VEC3(0.0f, -1.0f, 0.0f) : VEC3(0.0f, 1.0f, 0.0f);
            }
            else
            {
                info->penetrationDepth = overlapZ;
                normal = (centerA.z < centerB.z) ? VEC3(0.0f, 0.0f, -1.0f) : VEC3(0.0f, 0.0f, 1.0f);
            }

            info->hitNormal  = normal;

            return true;
        }
    }
    //=========================================================================================
    //
    //						スフィア同士
    //
    //=========================================================================================
    else if (colA_Type == COLLIDER_TYPE::SPHERE && colB_Type == COLLIDER_TYPE::SPHERE)
    {
        auto sphereA = std::static_pointer_cast<SphereCollider>(_colA);
        auto sphereB = std::static_pointer_cast<SphereCollider>(_colB);

        // コライダーの位置と、オブジェクトの位置を合わせる。
        VEC3 centerA = sphereA->get_Center() + _transA->get_VEC3ToPos();
        VEC3 centerB = sphereB->get_Center() + _transB->get_VEC3ToPos();

        // 中心位置と半径を入れる
        CollInData_Sphere data_A;
        data_A._pos = centerA;
        data_A._radius = sphereA->get_Radius();

        CollInData_Sphere data_B;
        data_B._pos = centerB;
        data_B._radius = sphereB->get_Radius();

        // 球同士の判定
        if (HitCheck_SphereVsSphere(data_A, data_B))
        {
            // 中心座標と半径で最大と最小位置を求める
            VEC3 maxPos_A = data_A._pos + data_A._radius;   
            VEC3 minPos_A = data_A._pos - data_A._radius;
            VEC3 maxPos_B = data_B._pos + data_B._radius;
            VEC3 minPos_B = data_B._pos - data_B._radius;

            // 各軸でのオーバーラップ量を計算
            float overlapX = std::min(maxPos_A.x - minPos_B.x, maxPos_B.x - minPos_A.x);
            float overlapY = std::min(maxPos_A.y - minPos_B.y, maxPos_B.y - minPos_A.y);
            float overlapZ = std::min(maxPos_A.z - minPos_B.z, maxPos_B.z - minPos_A.z);

            VEC3 normal = VEC3();

            if (overlapX <= overlapY && overlapX <= overlapZ)
            {
                info->penetrationDepth  = overlapX;
                normal = (centerA.x < centerB.x) ? VEC3(-1.0f, 0.0f, 0.0f) : VEC3(1.0f, 0.0f, 0.0f);
            }
            else if (overlapY <= overlapZ)
            {
                info->penetrationDepth = overlapY;
                normal = (centerA.y < centerB.y) ? VEC3(0.0f, -1.0f, 0.0f) : VEC3(0.0f, 1.0f, 0.0f);
            }
            else
            {
                info->penetrationDepth = overlapZ;
                normal = (centerA.z < centerB.z) ? VEC3(0.0f, 0.0f, -1.0f) : VEC3(0.0f, 0.0f, 1.0f);
            }

            info->hitNormal = normal;


            return true;
        }
    }
    //=========================================================================================
    //
    //						ボックスとスフィア
    //
    //=========================================================================================
    else if (colA_Type == COLLIDER_TYPE::BOX && colB_Type == COLLIDER_TYPE::SPHERE)
    {
        auto boxA = std::static_pointer_cast<BoxCollider>(_colA);
        auto sphereB = std::static_pointer_cast<SphereCollider>(_colB);

        // コライダーの位置と、オブジェクトの位置を合わせる。
        VEC3 centerA = boxA->get_Center() + _transA->get_VEC3ToPos();
        VEC3 centerB = sphereB->get_Center() + _transB->get_VEC3ToPos();

        // ボックス
        CollInData_AABB data_A;
        data_A._min = centerA - boxA->get_Size();
        data_A._max = centerA + boxA->get_Size();

        // スフィア
        CollInData_Sphere data_B;
        data_B._pos = centerB;
        data_B._radius = sphereB->get_Radius();

        // 球同士の判定
        if (HitCheck_BoxVsSphere(data_A, data_B))
        {
            // 中心座標と半径で最大と最小位置を求める
            // ボックスはそのまま
            VEC3 maxPos_A = data_A._max;
            VEC3 minPos_A = data_A._min;
            VEC3 maxPos_B = data_B._pos + data_B._radius;
            VEC3 minPos_B = data_B._pos - data_B._radius;

            // 各軸でのオーバーラップ量を計算
            float overlapX = std::min(maxPos_A.x - minPos_B.x, maxPos_B.x - minPos_A.x);
            float overlapY = std::min(maxPos_A.y - minPos_B.y, maxPos_B.y - minPos_A.y);
            float overlapZ = std::min(maxPos_A.z - minPos_B.z, maxPos_B.z - minPos_A.z);

            VEC3 normal = VEC3();

            if (overlapX <= overlapY && overlapX <= overlapZ)
            {
                info->penetrationDepth = overlapX;
                normal = (centerA.x < centerB.x) ? VEC3(-1.0f, 0.0f, 0.0f) : VEC3(1.0f, 0.0f, 0.0f);
            }
            else if (overlapY <= overlapZ)
            {
                info->penetrationDepth = overlapY;
                normal = (centerA.y < centerB.y) ? VEC3(0.0f, -1.0f, 0.0f) : VEC3(0.0f, 1.0f, 0.0f);
            }
            else
            {
                info->penetrationDepth = overlapZ;
                normal = (centerA.z < centerB.z) ? VEC3(0.0f, 0.0f, -1.0f) : VEC3(0.0f, 0.0f, 1.0f);
            }

            info->hitNormal = normal;


            return true;
        }
    }
    
    
    
    return false;
}

//*---------------------------------------------------------------------------------------
//*【?】レイキャスト判定
//      衝突したかどうかを確かめる
//*     コライダーごとに処理を分ける 
//*     参考サイト：https://qiita.com/Aqua-218/items/a432cf0410bff57202c5
// 
//* [引数]
//* _collider  : コライダー
//* _transform : トランスフォーム
//* _ray       : レイ情報
//* _outDist   : 衝突したコライダーへの距離の保存
//* info       : 衝突情報の保存先
//* [返値]
//* true : 成功
//* false : 失敗
//*----------------------------------------------------------------------------------------
bool CollisionManager::HitCheck_Raycast(
    std::shared_ptr<class Collider> _collider, 
    std::shared_ptr<class MyTransform> _transform, 
    const CollInData_Ray& _ray, 
    float* _outDistSq,
    CollisionInfo* _outHitInfo)
{
    bool isHit = false;
    COLLIDER_TYPE type = _collider->get_ColliderType();

    switch (type)
    {
    case COLLIDER_TYPE::NONE:
        break;
    case COLLIDER_TYPE::BOX:
    {
        auto boxCollider = std::static_pointer_cast<BoxCollider>(_collider);
        
        // コライダーの位置と、オブジェクトの位置を合わせる。
        VEC3 center = boxCollider->get_Center() + _transform->get_VEC3ToPos();

        // サイズを反映させる
        CollInData_AABB dataAABB;
        dataAABB._min = center - boxCollider->get_Size();
        dataAABB._max = center + boxCollider->get_Size();

        if (HitCheck_BoxVsRay(dataAABB, _ray, _outHitInfo))
        {
			// 衝突したコライダーとオブジェクトを保存
			_outHitInfo->hitCollider = _collider;
            _outHitInfo->hitObject = _collider->get_OwnerObj();

			// 衝突したコライダーへの距離を保存
			*_outDistSq = (_outHitInfo->hitPoint - _ray._point).LengthSq();
            
            isHit = true;
        }
    }
    break;
    case COLLIDER_TYPE::SPHERE:
    {
        CollInData_Sphere sphere;
        if (HitCheck_SphereVsRay(sphere, _ray, _outHitInfo))
        {

        }
    }
    break;
    case COLLIDER_TYPE::RAY:
        break;
    default:
        break;
    }

    return isHit;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//						3D判定
// 
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//*---------------------------------------------------------------------------------------
//*【?】範囲内のオブジェクトを取得する
//*
//* [引数]
//* &_center : 中心座標
//* _radius  : 範囲
//* _mask    : 衝突判定用マスク
//* [返値]
//* 範囲内オブジェクトのリスト 
//*----------------------------------------------------------------------------------------
std::vector<std::shared_ptr<Collider>> CollisionManager::CheckSphere(const VECTOR3::VEC3& _center, float _radius, unsigned _mask)
{
    const auto queryStart = std::chrono::steady_clock::now();
    m_QueryDebugMetrics.SphereQueryCount++;

    std::vector<std::shared_ptr<Collider>> hitList;

    for (auto& col : m_pCollidersList) {
        m_QueryDebugMetrics.SphereColliderScanCount++;

        if (!col->get_IsEnable()) continue;

        // マスクチェック
        if (!(UINT_CAST(col->get_CollisionCategory()) & _mask)) continue;
        if (!col->get_IsActiveOwnerObj())continue;

        // トランスフォーム取得
        auto trans = col->get_OwnerObj().lock()->get_Component<MyTransform>();

        // 相手が球の場合
        if (col->get_ColliderType() == COLLIDER_TYPE::SPHERE) {
            float colRadius = 0.0f;
            auto sphereCol = std::static_pointer_cast<SphereCollider>(col);
            colRadius = sphereCol->get_Radius();

            VEC3 pos = trans->get_VEC3ToPos();
            CollInData_Sphere srcA = { _center,_radius };
            CollInData_Sphere srcB = { pos,    colRadius };

            // スフィアとスフィアで判定
            m_QueryDebugMetrics.SphereNarrowPhaseCount++;
            if (HitCheck_SphereVsSphere(srcA, srcB))
            {
                hitList.push_back(col);
            }

        }

        // ボックスの場合
         else if (col->get_ColliderType() == COLLIDER_TYPE::BOX) {
            auto boxCol = std::static_pointer_cast<BoxCollider>(col);
            // コライダーの位置と、オブジェクトの位置を合わせる。
            VEC3 center = boxCol->get_Center() + trans->get_VEC3ToPos();


            // AABB
            CollInData_AABB srcA;
            srcA._min = center - boxCol->get_Size();
            srcA._max = center + boxCol->get_Size();

            // スフィア
            CollInData_Sphere srcB = { _center,_radius };

            // ボックスとスフィアで判定
            m_QueryDebugMetrics.SphereNarrowPhaseCount++;
            if (HitCheck_BoxVsSphere(srcA, srcB))
            {
                hitList.push_back(col);
            }
        }
         else {
            continue; // その他のコライダータイプはスキップ
		}

    }

    m_QueryDebugMetrics.SphereHitCount += static_cast<int>(hitList.size());
    const auto queryEnd = std::chrono::steady_clock::now();
    m_QueryDebugMetrics.SphereTotalTimeMs += std::chrono::duration<double, std::milli>(
        queryEnd - queryStart).count();

    return hitList;
}

//*---------------------------------------------------------------------------------------
//*【?】レイを飛ばす
//*
//* [引数]
//* &_ray : レイ情報
//* _mask : 衝突マスク
//* &_outHitInfo : 衝突情報格納先
//* [返値]
//* true : 当たった
//* false : 当たってない
//*----------------------------------------------------------------------------------------
bool CollisionManager::CheckRaycast(const CollInData_Ray& _ray, int _mask, PhysicsData::CollisionInfo* _outHitInfo)
{
    const auto queryStart = std::chrono::steady_clock::now();
    m_QueryDebugMetrics.RaycastQueryCount++;

    bool isHit = false;
    float closestDistSq = FLT_MAX;
    CollisionInfo tempHitInfo;

    // 判定ループ処理
    for (int i = 0; i < m_pCollidersList.size(); i++)
    {
        m_QueryDebugMetrics.RaycastColliderScanCount++;

        auto& col = m_pCollidersList[i];

        // 無効なコライダーはスキップ
        if (col == nullptr || col->get_IsEnable() == false || col->get_OwnerObj().expired()) {
            continue;
        }

        // 衝突マスクのチェック
        if ((_mask & UINT_CAST(col->get_CollisionCategory())) == 0)
        {
            continue;
        }

        auto trans = col->get_OwnerObj().lock()->get_Component<MyTransform>();
        if (trans == nullptr) {
            MessageBoxA(NULL, "Aトランスフォームコンポーネントがありません", "衝突判定", MB_OK);
            continue;
        }

        float distance = 0.0f;

        // 衝突チェック
        m_QueryDebugMetrics.RaycastNarrowPhaseCount++;
        if (HitCheck_Raycast(col, trans, _ray, &distance, &tempHitInfo))
        {
            // 最小距離のコライダーを調べる
            if (distance > 0.0f && distance < closestDistSq)
            {
                closestDistSq = distance;
                isHit = true;
                *_outHitInfo = tempHitInfo;  // 衝突情報格納
            }
        }
    }

    if (isHit)
    {
        m_QueryDebugMetrics.RaycastHitCount++;
    }

    const auto queryEnd = std::chrono::steady_clock::now();
    m_QueryDebugMetrics.RaycastTotalTimeMs += std::chrono::duration<double, std::milli>(
        queryEnd - queryStart).count();

    return isHit;
}


//*---------------------------------------------------------------------------------------
//*【?】ボックスとボックスの物理的な判定
//*
//* [引数]
//* &_src : ボックス 
//* &_dst : ボックス
//* [返値]
//* true : 当たった
//* false : 当たってない
//*----------------------------------------------------------------------------------------
bool CollisionManager::HitCheck_BoxVsBox_Physics(const CollInData_AABB &_src, const CollInData_AABB &_dst, CollisionInfo *info)
{
    return true;
}

//*---------------------------------------------------------------------------------------
//*【?】ボックスとボックスの判定
//*
//* [引数]
//* &_src : ボックス 
//* &_dst : ボックス
//* [返値]
//* true : 当たった
//* false : 当たってない
//*----------------------------------------------------------------------------------------
bool CollisionManager::HitCheck_BoxVsBox(const CollInData_AABB &_src, const CollInData_AABB &_dst)
{
    // 左側の判定
    if (_src._max.x < _dst._min.x) return false;

    // 右側の判定
    if (_src._min.x > _dst._max.x) return false;

    // 上側の判定
    if (_src._max.y < _dst._min.y) return false;

    // 下側の判定
    if (_src._min.y > _dst._max.y) return false;

    // 奥の判定
    if (_src._max.z < _dst._min.z) return false;

    // 手前の判定
    if (_src._min.z > _dst._max.z) return false;

    // 当たっている可能性しか残っていない
    return true;
}

// 軸上にOBBを投影した際の半径（長さ）を計算するヘルパー関数
float GetProjectedRadius(const CollInData_OBB& obb, const VEC3& axis)
{
    // 各ローカル軸と分離軸の内積の絶対値にハーフサイズを掛けて足し合わせる
    return  obb._harfLength.x * std::abs(VEC3::Dot(obb._axis[0], axis)) +
            obb._harfLength.y * std::abs(VEC3::Dot(obb._axis[1], axis)) +
            obb._harfLength.z * std::abs(VEC3::Dot(obb._axis[2], axis));
}

//*---------------------------------------------------------------------------------------
//*【?】OBBとOBBの判定
//*
//* [引数]
//* &_src : OBB 
//* &_dst : OBB
//* [返値]
//* true : 当たった
//* false : 当たってない
//*----------------------------------------------------------------------------------------
bool CollisionManager::HitCheck_OBBVsOBB(const CollInData_OBB& _src, const CollInData_OBB& _dst, CollisionInfo* _hitInfo)
{
    float minPenetration = FLT_MAX;
    VEC3 bestAxis = VEC3(0, 0, 0);

    // 15本の分離軸候補をリストアップ
    std::vector<VEC3> axes;
    axes.reserve(15);

    // OBB A の各ローカル軸 (3本)
    for (int i = 0; i < 3; ++i) axes.push_back(_src._axis[i]);
    // OBB B の各ローカル軸 (3本)
    for (int i = 0; i < 3; ++i) axes.push_back(_dst._axis[i]);

    // 外積による軸 (3×3 = 9本)
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            VEC3 crossAxis = VEC3::Cross(_src._axis[i], _dst._axis[j]);
            // 2つの軸が平行な場合、外積がゼロベクトルになるため除外する
            if (crossAxis.LengthSq() > 0.0001f) {
                axes.push_back(crossAxis.Normalize());
            }
        }
    }

    // AからBへの中心間ベクトル
    VEC3 t = _dst._center - _src._center;

    // 全ての軸に対して重なりをチェック
    for (const auto& axis : axes)
    {
        // 各OBBの投影半径を計算
        float rA = GetProjectedRadius(_src, axis);
        float rB = GetProjectedRadius(_dst, axis);

        // 中心間距離を軸に投影
        float distance = std::abs(VEC3::Dot(t, axis));

        // 分離している軸が1本でもあれば衝突していない
        if (distance > rA + rB)
        {
            return false;
        }

        // めり込み量（オーバーラップ）の計算
        float penetration = (rA + rB) - distance;
        if (penetration < minPenetration)
        {
            minPenetration = penetration;
            bestAxis = axis; // 最小のめり込みを起こしている軸が衝突法線となる
        }
    }

    // 法線の向きを「AからBへ向かう方向」に統一する
    if (VEC3::Dot(bestAxis, t) < 0.0f)
    {
        bestAxis = -bestAxis;
    }

    // 既存のシステムに合わせて衝突情報を格納
    _hitInfo->penetrationDepth = minPenetration;
    _hitInfo->hitNormal = bestAxis;

    return true;



    return true;
}

//*---------------------------------------------------------------------------------------
//*【?】ボックスとポイントの判定
//*
//* [引数]
//* &_src : 箱 
//* &_dst : 点
//* [返値]
//* true : 当たった
//* false : 当たってない
//*----------------------------------------------------------------------------------------
bool CollisionManager::HitCheck_BoxVsPoint(const CollInData_AABB& _box, const VECTOR3::VEC3& _p)
{
    // 左側の判定
    if (_box._max.x <= _p.x) return false;

    // 右側の判定
    if (_box._min.x >= _p.x) return false;

    // 上側の判定
    if (_box._max.y <= _p.y) return false;

    // 下側の判定
    if (_box._min.y >= _p.y) return false;

    // 奥の判定
    if (_box._max.z <= _p.z) return false;

    // 手前の判定
    if (_box._min.z >= _p.z) return false;

    // 当たっている可能性しか残っていない
    return true;
}


//*---------------------------------------------------------------------------------------
//*【?】ボックスとスフィアの判定
//* 参考サイト：https://yutateno.hatenablog.jp/entry/2019/11/27/001801 
//*           ：https://developer.mozilla.org/ja/docs/Games/Techniques/3D_collision_detection
//*
//* [引数]
//* &_src : 箱 
//* &_dst : 球
//* [返値]
//* true : 当たった
//* false : 当たってない
//*----------------------------------------------------------------------------------------
bool CollisionManager::HitCheck_BoxVsSphere(const CollInData_AABB &_box, const CollInData_Sphere &_sphere)
{
    // クランプして球の中心からボックスの最も近い点を取得
    VEC3 P;
    P.x = std::max(_box._min.x, std::min(_sphere._pos.x, _box._max.x));
    P.y = std::max(_box._min.y, std::min(_sphere._pos.y, _box._max.y));
    P.z = std::max(_box._min.z, std::min(_sphere._pos.z, _box._max.z));

    float distance = (P.x - _sphere._pos.x) * (P.x - _sphere._pos.x) +
                     (P.y - _sphere._pos.y) * (P.y - _sphere._pos.y) +
                     (P.z - _sphere._pos.z) * (P.z - _sphere._pos.z);

    // 距離が球の半径より小さいなら衝突
    return distance < (_sphere._radius * _sphere._radius);
}


//*---------------------------------------------------------------------------------------
//*【?】スフィアとスフィアの判定
//*
//* [引数]
//* &_src : スフィア 
//* &_dst : スフィア
//* [返値]
//* true : 当たった
//* false : 当たってない
//*----------------------------------------------------------------------------------------
bool CollisionManager::HitCheck_SphereVsSphere(const CollInData_Sphere& _src, const CollInData_Sphere& _dst)
{
    VECTOR3::VEC3 result;
    float dist = VEC3::DistanceSq(_src._pos, _dst._pos);

    float r = _src._radius + _dst._radius;

    if ((r * r) > dist)
    {
        return true;
    }

    return false;
}

//*---------------------------------------------------------------------------------------
//*【?】プレーンとレイの判定
//* 参考サイト：http://www.marupeke296.com/COL_3D_No3_LineToPlane.html
//*
//* [引数]
//* &_plane : 平面
//* &_ray : 光線
//* &_hitInfo : 衝突情報格納用
//* [返値]
//* true : 当たった
//* false : 当たってない
//*----------------------------------------------------------------------------------------
bool CollisionManager::HitCheck_PlaneVsRay(const CollInData_Plane& _plane, const CollInData_Ray& _ray, CollisionInfo* _hitInfo)
{
    // レイの方向と法線の垂直関係を調べる
    float t_Dot1 = VEC3::Dot(_plane._norm, _ray._dir);

    // レイと平面が垂直（平行）に近い場合
    if (abs(t_Dot1) <= 1e-6f) {
        // レイの始点が平面上にあるかどうかを確認
        VEC3 a_minus_q = _ray._point - _plane._point;
        return abs(VEC3::Dot(a_minus_q, _plane._norm)) <= 1e-6f;
    }

    // 平面とレイを結ぶベクトル
    VEC3 v1 = _plane._point - _ray._point;

    // 垂直（0.0f）なら平面とレイが重なっている
    float t_Dot2 = VEC3::Dot(v1, _plane._norm);

    // t が0以上なら進行方向に平面が存在
    float t = t_Dot2 / t_Dot1;

    //平面の背後の場合は当たっていない
    if (t <= 0.0f) {
        return false;
    }
    
    // 衝突点を計算して格納
    _hitInfo->hitPoint = _ray._point + (_ray._dir * t);
    _hitInfo->hitNormal = _plane._norm;

    return true;
}



//*---------------------------------------------------------------------------------------
//*【?】ボックスとレイの判定
//*
//* [引数]
//* &_box : AABB
//* &_ray : 光線
//* &_hitInfo : 衝突情報格納用
//* [返値]
//* true : 当たった
//* false : 当たってない
//*----------------------------------------------------------------------------------------
bool CollisionManager::HitCheck_BoxVsRay(const CollInData_AABB& _box, const CollInData_Ray& _ray, CollisionInfo* _hitInfo)
{
    float t_Min = 0.0f;
    float t_Max = 0.0f;
	VEC3 hitNnormal = VEC3();

    VEC3 minV = _box._min;
    VEC3 maxV = _box._max;
    VEC3 point = _ray._point;
    VEC3 dir = _ray._dir;

    // X軸の判定*************************************************
    float tx_Min = 0.0f;
    float tx_Max = 0.0f;
    // 線分ベクトルが0.0（平行）の場合
    if (std::abs(dir.x) <= 1e-6f)
    {
        // 線分のZ幅がボックスのZ幅に入ってなければ、衝突していない
        if (point.x < minV.x || point.x > maxV.x)
        {
            return false;
        }

        tx_Min = 0.0f;
        tx_Max = 1.0f;
    }
    else {
        // X軸での領域を決定
        float t0 = (minV.x - point.x) / dir.x;
        float t1 = (maxV.x - point.x) / dir.x;
        if (t0 < t1) {
            tx_Min = t0;
            tx_Max = t1;
        }
        else {
            tx_Min = t1;
            tx_Max = t0;
        }
        // X軸領域内ですでに範囲外
        if (tx_Max < 0.0f || tx_Min > 1.0f) {
            return false;
        }
    }

    t_Min = tx_Min;
    t_Max = tx_Max;

	// 方向がマイナスということは、正の方向から入ってきているので、衝突した面の法線は正のX軸方向
    hitNnormal = (dir.x < 0.0f) ? VEC3(1.0f, 0.0f, 0.0f) : VEC3(-1.0f, 0.0f, 0.0f);



    // Y軸の判定*************************************************
    float ty_Min = 0.0f;
    float ty_Max = 0.0f;
    // 線分ベクトルが0.0（平行）の場合
    if (std::abs(dir.y) <= 1e-6f) {

		// 線分のY幅がボックスのY幅に入ってなければ、衝突していない
        if (point.y < minV.y || point.y > maxV.y)
        {
            return false;
        }

        ty_Min = 0.0f;
        ty_Max = 1.0f;
    }
    else {
        // Y軸での領域を決定
        float t0 = (minV.y - point.y) / dir.y;
        float t1 = (maxV.y - point.y) / dir.y;
        if (t0 < t1) {
            ty_Min = t0;
            ty_Max = t1;
        }
        else {
            ty_Min = t1;
            ty_Max = t0;
        }
        // Y軸領域内ですでに範囲外
        if (ty_Max < 0.0f || ty_Min > 1.0f) {
            return false;
        }
    }

    if (t_Max < ty_Min || t_Min > ty_Max)
    {
        return false;
    }

    // Y軸の方が進入が遅い（Y軸の面から箱に入った）
    if (t_Min < ty_Min) {
        t_Min = ty_Min;
		hitNnormal = (dir.y < 0.0f) ? VEC3(0.0f, 1.0f, 0.0f) : VEC3(0.0f, -1.0f, 0.0f);
    }

    if (t_Max > ty_Max)t_Max = ty_Max;



    // Z軸の判定*************************************************
    float tz_Min = 0.0f;
    float tz_Max = 0.0f;
    // 線分ベクトルが0.0の場合
    if (std::abs(dir.z) <= 1e-6f) {
        if (point.z < minV.z || point.z > maxV.z)
        {
            return false;
        }

        tz_Min = 0.0f;
        tz_Max = 1.0f;
    }
    else {
        // Z軸での領域を決定
        float t0 = (minV.z - point.z) / dir.z;
        float t1 = (maxV.z - point.z) / dir.z;
        if (t0 < t1) {
            tz_Min = t0;
            tz_Max = t1;
        }
        else {
            tz_Min = t1;
            tz_Max = t0;
        }
        // Z軸領域内ですでに範囲外
        if (tz_Max < 0.0f || tz_Min > 1.0f) {
            return false;
        }
    }

    if (t_Max < tz_Min || t_Min > tz_Max)
    {
        return false;
    }

    // X軸とY軸の領域が被っていない
    if (t_Min < tz_Min) {
        t_Min = tz_Min;
		hitNnormal = (dir.z < 0.0f) ? VEC3(0.0f, 0.0f, 1.0f) : VEC3(0.0f, 0.0f, -1.0f);
    }
    if (t_Max > tz_Max)t_Max = tz_Max;

    // 共通領域のチェック
    if (t_Min > 1.0f || t_Max < 0.0f) {
        return false;
    }

    _hitInfo->hitPoint = point + dir * t_Min;
    _hitInfo->hitNormal = hitNnormal;

    return true;
}


//*---------------------------------------------------------------------------------------
//*【?】三角形とレイの判定
//* 参考サイト：https://github.com/mitsuba-renderer/mitsuba/blob/450a2b8a258f09ec7e0824861e2306340ccbb3f4/include/mitsuba/core/triangle.h#L109
//*
//* [引数]
//* &_triangle : 三角形
//* &_ray : レイ
//* &_u , &_v : 三角形の面内座標（重心座標）
//* &_t : レイの始点から衝突点までの距離
//* [返値]
//* true : 当たった
//* false : 当たってない
//*----------------------------------------------------------------------------------------
bool CollisionManager::HitCheck_TraiangleVsRay(
    const CollInData_Triangle& _triangle,
    const CollInData_Ray& _ray,
	float& _u, float& _v, float& _t
)
{
    /* Find vectors for two edges sharing */
    VEC3 edge1 = _triangle._v1 - _triangle._v0, edge2 = _triangle._v2 - _triangle._v0;

    /* Begin calculating determinant - also used to calculate U parameter */
    VEC3 pvec = VEC3::Cross(_ray._dir, edge2);

    float det = VEC3::Dot(edge1, pvec);
    if (det == 0) {
        return false;
    }
    float inv_det = 1.0f / det;

    /* Calculate distance from v[0] to ray origin */
    VEC3 tvec = _ray._point - _triangle._v0;

    /* Calculate U parameter and test bounds */
    _u = VEC3::Dot(tvec, pvec) * inv_det;
    if (_u < 0.0 || _u > 1.0) {
        return false;
    }

    /* Prepare to test V parameter */
    VEC3 qvec = VEC3::Cross(tvec, edge1);

    /* Calculate V parameter and test bounds */
    _v = VEC3::Dot(_ray._dir, qvec) * inv_det;

    /* Inverted comparison (to catch NaNs) */
    if (_v >= 0.0 && _u + _v <= 1.0) {
        /* ray intersects triangle -> compute t */
        _t = VEC3::Dot(edge2, qvec) * inv_det;

        return true;
    }

    return false;
}


//*---------------------------------------------------------------------------------------
//*【?】スフィアとレイの判定
//*
//* [引数]
//* &_sphere : スフィア
//* &_ray : 光線
//* &_hitInfo : 衝突情報格納用
//* [返値]
//* true : 当たった
//* false : 当たってない
//*----------------------------------------------------------------------------------------
bool CollisionManager::HitCheck_SphereVsRay(const CollInData_Sphere& _sphere, const CollInData_Ray& _ray, class CollisionInfo* _hitInfo)
{
    return true;
}



//*---------------------------------------------------------------------------------------
//*【?】プレーンと線分の判定
//* 参考サイト：http://www.marupeke296.com/COL_3D_No3_LineToPlane.html
//*
//* [引数]
//* &_plane : 平面
//* &_segment : 線分
//* [返値]
//* true : 当たった
//* false : 当たってない
//*----------------------------------------------------------------------------------------
bool CollisionManager::HitCheck_PlaneVsSegment(const CollInData_Plane& _plane, const CollInData_Segment& _segment)
{
    VEC3 v1 = _segment._start - _plane._point;  
    VEC3 v2 = _segment._end - _plane._point;

    // 鈍角になっていたらマイナスになるはずで、その場合は衝突している
    float t_Dot1 = VEC3::Dot(v1, _plane._norm); // 始点方向と法線の内積
    float t_Dot2 = VEC3::Dot(v2, _plane._norm); // 終点方向と法線の内積

    // マイナスじゃないなら衝突してない
    if ((t_Dot1 * t_Dot2) >= 0.0f)
    {
        return false;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//						2D判定
// 
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//*---------------------------------------------------------------------------------------
//*【?】2Dボックスと2D ポイントの判定
//*
//* [引数]
//* &_box : ボックス 
//* &_p : ポイント
//* [返値]
//* true : 当たった
//* false : 当たってない
//*----------------------------------------------------------------------------------------
bool CollisionManager::HitCheck2D_BoxVsPoint(const CollInData2D_AABB& _box, const VECTOR2::VEC2& _p)
{
    // 左側の判定
    if (_box._max.x < _p.x) return false;

    // 右側の判定
    if (_box._min.x > _p.x) return false;

    // 上側の判定
    if (_box._max.y < _p.y) return false;

    // 下側の判定
    if (_box._min.y > _p.y) return false;

    // 当たっている可能性しか残っていない
    return true;
}








//*---------------------------------------------------------------------------------------
//*【?】最終的な衝突応答を求める
//* （応答がそれぞれ違った場合にどれを優先するかなど）
//*
//* [引数]
//* _responseA : 応答A 
//* _responseB : 応答B
//* [返値]
//* 最終的な衝突応答
//*----------------------------------------------------------------------------------------
COLLISION_RESPONSE CollisionManager::CombineResponse(
    COLLISION_RESPONSE _responseA,
    COLLISION_RESPONSE _responseB)
{
    // どちらかがIgnoreなら判定しない
    if (_responseA == COLLISION_RESPONSE::RESPONSE_IGNORE ||
        _responseB == COLLISION_RESPONSE::RESPONSE_IGNORE)
    {
        return COLLISION_RESPONSE::RESPONSE_IGNORE;
    }

    // どちらかがOverlapなら押し出さない
    if (_responseA == COLLISION_RESPONSE::RESPONSE_OVERLAP ||
        _responseB == COLLISION_RESPONSE::RESPONSE_OVERLAP)
    {
        return COLLISION_RESPONSE::RESPONSE_OVERLAP;
    }

    // 両方Blockの場合のみ物理衝突
    return COLLISION_RESPONSE::RESPONSE_BLOCK;
}
