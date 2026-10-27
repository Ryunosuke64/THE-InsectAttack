#include "pch.h"
#include "Component_SkinnedMeshAnimator.h"
#include "Component_ModelMeshResource.h"
#include "RendererEngine.h"
#include "Component_3DCamera.h"
#include <cmath>

using namespace DirectX;
using namespace VERTEX;
using namespace Tool::UV;

//*---------------------------------------------------------------------------------------
//* @:SkinnedMeshAnimator Class 
//*【?】コンストラクタ
//* 引数：1.オーナーオブジェクト
//* 引数：2.更新レイヤー
//*----------------------------------------------------------------------------------------
SkinnedMeshAnimator::SkinnedMeshAnimator(std::weak_ptr<GameObject> pOwner, int updateRank) : IComponent(pOwner, updateRank),
m_AnimationTime(0.0),
m_CurrentAnimIndex(-1),
m_IsAnimationFlag(false),
m_IsAnimationLODEnabled(true),
m_AnimProcTime(0.0),
m_AnimationLODFrameCounter(0),
m_ShadowAnimProcTime(0.0)/*,
m_ShadowAnimProcTime(0.0)/*,
m_ConstanrBufferBonesData(nullptr)*/
{
    this->set_Tag("SkinnedMeshAnimator");
}


SkinnedMeshAnimator::~SkinnedMeshAnimator()
{

}


//*---------------------------------------------------------------------------------------
//* @:SkinnedMeshAnimator Class 
//*【?】初期化
//* 引数：1.RendererEngine 
//* 返値：void
//*----------------------------------------------------------------------------------------
void SkinnedMeshAnimator::Start(RendererEngine &renderer)
{
    m_NodeList      = m_pMeshResource.lock()->get_ModelData().lock()->get_NodeList();
    m_BoneList      = m_pMeshResource.lock()->get_ModelData().lock()->get_BoneList();
    m_BoneIndexMap  = m_pMeshResource.lock()->get_ModelData().lock()->get_BoneIndexMap();
    m_Animations    = m_pMeshResource.lock()->get_ModelData().lock()->get_Animations();

    //m_ConstanrBufferBonesData = m_pMeshResource.lock()->get_ModelData().lock()->GetConstantBufferBonesData();
}

//*---------------------------------------------------------------------------------------
//* @:SkinnedMeshAnimator Class 
//*【?】更新
//* 引数：1.RendererEngine 
//* 返値：void
//*----------------------------------------------------------------------------------------
void SkinnedMeshAnimator::Update(RendererEngine& renderer)
{
    BoneTransformsUpdate(renderer, m_AnimProcTime, m_CurrentAnimIndex);
}


//*---------------------------------------------------------------------------------------
//* @:SkinnedMeshAnimator Class 
//*【?】更新
//* 引数：1.RendererEngine 
//* 返値：void
//*----------------------------------------------------------------------------------------
void SkinnedMeshAnimator::Draw(RendererEngine &renderer)
{
    //Master::m_pDebugger->BeginDebugWindow(m_pOwner.lock()->get_Tag() + " ---AnimInfo");
    //Master::m_pDebugger->DG_TextValue("BoneNum: %d", m_BoneList.size());    // ボーン数
    //Master::m_pDebugger->DG_TextValue("NodeNum: %d", m_NodeList.size());    // ノード数
    //Master::m_pDebugger->DG_TextValue("AnimNum: %d", m_Animations.size());  // アニメーション数
    //Master::m_pDebugger->DG_CheckBox("IsAnimation##" + m_pOwner.lock()->get_Tag(), &m_IsAnimationFlag);    // アニメーション再生するか
    //Master::m_pDebugger->DG_SliderInt("AnimationIndex##" + m_pOwner.lock()->get_Tag(), 1, &m_CurrentAnimIndex, 0, m_Animations.size() - 1);    // アニメーションインデックス
    //Master::m_pDebugger->EndDebugWindow();

    //// メインパス
    //if (renderer.get_CrntRenderPass() == RENDER_PASS::MAIN)
    //{
    //    //if (m_CurrentAnimIndex == -1)return;
    //    if (m_IsAnimationFlag)
    //    {
    //        m_AnimProcTime += 0.023f;
    //    }


    //    BoneTransformsUpdate(renderer, m_AnimProcTime, m_CurrentAnimIndex);
    //}
    //// シャドウパス
    //else if (renderer.get_CrntRenderPass() == RENDER_PASS::SHADOW)
    //{
    //    //if (m_CurrentAnimIndex == -1)return;
    //    if (m_IsAnimationFlag)
    //    {
    //        m_ShadowAnimProcTime += 0.023f;
    //    }
    //    BoneTransformsUpdate(renderer, m_ShadowAnimProcTime, m_CurrentAnimIndex);
    //}


    auto pContext = renderer.get_DeviceContext();

    //auto bonesData = m_pMeshResource.lock()->get_ModelData().lock()->get_BonesData();

    CB_BONES_DATA bonesData = {};

    // 定数バッファに詰め込む
    for (size_t i = 0; i < MAX_BONES; i++)
    {
        if (m_BoneList.size() <= i)break;
        bonesData.BonesMatrices[i] = m_BoneList[i].FinalTransformation;
    }

    // モデルシェーダに切り替え
    Master::m_pShaderManager->DeviceToSetShader(
        m_pMeshResource.lock()->get_ModelData().lock()->get_ShaderType());

	Master::m_pShaderManager->BindConstantBuffer(CONSTANT_BUFFER_TYPE::BONE, (void*)&bonesData, sizeof(CB_BONES_DATA));

    //// 定数バッファ更新
    //pContext->UpdateSubresource(
    //    m_ConstanrBufferBonesData->pBuff,
    //    0,
    //    nullptr,
    //    &m_ConstanrBufferBonesData->Data,
    //    0,
    //    0
    //);

    //// ボーン変換用の定数バッファの更新＆セット
    //pContext->VSSetConstantBuffers(3, 1, &m_ConstanrBufferBonesData->pBuff);
}


//*---------------------------------------------------------------------------------------
//* @:MeshRenderer Class 
//*【?】アニメーションの時間更新
//*     デルタタイムを掛けたものを入れて！ 
//* 引数：1.時間
//* 返値：void
//*----------------------------------------------------------------------------------------
void SkinnedMeshAnimator::PlayAnim(float _time)
{
    if (m_IsAnimationFlag && !m_IsPlaybackPaused && std::isfinite(_time) && _time >= 0.0f) {
        _time *= m_PlaybackSpeed;
        m_AnimProcTime += _time;

        if (m_IsBlending) {
            m_PrevAnimProcTime += _time; // 前のアニメも内部的に進める
            m_BlendTime += _time;

            // ブレンド終了判定
            if (m_BlendTime >= m_BlendDuration) {
                m_IsBlending = false;
            }
        }
    }
}



//*---------------------------------------------------------------------------------------
//* @:MeshRenderer Class 
//*【?】IMeshResource参照用のポインタ設定
//* 引数：1.IMeshResource
//* 返値：void
//*----------------------------------------------------------------------------------------
void SkinnedMeshAnimator::set_MeshResource(std::weak_ptr<class ModelMeshResource> meshResource)
{
    m_pMeshResource = meshResource;
}


//*---------------------------------------------------------------------------------------
//* @:SkinnedMeshAnimator Class 
//*【?】ボーンの更新
// 
//* 引数：1.RendererEngine
//* 引数：2.秒単位の時間
//* 返値：void
//*----------------------------------------------------------------------------------------
void SkinnedMeshAnimator::BoneTransformsUpdate(RendererEngine &renderer, float timeInSeconds, int animIdx)
{
    if (m_NodeList.empty())
    {
        return;
    }
    if (animIdx < 0 || static_cast<size_t>(animIdx) >= m_Animations.size() ||
        !m_Animations[animIdx])
    {
        animIdx = -1;
    }

    float tickPerSecond = 0.0f;
    float timeInTicks = 0.0f;
    float animTimeTicks = 0.0f;

    std::shared_ptr<Camera3D> camera;
    if (camera = Master::m_pDataManager->get_CameraComponent().lock())
    {
        m_AimPitchAngle = camera->get_Angle_V();
        m_AimYawAngle = camera->get_Angle_H();
    }

    //// 範囲外チェック
    //if (animIdx > m_Animations.size()) {
    //    OutputDebugString("アニメーションインデックスが範囲外です。");
    //    return;
    //}

    // アニメーションがあるなら ***********************************
    if (!m_Animations.empty() && animIdx != -1) {
        tickPerSecond =
            static_cast<float>(
                m_Animations[animIdx]->TicksPerSecond != 0 ?     // ティック数が0じゃないならそのまま代入
                m_Animations[animIdx]->TicksPerSecond : 25.0f    // 0なら25.0を仮として入れる
                );

        // ティック単位の時間を求める
        /*  ティック単位時間 ＝ 秒単位時間 ｘ ティック数 */
        timeInTicks = timeInSeconds * tickPerSecond;


        // アニメーションの現在の再生時間を剰余演算で求める
        /* 現在の再生時間 ＝ ティック単位時間 ％ アニメーションの持続時間 */
        if (m_Animations[animIdx]->Duration > 0.0)
        {
            animTimeTicks = static_cast<float>(fmod(timeInTicks, m_Animations[animIdx]->Duration));
        }
    }

    VEC3 cameraPos = Master::m_pDataManager->get_CameraPos();
    VEC3 myPos = m_pOwner.lock()->get_TransformConst()->get_VEC3ToPos();

    float distanceFormCameraSq = VEC3::DistanceSq(cameraPos, myPos);
    int animationLODFrame = 16;
    const float ANIMATION_LOD_DISTANCE_NEAR     = 0.0f;
    const float ANIMATION_LOD_DISTANCE_MIDDLE   = 100.0f;
    const float ANIMATION_LOD_DISTANCE_LONG     = 200.0f;
    const float ANIMATION_LOD_DISTANCESQ_NEAR = ANIMATION_LOD_DISTANCE_NEAR * ANIMATION_LOD_DISTANCE_NEAR;
    const float ANIMATION_LOD_DISTANCESQ_MIDDLE = ANIMATION_LOD_DISTANCE_MIDDLE * ANIMATION_LOD_DISTANCE_MIDDLE;
    const float ANIMATION_LOD_DISTANCESQ_LONG = ANIMATION_LOD_DISTANCE_LONG * ANIMATION_LOD_DISTANCE_LONG;
    m_AnimationLODFrameCounter++;
    if (m_IsAnimationLODEnabled)
    {
        //
        // 遠距離
        //
        if (distanceFormCameraSq > ANIMATION_LOD_DISTANCESQ_LONG)
        {
            animationLODFrame = 16;
        }
        //
        // 中距離
        //
        else if (distanceFormCameraSq > ANIMATION_LOD_DISTANCESQ_MIDDLE)
        {
            animationLODFrame = 8;
        }
        //
        // 近距離
        //
        else if (distanceFormCameraSq > ANIMATION_LOD_DISTANCESQ_NEAR)
        {
            animationLODFrame = 0;
        }
    }
    else
    {
        // アニメーションLODなし
        animationLODFrame = 0;
    }

    if (m_AnimationLODFrameCounter >= animationLODFrame)
    {
        // ボーン変換
        TransformBone(animTimeTicks, 0, XMMatrixIdentity(), animIdx);

        m_AnimationLODFrameCounter = 0;
    }

}


//*---------------------------------------------------------------------------------------
//* @:SkinnedMeshAnimator Class 
//*【?】ボーン変換行列の計算    ※再帰関数
// 
//* 引数：1.アニメーション単位のティック数
//* 引数：2.処理するノードインデックス
//* 引数：3.親ボーンのワールド変換行列
//* 返値：void
//*----------------------------------------------------------------------------------------
void SkinnedMeshAnimator::TransformBone(float animTimeTicks, UINT nodeIdx, const DirectX::XMMATRIX &parent, int animIdx)
{
    XMMATRIX localTransformMtx = XMMatrixIdentity(); // ローカル行列
    XMMATRIX worldTransformMtx = XMMatrixIdentity(); // ボーンの最終ワールド座標変換行列
    localTransformMtx = m_NodeList[nodeIdx]->Transformation; // 初期姿勢で初期化
    std::string nodeName = m_NodeList[nodeIdx]->Name;

    if (m_IsAnimationFlag && animIdx >= 0)
    {
        // === 1. 現在のアニメーションの SRT を計算 ===
        const AnimationData* pAnimCrnt = m_Animations[animIdx];
        const NodeAnimChannel* pNodeCrnt = FindNodeAnim(pAnimCrnt, nodeName);

        XMVECTOR S_crnt, R_crnt, T_crnt;
        bool hasCrntAnim = false;

        if (pNodeCrnt) {
            S_crnt = CalcInterpolatedScalingVec(animTimeTicks, pNodeCrnt);
            R_crnt = CalcInterpolatedRotationQuat(animTimeTicks, pNodeCrnt);
            T_crnt = CalcInterpolatedPositionVec(animTimeTicks, pNodeCrnt);
            hasCrntAnim = true;
            localTransformMtx = XMMatrixScalingFromVector(S_crnt) * XMMatrixRotationQuaternion(R_crnt) * XMMatrixTranslationFromVector(T_crnt);
        }

        // === 2. ブレンド処理 ===
        if (m_IsBlending && m_BlendDuration > 0.0f && m_PrevAnimIndex >= 0 &&
            static_cast<size_t>(m_PrevAnimIndex) < m_Animations.size() &&
            m_Animations[m_PrevAnimIndex] && m_Animations[m_PrevAnimIndex]->Duration > 0.0)
        {
            const AnimationData* pAnimPrev = m_Animations[m_PrevAnimIndex];
            const NodeAnimChannel* pNodePrev = FindNodeAnim(pAnimPrev, nodeName);

            if (pNodePrev && hasCrntAnim) {
                // 前のアニメーションの Tick を計算 (PlayAnimで進めた時間をTick変換)
                float tickPerSecPrev = pAnimPrev->TicksPerSecond != 0 ? pAnimPrev->TicksPerSecond : 25.0f;
                float prevAnimTimeTicks = fmod(m_PrevAnimProcTime * tickPerSecPrev, pAnimPrev->Duration);

                XMVECTOR S_prev = CalcInterpolatedScalingVec(prevAnimTimeTicks, pNodePrev);
                XMVECTOR R_prev = CalcInterpolatedRotationQuat(prevAnimTimeTicks, pNodePrev);
                XMVECTOR T_prev = CalcInterpolatedPositionVec(prevAnimTimeTicks, pNodePrev);

                // ブレンド率 (0.0 ～ 1.0)
                float blendFactor = std::clamp(m_BlendTime / m_BlendDuration, 0.0f, 1.0f);

                // 各要素をブレンド (Slerp と Lerp)
                XMVECTOR S_blend = XMVectorLerp(S_prev, S_crnt, blendFactor);
                XMVECTOR R_blend = XMQuaternionSlerp(R_prev, R_crnt, blendFactor);
                XMVECTOR T_blend = XMVectorLerp(T_prev, T_crnt, blendFactor);

                // ブレンドされた SRT からローカル行列を作成
                localTransformMtx = XMMatrixScalingFromVector(S_blend) * XMMatrixRotationQuaternion(R_blend) * XMMatrixTranslationFromVector(T_blend);
            }
        }

    }


    // =====================================================================
    // 上半身のプロシージャル回転（エイムの上下対応）
    // =====================================================================
    // 曲げたいボーンの名前を指定
    if (/*nodeName == "mixamorig:LeftShoulder" || nodeName == "mixamorig:RightShoulder" ||*//*nodeName == "mixamorig:Spine" || nodeName == "mixamorig:Spine1" || nodeName == "mixamorig:Spine2" || */nodeName == "WeaponSocket_Right")
    {
        // カメラのピッチ角（上下角）を取得する
        float pitchAngle = -m_AimPitchAngle;
        float yawAngle = -m_AimYawAngle;

        // 複数のボーンで角度を分散させる（1つのボーンだけ曲げると体が折れるため）
        // 例: Spine1で30%、Spine2で30%、Chestで40% など
        if (nodeName == "mixamorig:Spine") { pitchAngle *= 0.1f;  yawAngle *= 0.4f; }
        if (nodeName == "mixamorig:Spine1") { pitchAngle *= 0.1f; yawAngle *= 0.3f; }
        if (nodeName == "mixamorig:Spine2") { pitchAngle *= 0.1f; yawAngle *= 0.3f; }
        // TODO:一旦ソケットも直接回転させる
        if (nodeName == "WeaponSocket_Right") { pitchAngle *= -1.0f; }


        if (nodeName == "mixamorig:LeftShoulder") { localTransformMtx = m_NodeList[nodeIdx]->Transformation; }
        if (nodeName == "mixamorig:RightShoulder") { localTransformMtx = m_NodeList[nodeIdx]->Transformation; }


        // 左右制限
        //if(yawAngle > FLOAT_CAST(M_PI_4) || yawAngle < FLOAT_CAST(M_PI_4))
        //{
        //    yawAngle = std::clamp(yawAngle, -FLOAT_CAST(M_PI_4), FLOAT_CAST(M_PI_4));
        //}

        // 追加の回転行列を作成する
        XMMATRIX aimPitchRotMtx = DirectX::XMMatrixRotationX(pitchAngle);
        XMMATRIX aimYawRotMtx = DirectX::XMMatrixRotationY(yawAngle);

        // ローカル行列に追加の回転を合成する
        localTransformMtx = aimPitchRotMtx * localTransformMtx;
    }


    /* ローカル変換 x 親のワールド行列 （サイトによって掛ける順番が変わったりする）*/
    // 親の変換行列をかけて、ワールド空間に配置するための変換行列を求める
    worldTransformMtx = localTransformMtx * parent;


    // ボーンの最終行列を計算
    if (m_BoneIndexMap.find(nodeName) != m_BoneIndexMap.end())
    {
        UINT boneIndex = m_BoneIndexMap[nodeName];

        // 
        /* スキニング用変換行列　＝　オフセット行列（初期姿勢の逆行列）ｘ ワールド行列 */
        //
        XMMATRIX mat = m_BoneList[boneIndex].OffsetMtx * worldTransformMtx;
        XMStoreFloat4x4(&m_BoneList[boneIndex].FinalTransformation, mat);   // 4*4に格納


        // デバッグ用
        m_BoneList[boneIndex].DebugWorldMatrix = worldTransformMtx;
    }

    // 再帰処理で子ノードも変換させる
    for (size_t i = 0; i < m_NodeList[nodeIdx]->ChildrenIndices.size(); i++)
    {
        UINT childIndex = m_NodeList[nodeIdx]->ChildrenIndices[i];
        TransformBone(animTimeTicks, childIndex, worldTransformMtx, animIdx);
    }
}


//*---------------------------------------------------------------------------------------
//* @:SkinnedMeshAnimator Class 
//*【?】指定のアニメーション情報があるか確認する
// 
//* 引数：1.AnimationData
//* 引数：2.確認するノード名
//* 返値：NodeAnimChannel*
//*----------------------------------------------------------------------------------------
const NodeAnimChannel *SkinnedMeshAnimator::FindNodeAnim(const AnimationData *pAnim, const std::string &nodeName)
{
    for (size_t i = 0; i < pAnim->Channels.size(); i++) {
        const NodeAnimChannel *pNodeAnim = &pAnim->Channels[i];
        if (pNodeAnim->NodeName == nodeName)
        {
            return pNodeAnim;
        }
    }
    return nullptr;
}


//*---------------------------------------------------------------------------------------
//* @:SkinnedMeshAnimator Class 
//*【?】対応するキーインデックスを探す 
//      ・Position
//      ・Rotation
//      ・Scaling
// 
//* 引数：1.ティック単位アニメーション時間
//* 引数：2.確認するノードに含まれるアニメーション情報
//* 返値：UINT インデックス
//*----------------------------------------------------------------------------------------
/* Positionのキーフレームインデックスを探す */
UINT SkinnedMeshAnimator::FindPosition(float animTimeTicks, const NodeAnimChannel *pNodeAnimChannel)
{
    assert(pNodeAnimChannel->PositionKeys.size() > 0);

    for (UINT i = 0; i < pNodeAnimChannel->PositionKeys.size() - 1; i++)
    {
        double t = pNodeAnimChannel->PositionKeys[i + 1].Time;
        if (animTimeTicks < t) {
            return i;
        }
    }
    return 0;
}

/* Rotationのキーフレームインデックスを探す */
UINT SkinnedMeshAnimator::FindRotation(float animTimeTicks, const NodeAnimChannel *pNodeAnimChannel)
{
    assert(pNodeAnimChannel->RotationKeys.size() > 0);

    for (UINT i = 0; i < pNodeAnimChannel->RotationKeys.size() - 1; i++)
    {
        double t = pNodeAnimChannel->RotationKeys[i + 1].Time;
        if (animTimeTicks < t) {
            return i;
        }
    }
    return 0;
}

/* Scalingのキーフレームインデックスを探す */
UINT SkinnedMeshAnimator::FindScaling(float animTimeTicks, const NodeAnimChannel *pNodeAnimChannel)
{
    assert(pNodeAnimChannel->ScalingKeys.size() > 0);

    for (UINT i = 0; i < pNodeAnimChannel->ScalingKeys.size() - 1; i++)
    {
        double t = pNodeAnimChannel->ScalingKeys[i + 1].Time;
        if (animTimeTicks < t) {
            return i;
        }
    }
    return 0;
}


//*---------------------------------------------------------------------------------------
//* @:SkinnedMeshAnimator Class 
//*【?】現在のアニメーション時間からキーフレーム同士の補間をする
//      ・Position
//      ・Rotation
//      ・Scaling
// 
//* 引数：1.行列出力用の参照
//* 引数：2.ティック単位アニメーション時間
//* 引数：3.確認するノードに含まれるアニメーション情報
//*----------------------------------------------------------------------------------------
/* 位置のキーフレーム補間 */
void SkinnedMeshAnimator::CalcInterpolatedPosition(DirectX::XMMATRIX &out, float animTimeTicks, const NodeAnimChannel *pNodeAnimChannel)
{
    if ((pNodeAnimChannel->PositionKeys.size() < 2)) {
        VEC3 res = pNodeAnimChannel->PositionKeys[0].Value;
        out = XMMatrixTranslation(res.x, res.y, res.z);
        return;
    }

    VEC3 startPos{};
    VEC3 endPos{};
    UINT posIndex = FindPosition(animTimeTicks, pNodeAnimChannel);
    UINT nextPosIndex = posIndex + 1;
    startPos = pNodeAnimChannel->PositionKeys[posIndex].Value;          // 現在位置
    endPos = pNodeAnimChannel->PositionKeys[nextPosIndex].Value;      // 次の位置
    double t1 = pNodeAnimChannel->PositionKeys[posIndex].Time;
    double t2 = pNodeAnimChannel->PositionKeys[nextPosIndex].Time;
    double delta = t2 - t1;
    double factor = (animTimeTicks - t1) / delta;
    assert(factor >= 0.0f && factor <= 1.0f);

    // 線形補間
    VEC3 tempVec = VEC3::Lerp(startPos, endPos, static_cast<float>(factor));

    //平行移動行列を作成する
    out = XMMatrixTranslation(tempVec.x, tempVec.y, tempVec.z);
}

/* 回転のキーフレーム補間 */
void SkinnedMeshAnimator::CalcInterpolatedRotation(DirectX::XMMATRIX &out, float animTimeTicks, const NodeAnimChannel *pNodeAnimChannel)
{
    if ((pNodeAnimChannel->RotationKeys.size() < 2)) {
        aiQuaternion res = pNodeAnimChannel->RotationKeys[0].Value;
        XMVECTOR q = XMVectorSet(res.x, res.y, res.z, res.w);        // XMVECTORへ変換
        out = XMMatrixRotationQuaternion(q);        // 回転行列の作成
        return;
    }

    aiQuaternion startRot{};
    aiQuaternion endRot{};
    UINT rotIndex = FindRotation(animTimeTicks, pNodeAnimChannel);
    UINT nextRotIndex = rotIndex + 1;
    startRot = pNodeAnimChannel->RotationKeys[rotIndex].Value;      // 現在位置
    endRot = pNodeAnimChannel->RotationKeys[nextRotIndex].Value;    // 次の位置
    double t1 = pNodeAnimChannel->RotationKeys[rotIndex].Time;
    double t2 = pNodeAnimChannel->RotationKeys[nextRotIndex].Time;
    double delta = t2 - t1;
    double factor = (animTimeTicks - t1) / delta;
    assert(factor >= 0.0f && factor <= 1.0f);

    aiQuaternion res{};
    aiQuaternion::Interpolate(res, startRot, endRot, static_cast<float>(factor));
    res.Normalize();

    // XMVECTORへ変換
    XMVECTOR q = XMVectorSet(res.x, res.y, res.z, res.w);

    // 回転行列の作成
    out = XMMatrixRotationQuaternion(q);
}

/* スケールのキーフレーム補間 */
void SkinnedMeshAnimator::CalcInterpolatedScaling(DirectX::XMMATRIX &out, float animTimeTicks, const NodeAnimChannel *pNodeAnimChannel)
{
    if ((pNodeAnimChannel->ScalingKeys.size() < 2)) {
        VEC3 res = pNodeAnimChannel->ScalingKeys[0].Value;
        out = XMMatrixScaling(res.x, res.y, res.z);
        return;
    }

    VEC3 startScl{};
    VEC3 endScl{};
    UINT sclIndex = FindScaling(animTimeTicks, pNodeAnimChannel);
    UINT nextsclIndex = sclIndex + 1;
    startScl = pNodeAnimChannel->ScalingKeys[sclIndex].Value;      // 現在位置
    endScl = pNodeAnimChannel->ScalingKeys[nextsclIndex].Value;    // 次の位置
    double t1 = pNodeAnimChannel->ScalingKeys[sclIndex].Time;
    double t2 = pNodeAnimChannel->ScalingKeys[nextsclIndex].Time;
    double delta = t2 - t1;
    double factor = (animTimeTicks - t1) / delta;
    assert(factor >= 0.0f && factor <= 1.0f);

    // 線形補間
    VEC3 tempVec = VEC3::Lerp(startScl, endScl, static_cast<float>(factor));

    //スケール行列を作成する
    out = XMMatrixScaling(tempVec.x, tempVec.y, tempVec.z);
}

DirectX::XMVECTOR SkinnedMeshAnimator::CalcInterpolatedScalingVec(float animTimeTicks, const NodeAnimChannel* pNodeAnim)
{
    if (pNodeAnim->ScalingKeys.size() < 2) {
        return XMVectorSet(pNodeAnim->ScalingKeys[0].Value.x, pNodeAnim->ScalingKeys[0].Value.y, pNodeAnim->ScalingKeys[0].Value.z, 0.0f);
    }
    VEC3 startScl{};
    VEC3 endScl{};
    UINT sclIndex = FindScaling(animTimeTicks, pNodeAnim);
    UINT nextSclIndex = sclIndex + 1;
    startScl = pNodeAnim->ScalingKeys[sclIndex].Value;      // 現在位置
    endScl = pNodeAnim->ScalingKeys[nextSclIndex].Value;    // 次の位置
    double t1 = pNodeAnim->ScalingKeys[sclIndex].Time;
    double t2 = pNodeAnim->ScalingKeys[nextSclIndex].Time;
    double delta = t2 - t1;
    double factor = (animTimeTicks - t1) / delta;
    assert(factor >= 0.0f && factor <= 1.0f);

    // 線形補間
    VEC3 tempVec = VEC3::Lerp(startScl, endScl, static_cast<float>(factor));

    return XMVectorSet(tempVec.x, tempVec.y, tempVec.z, 0.0f); 
}
DirectX::XMVECTOR SkinnedMeshAnimator::CalcInterpolatedRotationQuat(float animTimeTicks, const NodeAnimChannel* pNodeAnim)
{
    if (pNodeAnim->RotationKeys.size() < 2) {
        return XMVectorSet(pNodeAnim->RotationKeys[0].Value.x, pNodeAnim->RotationKeys[0].Value.y, pNodeAnim->RotationKeys[0].Value.z, pNodeAnim->RotationKeys[0].Value.w);
    }
    aiQuaternion startRot{};
    aiQuaternion endRot{};
    UINT rotIndex = FindRotation(animTimeTicks, pNodeAnim);
    UINT nextRotIndex = rotIndex + 1;
    startRot = pNodeAnim->RotationKeys[rotIndex].Value;      // 現在位置
    endRot = pNodeAnim->RotationKeys[nextRotIndex].Value;    // 次の位置
    double t1 = pNodeAnim->RotationKeys[rotIndex].Time;
    double t2 = pNodeAnim->RotationKeys[nextRotIndex].Time;
    double delta = t2 - t1;
    double factor = (animTimeTicks - t1) / delta;
    assert(factor >= 0.0f && factor <= 1.0f);

    aiQuaternion res;
    aiQuaternion::Interpolate(res, startRot, endRot, static_cast<float>(factor));
    res.Normalize();
    return XMVectorSet(res.x, res.y, res.z, res.w); // QuaternionとしてXMVECTORを返す
}
DirectX::XMVECTOR SkinnedMeshAnimator::CalcInterpolatedPositionVec(float animTimeTicks, const NodeAnimChannel* pNodeAnim)
{
    if (pNodeAnim->PositionKeys.size() < 2) {
        return XMVectorSet(pNodeAnim->PositionKeys[0].Value.x, pNodeAnim->PositionKeys[0].Value.y, pNodeAnim->PositionKeys[0].Value.z, 0.0f);
    }
    VEC3 startScl{};
    VEC3 endScl{};
    UINT sclIndex = FindPosition(animTimeTicks, pNodeAnim);
    UINT nextSclIndex = sclIndex + 1;
    startScl = pNodeAnim->PositionKeys[sclIndex].Value;      // 現在位置
    endScl = pNodeAnim->PositionKeys[nextSclIndex].Value;    // 次の位置
    double t1 = pNodeAnim->PositionKeys[sclIndex].Time;
    double t2 = pNodeAnim->PositionKeys[nextSclIndex].Time;
    double delta = t2 - t1;
    double factor = (animTimeTicks - t1) / delta;
    assert(factor >= 0.0f && factor <= 1.0f);

    // 線形補間
    VEC3 tempVec = VEC3::Lerp(startScl, endScl, static_cast<float>(factor));

    return XMVectorSet(tempVec.x, tempVec.y, tempVec.z, 0.0f);
}

void SkinnedMeshAnimator::CrossFadeAnim(int newAnimIndex, float blendDurationInSeconds)
{
    if (newAnimIndex < 0 || static_cast<size_t>(newAnimIndex) >= m_Animations.size() ||
        !m_Animations[newAnimIndex] || m_CurrentAnimIndex == newAnimIndex)
    {
        return;
    }

    m_PrevAnimIndex = m_CurrentAnimIndex;
    m_PrevAnimProcTime = m_AnimProcTime;
    m_CurrentAnimIndex = newAnimIndex;
    m_AnimationTime = 0.0;
    m_AnimProcTime = 0.0f;
    m_ShadowAnimProcTime = 0.0f;
    m_BlendTime = 0.0f;
    m_BlendDuration = std::isfinite(blendDurationInSeconds) ?
        std::max(0.0f, blendDurationInSeconds) : 0.0f;
    m_IsBlending = m_BlendDuration > 0.0f && m_PrevAnimIndex >= 0 &&
        static_cast<size_t>(m_PrevAnimIndex) < m_Animations.size() &&
        m_Animations[m_PrevAnimIndex] && m_Animations[m_PrevAnimIndex]->Duration > 0.0;
}
//*---------------------------------------------------------------------------------------
//*【?】アニメーションの切り替え
//*
//* [引数] 
//* newAnimIndex : アニメーションインデックス 
//* blendDurationInSeconds : ブレンド時間（ブレンドさせる場合）
//* 
//* [返値] なし 
//*----------------------------------------------------------------------------------------
void SkinnedMeshAnimator::ChangeAnim(int newAnimIndex, float blendDurationInSeconds)
{
    // Selection is also allowed while animation evaluation is disabled.
    CrossFadeAnim(newAnimIndex, blendDurationInSeconds);
}

void SkinnedMeshAnimator::set_PlaybackSpeed(float speed)
{
    if (std::isfinite(speed))
    {
        m_PlaybackSpeed = std::clamp(speed, 0.0f, 10.0f);
    }
}

float SkinnedMeshAnimator::get_AnimDurationSeconds(int animIndex)const
{
    if (animIndex < 0 || static_cast<size_t>(animIndex) >= m_Animations.size())
    {
        return 0.0f;
    }

    const auto* anim = m_Animations[animIndex];
    if (!anim || anim->Duration <= 0.0)
    {
        return 0.0f;
    }

    const double ticksPerSecond = anim->TicksPerSecond != 0.0 ? anim->TicksPerSecond : 25.0;
    return static_cast<float>(anim->Duration / ticksPerSecond);
}

void SkinnedMeshAnimator::SeekAnim(float timeInSeconds)
{
    if (!std::isfinite(timeInSeconds))
    {
        return;
    }

    const float duration = get_AnimDurationSeconds(m_CurrentAnimIndex);
    m_AnimProcTime = duration > 0.0f ? std::fmod(std::max(0.0f, timeInSeconds), duration) : 0.0f;
    m_ShadowAnimProcTime = m_AnimProcTime;
    m_AnimationTime = 0.0;
    m_PrevAnimProcTime = 0.0f;
    m_IsBlending = false;
    m_BlendTime = 0.0f;
}
//*---------------------------------------------------------------------------------------
//*【?】ノード名からボーンのワールド変換行列を取得する
//*
//* [引数]
//* _nodeName : ノード名
//*
//* [返値]
//* ボーンのワールド変換行列 
//*----------------------------------------------------------------------------------------
XMMATRIX& SkinnedMeshAnimator::get_BoneLocalWorldMatrix(const std::string& _nodeName)
{
    int index = m_BoneIndexMap[_nodeName];
    return m_BoneList[index].DebugWorldMatrix;  // Final...の方だとXMFLOAT4X4になってしまってるので
}

//*---------------------------------------------------------------------------------------
//*【?】ボーン行列を計算する
//*
//* [引数]
//* & renderer : 描画エンジン
//*
//* [返値]
//* なし 
//*----------------------------------------------------------------------------------------
void SkinnedMeshAnimator::RefreshPose(RendererEngine& renderer)
{
    m_AnimationLODFrameCounter = 16;
    BoneTransformsUpdate(
        renderer,
        m_AnimProcTime,
        m_CurrentAnimIndex);
}

//*---------------------------------------------------------------------------------------
//*【?】アニメーションが終了しているか
//*
//* [引数]
//* [返値]
//* 終了しているか 
//*----------------------------------------------------------------------------------------
bool SkinnedMeshAnimator::get_IsCrntAnimationHasFinished()const
{
    // 未選択・範囲外なら終了扱いにしない
    if (m_CurrentAnimIndex < 0 ||
        static_cast<size_t>(m_CurrentAnimIndex) >= m_Animations.size())
    {
        return false;
    }

    const auto* anim = m_Animations[m_CurrentAnimIndex];
    if (!anim || anim->Duration <= 0.0)
    {
        return false;
    }

    // BoneTransformsUpdate と同じ変換ルールを使う
    const double ticksPerSecond =
        anim->TicksPerSecond != 0.0 ? anim->TicksPerSecond : 25.0;

    const double elapsedTicks =
        static_cast<double>(m_AnimProcTime) * ticksPerSecond;

    return elapsedTicks >= anim->Duration;
}