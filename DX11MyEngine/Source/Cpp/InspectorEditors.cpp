#include "pch.h"
#include "InspectorEditors.h"
#include "RendererEngine.h"
#include "GameObject.h"
#include "Component_Transform.h"
#include "Component_RectTransform.h"
#include "Component_DirectionalLight.h"
#include "Component_PointLight.h"
#include "Component_SpotLight.h"
#include "Component_PlayerController.h"
#include "Component_3DCamera.h"
#include "Component_SkinnedMeshAnimator.h"
#include "Component_ModelMeshResource.h"
#include "Component_IMeshResource.h"
#include "Component_ModelMeshRenderer.h"
#include "Component_BoxCollider.h"
#include "Component_SphereCollider.h"
#include "Component_TrailRenderer.h"
#include "Component_LineRenderer.h"
#include "Component_Physics.h"

using namespace VECTOR2;
using namespace VECTOR3;
using namespace VECTOR4;
using namespace UtilityData;
using namespace Tool;

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:TransformEditor Class --- */
//
// ***************************************************************************************
bool TransformEditor::Init(RendererEngine &renderer)
{
    return true;
}

void TransformEditor::OnEditorGUI(RendererEngine &renderer, GameObject &pObj)
{
    auto pTransform = pObj.get_Component<MyTransform>();

    if (pTransform == nullptr)
    {
        return;
    }

    VEC3 pos = pTransform->get_VEC3ToPos();
    VEC3 rot = pTransform->get_VEC3ToRotateToDeg();
    VEC3 scl = pTransform->get_VEC3ToScale();
    VEC3 pos_L = pTransform->get_VEC3ToLocal_Pos();
    VEC3 rot_L = pTransform->get_VEC3ToLocal_RotateToDeg();
    VEC3 scl_L = pTransform->get_VEC3ToLocal_Scale();

    // beginはInspectorWindowで行っている


    // ノード
    if (Master::m_pDebugger->DG_TreeNode(U8ToChar(u8"トランスフォーム")))
    {
        Master::m_pDebugger->DG_Text(U8ToChar(u8"ワールド/ローカル"));

        
        Master::m_pDebugger->DG_Separator();    // 区切り線
        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"座標"));
        if (Master::m_pDebugger->DG_DragVec3("##Pos", &pos, m_SlideAccuRate, -100000.0f, 100000.0f))
        {
            pTransform->set_Pos(pos);
        }
        if (Master::m_pDebugger->DG_DragVec3("##PosL", &pos_L, m_SlideAccuRate, -100000.0f, 100000.0f))
        {
            pTransform->set_VEC3ToLocal_Pos(pos_L);
        }


        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"回転"));
        if (Master::m_pDebugger->DG_DragVec3("##Rot", &rot, m_SlideAccuRate, -180.0f, 180.0f))
        {
            pTransform->set_RotateToDeg(rot);
        }
        if (Master::m_pDebugger->DG_DragVec3("##RotL", &rot_L, m_SlideAccuRate, -180.0f, 180.0f))
        {
            pTransform->set_VEC3ToLocal_RotateToDeg(rot_L);
        }


        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"拡縮")); 
        if (Master::m_pDebugger->DG_DragVec3("##Scale", &scl, m_SlideAccuRate, -100000.0f, 100000.0f))
        {
            pTransform->set_Scale(scl);
        }
        if (Master::m_pDebugger->DG_DragVec3("##ScaleL", &scl_L, m_SlideAccuRate, -100000.0f, 100000.0f))
        {
            pTransform->set_VEC3ToLocal_Scale(scl_L);
        }


        Master::m_pDebugger->DG_SliderFloat(U8ToChar(u8"編集スライド精度"), 1, &m_SlideAccuRate, 0.001f, 2.0f);

        Master::m_pDebugger->DG_TreePop();
    }
}



// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:RectTransformEditor Class --- */
//
// ***************************************************************************************
bool RectTransformEditor::Init(RendererEngine &renderer)
{
    return true;
}

void RectTransformEditor::OnEditorGUI(RendererEngine &renderer, GameObject &pObj)
{
    auto pTransform = pObj.get_Component<RectTransform>();

    if (pTransform == nullptr)
    {
        return;
    }

    float width = pTransform->get_SizeDelta().x;
    float height = pTransform->get_SizeDelta().y;
    VEC2 anchorMax = pTransform->get_AnchorMax();
    VEC2 anchorMin = pTransform->get_AnchorMin();
    VEC2 pivot = pTransform->get_Pivot();
    VEC2 pos = pTransform->get_RectPosition();
    VEC3 rot = pTransform->get_VEC3ToRotateToDeg();



    // ノード
    if (Master::m_pDebugger->DG_TreeNode(U8ToChar(u8"UIトランスフォーム")))
    {
        Master::m_pDebugger->DG_Separator();    // 区切り線

        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"座標"));
        if (Master::m_pDebugger->DG_DragVec2("##Pos", &pos, m_SlideAccuRate, -100000.0f, 100000.0f))
        {
            pTransform->set_RectPosition(pos);
        }

        if (Master::m_pDebugger->DG_TreeNode(U8ToChar(u8"アンカー"))) {
            if (Master::m_pDebugger->DG_DragVec2(U8ToChar(u8"最小"), &anchorMin, 0.01f, -100000.0f, 100000.0f)) {
                pTransform->set_AnchorMin(anchorMin);
            }
            if (Master::m_pDebugger->DG_DragVec2(U8ToChar(u8"最大"), &anchorMax, 0.01f, -100000.0f, 100000.0f)) {
                pTransform->set_AnchorMax(anchorMax);
            }
            Master::m_pDebugger->DG_TreePop();  // ツリー終了
        }

        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"ピボット"));
        Master::m_pDebugger->DG_SameLine(); // くっつける
        if (Master::m_pDebugger->DG_DragVec2("##Pivot", &pivot, 0.01f, -100000.0f, 100000.0f)) {
            pTransform->set_Pivot(pivot);
        }

        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"幅"));
        Master::m_pDebugger->DG_SameLine(); // くっつける
        if (Master::m_pDebugger->DG_DragFloat("##SizeX", 1, &width, m_SlideAccuRate, -100000.0f, 100000.0f)) {
            pTransform->set_SizeDelta(VEC2(width, height));
        }
        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"高さ"));
        Master::m_pDebugger->DG_SameLine(); // くっつける
        if (Master::m_pDebugger->DG_DragFloat("##SizeY", 1, &height, m_SlideAccuRate, -100000.0f, 100000.0f)) {
            pTransform->set_SizeDelta(VEC2(width, height));
        }

        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"回転"));
        if (Master::m_pDebugger->DG_DragVec3("##Rot", &rot, m_SlideAccuRate, -180.0f, 180.0f))
        {
            pTransform->set_RotateToDeg(rot);
        }


        Master::m_pDebugger->DG_SliderFloat(U8ToChar(u8"編集スライド精度"), 1, &m_SlideAccuRate, 0.001f, 2.0f);

        Master::m_pDebugger->DG_TreePop();
    }
}



// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:PhysicsEditor Class --- */
//
// ***************************************************************************************
bool PhysicsEditor::Init(RendererEngine& renderer)
{
    return true;
}

void PhysicsEditor::OnEditorGUI(RendererEngine& renderer, GameObject& pObj)
{
    auto pPhysics = pObj.get_Component<Physics>();

    if (pPhysics == nullptr)
    {
        return;
    }

    VEC3 vel = pPhysics->get_Velocity();
    VEC3 forceAccumulator = pPhysics->get_ForceAccumulator();
    float mass = pPhysics->get_Mass();
    float gravityScale = pPhysics->get_GravityScale();
    float airDrag = pPhysics->get_AirDrag();
    float restitution = pPhysics->get_Restitution();
    float maxSpeed = pPhysics->get_MaxSpeed();
    float moveDrag = pPhysics->get_MoveDrag();

    // ノード
    if (Master::m_pDebugger->DG_TreeNode(U8ToChar(u8"物理")))
    {
        Master::m_pDebugger->DG_Separator();    // 区切り線
        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"質量"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_DragFloat("##Mass", 1, &mass, 0.5f, 0.0f, 100.0f);

        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"重力"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_DragFloat("##Gravity", 1, &gravityScale, 0.5f, 0.0f, 100.0f);

        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"空中抵抗"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_DragFloat("##AirDrag", 1, &airDrag, 0.01f, 0.0f, 1.0f);

        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"移動抵抗"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_DragFloat("##MoveDrag", 1, &moveDrag, 0.01f, 0.0f, 1.0f);

        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"反発係数"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_DragFloat("##Restitution", 1, &restitution, 0.01f, 0.0f, 1.0f);

        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"最大速度"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_DragFloat("##MaxSpeed", 1, &maxSpeed, 0.01f, 0.0f, 100.0f);


        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"移動ベクトル"));
        Master::m_pDebugger->DG_DragVec3("##Velocity", &vel, 0.0f, -0.0f, 0.0f);

        Master::m_pDebugger->DG_BulletText(U8ToChar(u8" 1フレームに蓄積された力"));
        Master::m_pDebugger->DG_DragVec3("##ForceAccumulator", &forceAccumulator, 0.0f, -0.0f, 0.0f);


        Master::m_pDebugger->DG_TreePop();
    }


    pPhysics->set_Mass(mass);
    pPhysics->set_GravityScale(gravityScale);
	pPhysics->set_AirDrag(airDrag);
	pPhysics->set_Restitution(restitution);
	pPhysics->set_MaxSpeed(maxSpeed);
    pPhysics->set_MoveDrag(moveDrag);
}



// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:DirectionLightEditor Class --- */
//
// ***************************************************************************************
bool DirectionLightEditor::Init(RendererEngine &renderer)
{
    return true;
}

void DirectionLightEditor::OnEditorGUI(RendererEngine &renderer, GameObject &pObj)
{
    // ディレクショナルライトコンポーネントの取得
    auto pDirectionLig = pObj.get_Component<DirectionalLight>();

    if (pDirectionLig == nullptr)
    {
        return;
    }

    // beginはInspectorWindowで行っている
    float intensity = pDirectionLig->get_Intensity();
    VEC3 lightColor = pDirectionLig->get_LightColor();
    float shadowDist = pDirectionLig->get_ShadowDistance();
    float shadowFocusOfsDist = pDirectionLig->get_ShadowFocusOffsetDistance();;
    float orthogriphicWidth = pDirectionLig->get_OrthographicWidth();
    float orthogriphicHeight = pDirectionLig->get_OrthographicHeight();

    // ノード
    if (Master::m_pDebugger->DG_TreeNode(U8ToChar(u8"ディレクションライト")))
    {
        Master::m_pDebugger->DG_Separator();    // 区切り線
        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"強さ"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_SliderFloat("##Insensity", 1, &intensity, 0.0f, 100.0f);

        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"カラー"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_ColorEdit3("##DirLigColor", &lightColor);

        Master::m_pDebugger->DG_Separator();    // 区切り線
        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"シャドウ"));
        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"距離"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_SliderFloat("##ShadowDist", 1, &shadowDist,         10.0f, 10000.0f);

        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"追従オブジェクトからのオフセット距離"));
        Master::m_pDebugger->DG_SliderFloat("##FocusOfs",   1, &shadowFocusOfsDist, 10.0f, 2000.0f);
        
        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"横幅"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_SliderFloat("##Width",      1, &orthogriphicWidth,  1.0f, 2000.0f);

        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"縦幅"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_SliderFloat("##Height",     1, &orthogriphicHeight, 1.0f, 2000.0f);

        Master::m_pDebugger->DG_TreePop();
    }

    // 反映
    pDirectionLig->set_Intensity(intensity);
    pDirectionLig->set_LightColor(lightColor);

    pDirectionLig->set_ShadowDistance(shadowDist);
    pDirectionLig->set_ShadowFocusOffsetDistance(shadowFocusOfsDist);
    pDirectionLig->set_OrthographicWidth(orthogriphicWidth);
    pDirectionLig->set_OrthographicHeight(orthogriphicHeight);
}



// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:PointLightEditor Class --- */
//
// ***************************************************************************************
bool PointLightEditor::Init(RendererEngine &renderer)
{
    return true;
}

void PointLightEditor::OnEditorGUI(RendererEngine &renderer, GameObject &pObj)
{
    // ポイントライトコンポーネントの取得
    auto pPointLig = pObj.get_Component<PointLight>();

    if (pPointLig == nullptr)
    {
        return;
    }

    // beginはInspectorWindowで行っている
    float intensity = pPointLig->get_Intensity();
    float range = pPointLig->get_Range();
    VEC3 lightColor = pPointLig->get_LightColor();

    // ノード
    if (Master::m_pDebugger->DG_TreeNode(U8ToChar(u8"ポイントライト")))
    {
        Master::m_pDebugger->DG_Separator();    // 区切り線
        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"強さ"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_DragFloat("##Insensity", 1, &intensity, 0.5f, 0.0f, 100.0f);

        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"範囲"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_DragFloat("##Range", 1, &range, 0.5f, 0.0f, 1000.0f);

        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"カラー"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_ColorEdit3("##DirLigColor", &lightColor);

        Master::m_pDebugger->DG_TreePop();
    }

    // 反映
    pPointLig->set_Intensity(intensity);
    pPointLig->set_LightColor(lightColor);
    pPointLig->set_Range(range);
}

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:SpotLightEditor Class --- */
//
// ***************************************************************************************
bool SpotLightEditor::Init(RendererEngine& renderer)
{
    return true;
}

void SpotLightEditor::OnEditorGUI(RendererEngine& renderer, GameObject& pObj)
{
    // ポイントライトコンポーネントの取得
    auto pSpotLig = pObj.get_Component<SpotLight>();

    if (pSpotLig == nullptr)
    {
        return;
    }

    // beginはInspectorWindowで行っている
    float intensity = pSpotLig->get_Intensity();
    float range = pSpotLig->get_Range();
    float angle = pSpotLig->get_AngleDeg();
    VEC3 lightColor = pSpotLig->get_LightColor();

    // ノード
    if (Master::m_pDebugger->DG_TreeNode(U8ToChar(u8"スポットライト")))
    {
        Master::m_pDebugger->DG_Separator();    // 区切り線
        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"強さ"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_DragFloat("##Insensity", 1, &intensity, 0.5f, 0.0f, 100.0f);

        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"範囲"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_DragFloat("##Range", 1, &range, 0.5f, 0.0f, 1000.0f);

        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"放射角度"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_DragFloat("##Angle", 1, &angle, 0.5f, -360.0f, 360.0f);

        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"カラー"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_ColorEdit3("##DirLigColor", &lightColor);

        Master::m_pDebugger->DG_TreePop();
    }

    // 反映
    pSpotLig->set_Intensity(intensity);
    pSpotLig->set_LightColor(lightColor);
    pSpotLig->set_SpotLightData(range, angle);
}




// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:PlayerControllerEditor Class --- */
//
// ***************************************************************************************
bool PlayerControllerEditor::Init(RendererEngine &renderer)
{
    return true;
}

void PlayerControllerEditor::OnEditorGUI(RendererEngine &renderer, GameObject &pObj)
{
    // コンポーネントの取得
    auto pComp = pObj.get_Component<PlayerController>();

    if (pComp == nullptr)
    {
        return;
    }

    // beginはInspectorWindowで行っている
    float moveSpeed             = pComp->get_MoveSpeed();
    float moveMaxSpeed          = pComp->get_MoveMaxSpeed();
    PlayerData::PLAYER_RANGER_ANIM_ID animId  = pComp->get_AnimID();
    VEC3 moveVelocity           = pComp->get_MoveVelocity();
    bool isJump                 = pComp->get_IsJump();
    bool isGrounded             = pComp->get_IsGrounded();
    bool isAngle                 = pComp->get_IsContinuousAngle();
    float jumpFoce              = pComp->get_JumpFoce();
    float gravity               = pComp->get_Gravity();

    // ノード
    if (Master::m_pDebugger->DG_TreeNode(U8ToChar(u8"プレイヤーコントローラー")))
    {
        Master::m_pDebugger->DG_Separator();    // 区切り線

        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"移動速度"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_DragFloat("##MoveSpeed", 1, &moveSpeed, 1.0f, 0.0f, 1000.0f);
        
        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"moveMaxSpeed移動x速度"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_DragFloat("##MoveMaxSpeed", 1, &moveMaxSpeed, 1.0f, 0.0f, 1000.0f);

        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"速度ベクトル"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_DragVec3("##Velocity", &moveVelocity, 0.0f, -10000, 10000);

        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"再生中のアニメーション"));
        Master::m_pDebugger->DG_TextValue("ID : %d",static_cast<int>(animId));
        Master::m_pDebugger->DG_TextValue(U8ToChar(u8"名前 : %s"), PlayerData::g_PlayerAnimationNames[static_cast<int>(animId) + 1].c_str());

        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"ジャンプフラグ"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_RadioButton("##JumpFlag", isJump);

        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"接地"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_RadioButton("##Grounded", isGrounded);

        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"ジャンプ力"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_DragFloat("##JumpFoce", 1, &jumpFoce,0.01f, 0.0f, 100.0f);

        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"重力"));
        Master::m_pDebugger->DG_Spacing();
        Master::m_pDebugger->DG_Spacing();
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_DragFloat("##Gravity", 1, &gravity, 0.01f, 0.0f, 10.0f);

        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"マウスに合わせて継続的に回転"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_CheckBox("##IsAngle", &isAngle);

        Master::m_pDebugger->DG_TreePop();
    }

    // 反映
    pComp->set_MoveSpeed(moveSpeed);
    pComp->set_AnimID(animId);
    pComp->set_JumpFoce(jumpFoce);
    pComp->set_Gravity(gravity);
    pComp->set_IsContinuousAngle(isAngle);
}


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:PlayerControllerEditor Class --- */
//
// ***************************************************************************************
bool Camera3DEditor::Init(RendererEngine &renderer)
{
    return true;
}

void Camera3DEditor::OnEditorGUI(RendererEngine &renderer, GameObject &pObj)
{
    // コンポーネントの取得
    auto pComp = pObj.get_Component<Camera3D>();

    if (pComp == nullptr)
    {
        return;
    }

    // beginはInspectorWindowで行っている

    float angle_H = pComp->get_Angle_H();
    float angle_V = pComp->get_Angle_V();
    float fov = pComp->get_Fov();
    float nearClip = pComp->get_Near();
    float farClip = pComp->get_Far();
    std::string focusObjTag = pComp->get_FocusObjectTag();
    VEC3 focusPoint = pComp->get_FocusPoint();
    VEC3 posOffset = pComp->get_PosOffset();
    VEC3 focusOffset = pComp->get_FocusOffset();

    // ノード
    if (Master::m_pDebugger->DG_TreeNode(U8ToChar(u8"3Dカメラ")))
    {
        Master::m_pDebugger->DG_Separator();    // 区切り線

        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"視野"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_SliderFloat("##Fov", 1, &fov, 0.1f, 179.0f);

        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"手前クリップ"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_DragFloat("##NearClip", 1, &nearClip, 1.0f, 0.0f, 600.0f);
        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"奥側クリップ"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_DragFloat("##FarClip", 1, &farClip, 1.0f, 0.0f, 30000.0f);

        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"水平アングル"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_SliderFloat("##AngleH", 1, &angle_H, -5.0f, 5.0f);
        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"垂直アングル"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_SliderFloat("##AngleV", 1, &angle_V, -5.0f, 5.0f);

        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"注視点"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_DragVec3("##Focus:", &focusPoint, 0.1f, -10000.0f, 10000.0f);

        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"フォーカスしているオブジェクトのタグ："));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_Text(focusObjTag.c_str());

        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"オフセット座標"));
        Master::m_pDebugger->DG_DragVec3("##ofsP", &posOffset, 1.0f, 0.0f, 1000.0f);

        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"オフセット注視点"));
        Master::m_pDebugger->DG_DragVec3("##ofsF", &focusOffset, 1.0f, -1000.0f, 1000.0f);

        Master::m_pDebugger->DG_TreePop();
    }

    /*
    * 補正
    */
    if (nearClip <= 0.0f)
    {
        nearClip = 1.0f;
    }
    if (farClip <= nearClip)
    {
        farClip = nearClip + 200.0f;    // 同じだとダメなので補正値足す
    }
    if (fov <= 0.0f)
    {
        fov = 1.0f;
    }

    // 反映
    pComp->set_Angle_H(angle_H);
    pComp->set_Angle_V(angle_V);
    pComp->set_FocusPoint(focusPoint);
    pComp->set_PosOffset(posOffset);
    pComp->set_FocusOffset(focusOffset);
    pComp->set_Fov(fov);
    pComp->set_Near(nearClip);
    pComp->set_Far(farClip);
}

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:SkinnedMeshAnimatorEditor Class --- */
//
// ***************************************************************************************
bool SkinnedMeshAnimatorEditor::Init(RendererEngine &renderer)
{
    return true;
}

void SkinnedMeshAnimatorEditor::OnEditorGUI(RendererEngine &renderer, GameObject &pObj)
{
    // コンポーネントの取得
    auto pComp = pObj.get_Component<SkinnedMeshAnimator>();

    if (pComp == nullptr)
    {
        return;
    }

    float animTime = pComp->get_AnimProcTime();
    int animIdx = pComp->getAnimIndex();
    int prebAnimIdx = pComp->get_PrevAnimIndex();
    bool isAnim = pComp->get_IsAnim();
    std::vector<AnimationData *>animDataList = pComp->get_AnimationDataList();

    // beginはInspectorWindowで行っている

    // ノード
    if (Master::m_pDebugger->DG_TreeNode(U8ToChar(u8"スキニングアニメーター")))
    {
        Master::m_pDebugger->DG_Separator();    // 区切り線
        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"アニメーション時間：%f"), animTime);
        Master::m_pDebugger->DG_RadioButton(U8ToChar(u8"再生中か"), isAnim);

        if (Master::m_pDebugger->DG_TreeNode(U8ToChar(u8"アニメーション情報")))
        {
            for (auto &anim : animDataList)
            {
                anim->Duration;
                anim->Name;
                anim->TicksPerSecond;

                if (Master::m_pDebugger->DG_TreeNode(anim->Name))
                {
                    Master::m_pDebugger->DG_BulletText(U8ToChar(u8"総時間：%f"), anim->Duration);
                    Master::m_pDebugger->DG_BulletText(U8ToChar(u8"ティックパーセカンド：%f"), anim->TicksPerSecond);
                    Master::m_pDebugger->DG_TreePop();
                }
            }
            Master::m_pDebugger->DG_TreePop();
        }
        Master::m_pDebugger->DG_TreePop();
    }

    /*
    * 補正
    */

    // 反映
}

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:ModelMeshRendererEditor Class --- */
//
// ***************************************************************************************
bool ModelMeshRendererEditor::Init(RendererEngine& renderer)
{
    return true;
}

void ModelMeshRendererEditor::OnEditorGUI(RendererEngine& renderer, GameObject& pObj)
{
    // コンポーネントの取得
    auto pComp = pObj.get_Component<ModelMeshRenderer>();

    if (pComp == nullptr)
    {
        return;
    }

	uint32_t meshNum= pComp->get_MeshNum();		// メッシュ数
    bool isWireFrame = pComp->get_IsDrawWireframe();

    // ノード
    if (Master::m_pDebugger->DG_TreeNode(U8ToChar(u8"モデルレンダラー")))
    {
		Master::m_pDebugger->DG_Separator();    // 区切り線

        // メッシュの表示非表示の切り替え
        for (uint32_t i = 0; i < meshNum; i++)
        {
            bool isVisible = pComp->get_IsMeshVisible(i);
            if (Master::m_pDebugger->DG_CheckBox(U8ToChar(u8"メッシュ") + std::to_string(i + 1), &isVisible))
            {
                pComp->set_IsMeshVisible(i, isVisible);
            }
		}

        // ワイヤーフレームモードの切り替え
        if (Master::m_pDebugger->DG_CheckBox(U8ToChar(u8"ワイヤーフレーム"),&isWireFrame))
        {
            pComp->set_IsDrawWireframe(isWireFrame);
        }

        Master::m_pDebugger->DG_TreePop();
    }
}


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:ModelMeshResourceEditor Class --- */
//
// ***************************************************************************************
bool ModelMeshResourceEditor::Init(RendererEngine &renderer)
{
    return true;
}

void ModelMeshResourceEditor::OnEditorGUI(RendererEngine &renderer, GameObject &pObj)
{
    // ディレクショナルライトコンポーネントの取得
    auto pComp = pObj.get_Component<ModelMeshResource>();

    if (pComp == nullptr)
    {
        return;
    }

    auto modelData = pComp->get_ModelData().lock();
    auto meshNum = modelData->get_MeshNum();        // メッシュ数
    auto vertexNum = modelData->get_VertexNum();    // 頂点数
    auto indexNum = modelData->get_IndexNum();      // インデックス数
    auto boneNum = modelData->get_BoneNum();        // ボーン数
    auto shaderType = modelData->get_ShaderType();  // シェーダ
    auto matNum = modelData->get_MaterialNum();     // マテリアル数
    std::vector<std::weak_ptr<Material>> matList = modelData->get_MaterialList();   // マテリアルリスト

    // beginはInspectorWindowで行っている

    using VECTOR4::VEC4;
    using VECTOR2::VEC2;

    // ノード
    if (Master::m_pDebugger->DG_TreeNode(U8ToChar(u8"モデルリソース")))
    {
        Master::m_pDebugger->DG_Separator();    // 区切り線
        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"メッシュ数：%d"), meshNum);
        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"頂点数：%d"), vertexNum);
        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"インデックス数：%d"), indexNum);
        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"ボーン数：%d"), boneNum);
        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"しぇーたの種類数：%d"), static_cast<int>(shaderType));
        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"マテリアル数：%d"), matNum);

        if (Master::m_pDebugger->DG_TreeNode(U8ToChar(u8"マテリアル")))
        {
            for (int i = 0; i < matList.size(); i++)
            {
                auto mat = matList[i].lock();

                if (mat == nullptr)
                {
                    continue;
                }

                VEC4 diffuseCol     = mat->m_DiffuseColor;
                VEC4 specCol        = mat->m_SpecularColor;
                VEC3 emissiveCol    = mat->m_EmissiveColor;
                float specPow       = mat->m_SpecularPower;
                float emissivePow   = mat->m_EmissivePower;
                float reflectionStrength   = mat->m_EnvironmentReflectionStrength;

                ID3D11ShaderResourceView* diffSRV = nullptr;
                ID3D11ShaderResourceView* norSRV = nullptr;
                ID3D11ShaderResourceView* specSRV = nullptr;

                /* SRVマップ取得 */
                // ディフューズ
                if (!mat->m_DiffuseMap.Texture.expired())
                {
                    diffSRV = mat->m_DiffuseMap.Texture.lock()->get_SRV();
                }
                // ノーマル
                if (!mat->m_NormalMap.Texture.expired())
                {
                    norSRV = mat->m_NormalMap.Texture.lock()->get_SRV();
                }
                // スペキュラ
                if (!mat->m_SpecularMap.Texture.expired())
                {
                    specSRV = mat->m_SpecularMap.Texture.lock()->get_SRV();
                }


                Master::m_pDebugger->DG_Separator();    // 区切り線

                if (Master::m_pDebugger->DG_TreeNode(U8ToChar(u8"マテリアル") + std::to_string(i + 1)))
                {
                    Master::m_pDebugger->DG_Separator();    // 区切り線
                    Master::m_pDebugger->DG_BulletText(U8ToChar(u8"ディフューズ色"));
                    Master::m_pDebugger->DG_Image(diffSRV, VEC2(100, 100));
                    if (Master::m_pDebugger->DG_ColorEdit4("##Diffuse" + std::to_string(i), &diffuseCol))
                    {
                        mat->m_DiffuseColor = diffuseCol;
                    }


                    Master::m_pDebugger->DG_Separator();    // 区切り線
                    Master::m_pDebugger->DG_BulletText(U8ToChar(u8"ノーマル"));
                    Master::m_pDebugger->DG_Image(norSRV, VEC2(100, 100));


                    Master::m_pDebugger->DG_Separator();    // 区切り線
                    Master::m_pDebugger->DG_BulletText(U8ToChar(u8"スペキュラ色"));
                    Master::m_pDebugger->DG_Image(specSRV, VEC2(100, 100));
                    if (Master::m_pDebugger->DG_ColorEdit4("##Speular" + std::to_string(i), &specCol))
                    {
                        mat->m_SpecularColor = specCol;
                    }

                    Master::m_pDebugger->DG_BulletText(U8ToChar(u8"スペキュラ強度"));
                    Master::m_pDebugger->DG_SameLine();
                    if (Master::m_pDebugger->DG_DragFloat("##SpecularPower" + std::to_string(i), 1, &specPow, 1.0f, 0.0f, 255.0f))
                    {
                        mat->m_SpecularPower = specPow;
                    }

                    Master::m_pDebugger->DG_Separator();    // 区切り線
                    Master::m_pDebugger->DG_BulletText(U8ToChar(u8"エミッシブ色"));
                    Master::m_pDebugger->DG_Image(specSRV, VEC2(100, 100));
                    if (Master::m_pDebugger->DG_ColorEdit3("##EmissiveColor" + std::to_string(i), &emissiveCol))
                    {
                        mat->m_EmissiveColor = emissiveCol;
                    }

                    Master::m_pDebugger->DG_BulletText(U8ToChar(u8"エミッシブ強度"));
                    Master::m_pDebugger->DG_SameLine();
                    if (Master::m_pDebugger->DG_DragFloat("##EmissivePower" + std::to_string(i), 1, &emissivePow, 0.5f, 0.0f, 255.0f))
                    {
                        mat->m_EmissivePower = emissivePow;
                    }

                    Master::m_pDebugger->DG_BulletText(U8ToChar(u8"反射強度"));
                    Master::m_pDebugger->DG_SameLine();
                    if (Master::m_pDebugger->DG_DragFloat("##ReflectionStrength" + std::to_string(i), 1, &reflectionStrength, 0.01f, 0.0f, 1.0f))
                    {
                        mat->m_EnvironmentReflectionStrength = reflectionStrength;
                    }

                    Master::m_pDebugger->DG_TreePop();
                }
            }
            Master::m_pDebugger->DG_Separator();    // 区切り線

            Master::m_pDebugger->DG_TreePop();
        }

        Master::m_pDebugger->DG_TreePop();
    }

    /*
    * 補正
    */

    // 反映
}

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:MeshResourceEditor Class --- */
//
// ***************************************************************************************
bool MeshResourceEditor::Init(RendererEngine& renderer)
{
    return true;
}

void MeshResourceEditor::OnEditorGUI(RendererEngine& renderer, GameObject& pObj)
{
    auto pComp = pObj.get_Component<IMeshResource>();

    if (pComp == nullptr)
    {
        return;
    }

    if (Master::m_pDebugger->DG_TreeNode(U8ToChar(u8"メッシュリソース")))
    {
        Master::m_pDebugger->DG_Separator();

        auto meshData = pComp->m_pMeshData;
        if (meshData == nullptr)
        {
            Master::m_pDebugger->DG_BulletText(U8ToChar(u8"メッシュデータ：未設定"));
            Master::m_pDebugger->DG_TreePop();
            return;
        }

        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"頂点数：%u"), meshData->NumVertex);
        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"インデックス数：%u"), meshData->NumIndex);
        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"頂点ストライド：%u byte"), meshData->VertexStride);
        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"マテリアル数：%u"), meshData->NumMaterial);
        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"シェーダー種類：%d"), static_cast<int>(pComp->get_ShaderType()));
        Master::m_pDebugger->DG_BulletText(
            U8ToChar(u8"動的頂点バッファ：%s"),
            meshData->IsDynamic ? U8ToChar(u8"はい") : U8ToChar(u8"いいえ"));
        Master::m_pDebugger->DG_BulletText(
            U8ToChar(u8"頂点バッファ：%s"),
            meshData->pVertexBuffer != nullptr ? U8ToChar(u8"有効") : U8ToChar(u8"未設定"));
        Master::m_pDebugger->DG_BulletText(
            U8ToChar(u8"インデックスバッファ：%s"),
            meshData->pIndexBuffer != nullptr ? U8ToChar(u8"有効") : U8ToChar(u8"未設定"));

        Master::m_pDebugger->DG_Separator();


        auto material = meshData->pMaterials.lock();
        if (material == nullptr)
        {
            Master::m_pDebugger->DG_BulletText(U8ToChar(u8"マテリアル：未設定"));
            Master::m_pDebugger->DG_TreePop();
            return;
        }

        if (Master::m_pDebugger->DG_TreeNode(U8ToChar(u8"マテリアル")))
        {
            VEC4 diffuseColor = material->m_DiffuseColor;
            VEC4 specularColor = material->m_SpecularColor;
            VEC3 emissiveColor = material->m_EmissiveColor;
            float specularPower = material->m_SpecularPower;
            float emissivePower = material->m_EmissivePower;
            float reflectionStrength = material->m_EnvironmentReflectionStrength;

            ID3D11ShaderResourceView* diffuseSRV = nullptr;
            ID3D11ShaderResourceView* normalSRV = nullptr;
            ID3D11ShaderResourceView* specularSRV = nullptr;

            if (auto texture = material->m_DiffuseMap.Texture.lock())
            {
                diffuseSRV = texture->get_SRV();
            }
            if (auto texture = material->m_NormalMap.Texture.lock())
            {
                normalSRV = texture->get_SRV();
            }
            if (auto texture = material->m_SpecularMap.Texture.lock())
            {
                specularSRV = texture->get_SRV();
            }

            Master::m_pDebugger->DG_Separator();
            Master::m_pDebugger->DG_BulletText(U8ToChar(u8"ディフューズ色"));
            Master::m_pDebugger->DG_Image(diffuseSRV, VEC2(100.0f, 100.0f));
            if (Master::m_pDebugger->DG_ColorEdit4("##MeshResourceDiffuse", &diffuseColor))
            {
                material->m_DiffuseColor = diffuseColor;
            }

            Master::m_pDebugger->DG_Separator();
            Master::m_pDebugger->DG_BulletText(U8ToChar(u8"ノーマルマップ"));
            Master::m_pDebugger->DG_Image(normalSRV, VEC2(100.0f, 100.0f));

            Master::m_pDebugger->DG_Separator();
            Master::m_pDebugger->DG_BulletText(U8ToChar(u8"スペキュラ色"));
            Master::m_pDebugger->DG_Image(specularSRV, VEC2(100.0f, 100.0f));
            if (Master::m_pDebugger->DG_ColorEdit4("##MeshResourceSpecular", &specularColor))
            {
                material->m_SpecularColor = specularColor;
            }

            Master::m_pDebugger->DG_BulletText(U8ToChar(u8"スペキュラ強度"));
            Master::m_pDebugger->DG_SameLine();
            if (Master::m_pDebugger->DG_DragFloat(
                "##MeshResourceSpecularPower", 1, &specularPower, 1.0f, 0.0f, 255.0f))
            {
                material->m_SpecularPower = specularPower;
            }

            Master::m_pDebugger->DG_Separator();
            Master::m_pDebugger->DG_BulletText(U8ToChar(u8"エミッシブ色"));
            if (Master::m_pDebugger->DG_ColorEdit3("##MeshResourceEmissive", &emissiveColor))
            {
                material->m_EmissiveColor = emissiveColor;
            }

            Master::m_pDebugger->DG_BulletText(U8ToChar(u8"エミッシブ強度"));
            Master::m_pDebugger->DG_SameLine();
            if (Master::m_pDebugger->DG_DragFloat(
                "##MeshResourceEmissivePower", 1, &emissivePower, 0.5f, 0.0f, 255.0f))
            {
                material->m_EmissivePower = emissivePower;
            }

            Master::m_pDebugger->DG_BulletText(U8ToChar(u8"反射強度"));
            Master::m_pDebugger->DG_SameLine();
            if (Master::m_pDebugger->DG_DragFloat(
                "##MeshResourceReflectionStrength", 1, &reflectionStrength, 0.01f, 0.0f, 1.0f))
            {
                material->m_EnvironmentReflectionStrength = reflectionStrength;
            }

            Master::m_pDebugger->DG_BulletText(U8ToChar(u8"カリングモード"));
            if (Master::m_pDebugger->DG_RadioButton(
                U8ToChar(u8"なし##MeshResourceCullNone"),
                material->m_CullMode == RenderData::CULL_MODE::NONE))
            {
                material->m_CullMode = RenderData::CULL_MODE::NONE;
            }
            Master::m_pDebugger->DG_SameLine();
            if (Master::m_pDebugger->DG_RadioButton(
                U8ToChar(u8"表面##MeshResourceCullFront"),
                material->m_CullMode == RenderData::CULL_MODE::FRONT))
            {
                material->m_CullMode = RenderData::CULL_MODE::FRONT;
            }
            Master::m_pDebugger->DG_SameLine();
            if (Master::m_pDebugger->DG_RadioButton(
                U8ToChar(u8"裏面##MeshResourceCullBack"),
                material->m_CullMode == RenderData::CULL_MODE::BACK))
            {
                material->m_CullMode = RenderData::CULL_MODE::BACK;
            }

            Master::m_pDebugger->DG_TreePop();
        }

        Master::m_pDebugger->DG_TreePop();
    }
}


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:BoxColliderEditor Class --- */
//
// ***************************************************************************************
bool BoxColliderEditor::Init(RendererEngine &renderer)
{
    return true;
}

void BoxColliderEditor::OnEditorGUI(RendererEngine &renderer, GameObject &pObj)
{
    // コンポーネントの取得
    auto pComp = pObj.get_Component<BoxCollider>();

    if (pComp == nullptr)
    {
        return;
    }

    // beginはInspectorWindowで行っている
    bool isTrigger = pComp->get_IsTrigger();
    bool isEnable = pComp->get_IsEnable();
    bool isHit = pComp->get_IsHit();
    bool isStatic = pComp->get_IsStatic();
    bool isDebug = pComp->get_IsDrawDebugMesh();
    COLLISION_JUDGMENT judgmentType = pComp->get_CollisionJudgmentType();
    COLLISION_CATEGORY category = pComp->get_CollisionCategory();
    uint32_t collisionBitMask = pComp->get_CollisionBitMask();
    uint32_t responseBitMask = pComp->get_ResponseBitMask();

    // カテゴリを文字列に変換
    std::string categoryStr = g_CollisionCategoryStringMap[(category)];

    VEC3 center = pComp->get_Center();
    VEC3 size = pComp->get_Size();

    // ノード
    if (Master::m_pDebugger->DG_TreeNode(U8ToChar(u8"ボックスコライダー")))
    {
        Master::m_pDebugger->DG_Separator();    // 区切り線

        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"使用フラグ"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_CheckBox("##IsEnable", &isEnable);

        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"衝突カテゴリ : ") + categoryStr);


        // 衝突判定を行う対象
        if (DrawCollisionBitMaskEditor(
            U8ToChar(u8"判定対象マスク"),
            collisionBitMask,
            "CollisionMask"))
        {
            pComp->set_CollisionBitMask(collisionBitMask);
        }

        // 押し戻しなどの物理反応を行う対象
        if (DrawCollisionBitMaskEditor(
            U8ToChar(u8"物理反応マスク"),
            responseBitMask,
            "ResponseMask"))
        {
            pComp->set_ResponseBitMask(responseBitMask);
        }


        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"デバッグ用メッシュを表示"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_CheckBox("##IsDebug", &isDebug);

        
        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"トリガー"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_CheckBox("##IsTrigger", &isTrigger);


        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"静的フラグ"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_CheckBox("##IsStatic", &isStatic);

        
        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"衝突フラグ"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_CheckBox("##IsHit", &isHit);
        
        if (Master::m_pDebugger->DG_TreeNode(U8ToChar(u8"判定方法")))
        {
            if (Master::m_pDebugger->DG_RadioButton("AABB", judgmentType == COLLISION_JUDGMENT::AABB))
            {
                judgmentType = COLLISION_JUDGMENT::AABB;
            }

            Master::m_pDebugger->DG_SameLine();
            
            if (Master::m_pDebugger->DG_RadioButton("OBB", judgmentType == COLLISION_JUDGMENT::OBB))
            {
                judgmentType = COLLISION_JUDGMENT::OBB;
            }

            Master::m_pDebugger->DG_TreePop();

        }

        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"中心位置"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_DragVec3("##Center", &center, 0.1f, -10000.0f, 10000.0f);


        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"サイズ"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_DragVec3("##Size", &size, 0.1f, -1000.0f, 1000.0f);

        Master::m_pDebugger->DG_TreePop();

    }

    // 反映
    pComp->set_IsEnable(isEnable);
    pComp->set_IsTrigger(isTrigger);
    pComp->set_IsStatic(isStatic);
    pComp->set_IsDrawDebugMesh(isDebug);
    pComp->set_Center(center);
    pComp->set_Size(size);
    pComp->set_CollisionJudgmentType(judgmentType);
}
bool BoxColliderEditor::DrawCollisionBitMaskEditor(
    const char* label,
    uint32_t& bitMask,
    const char* id)
{
    bool isChanged = false;

    ImGui::PushID(id);

    if (ImGui::TreeNode(label))
    {
        if (ImGui::Button(
            U8ToChar(u8"すべてON")))
        {
            bitMask =
                static_cast<unsigned>(
                    COLLISION_CATEGORY::EVERY);

            isChanged = true;
        }

        ImGui::SameLine();

        if (ImGui::Button(
            U8ToChar(u8"すべてOFF")))
        {
            bitMask =
                static_cast<unsigned>(
                    COLLISION_CATEGORY::NONE);

            isChanged = true;
        }

        ImGui::Separator();

        for (const auto& [category, categoryName] :
            g_CollisionCategoryStringMap)
        {
            if (category == COLLISION_CATEGORY::EVERY ||
                category == COLLISION_CATEGORY::NONE)
            {
                continue;
            }

            const unsigned categoryBit =
                static_cast<unsigned>(category);

            bool isEnabled =
                (bitMask & categoryBit) != 0;

            ImGui::PushID(categoryName.c_str());

            if (ImGui::Checkbox(
                categoryName.c_str(),
                &isEnabled))
            {
                if (isEnabled)
                {
                    bitMask |= categoryBit;
                }
                else
                {
                    bitMask &= ~categoryBit;
                }

                isChanged = true;
            }

            ImGui::PopID();
        }

        ImGui::TreePop();
    }

    ImGui::PopID();

    return isChanged;
}


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:SphereColliderEditor Class --- */
//
// ***************************************************************************************
bool SphereColliderEditor::Init(RendererEngine &renderer)
{
    return true;
}

void SphereColliderEditor::OnEditorGUI(RendererEngine &renderer, GameObject &pObj)
{
    // コンポーネントの取得
    auto pComp = pObj.get_Component<SphereCollider>();

    if (pComp == nullptr)
    {
        return;
    }

    // beginはInspectorWindowで行っている
    bool isTrigger = pComp->get_IsTrigger();
    bool isEnable = pComp->get_IsEnable();
    bool isHit = pComp->get_IsHit();
    bool isStatic = pComp->get_IsStatic();
    bool isDebug = pComp->get_IsDrawDebugMesh();

    VEC3 center = pComp->get_Center();
    float radius = pComp->get_Radius();

    // ノード
    if (Master::m_pDebugger->DG_TreeNode(U8ToChar(u8"スフィアコライダー")))
    {
        Master::m_pDebugger->DG_Separator();    // 区切り線

        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"使用フラグ"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_CheckBox("##IsEnable", &isEnable);


        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"デバッグ用メッシュを表示"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_CheckBox("##IsDebug", &isDebug);
        
        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"トリガー"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_CheckBox("##IsTrigger", &isTrigger);


        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"静的フラグ"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_CheckBox("##IsStatic", &isStatic);

        
        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"衝突フラグ"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_CheckBox("##IsHit", &isHit);


        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"中心位置"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_DragVec3("##Center", &center, 0.1f, -10000.0f, 10000.0f);


        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"サイズ"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_DragFloat("##Radius", 1, &radius, 0.1f, 1.0f, 1000.0f);

        Master::m_pDebugger->DG_TreePop();

    }

    // 反映
    pComp->set_IsEnable(isEnable);
    pComp->set_IsTrigger(isTrigger);
    pComp->set_IsStatic(isStatic);
    pComp->set_IsDrawDebugMesh(isDebug);
    pComp->set_Center(center);
    pComp->set_Radius(radius);
}


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:TrailRendererEditor Class --- */
//
// ***************************************************************************************
bool TrailRendererEditor::Init(RendererEngine &renderer)
{
    return true;
}

void TrailRendererEditor::OnEditorGUI(RendererEngine &renderer, GameObject &pObj)
{
    // コンポーネントの取得
    auto pComp = pObj.get_Component<TrailRenderer>();

    if (pComp == nullptr)
    {
        return;
    }

    // beginはInspectorWindowで行っている
    float minDist = pComp->get_MinVertexDistance();
    float width = pComp->get_Width();
    float time = pComp->get_DrawTime();
    VEC3 randPos = pComp->get_PosRandVec();


    // ノード
    if (Master::m_pDebugger->DG_TreeNode(U8ToChar(u8"トレイルレンダラー")))
    {
        Master::m_pDebugger->DG_Separator();    // 区切り線

        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"頂点間の最小距離"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_DragFloat("##Dist", 1, &minDist, 0.1f, 1.0f, 5000.0f);

        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"幅"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_DragFloat("##Width", 1, &width, 0.1f, 1.0f, 5000.0f);

        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"持続時間"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_DragFloat("##Time", 1, &time, 0.1f, 1.0f, 5000.0f);

        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"位置のずれ"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_DragVec3("##RandPos", &randPos, 0.1f, 1.0f, 5000.0f);

        Master::m_pDebugger->DG_TreePop();
    }

    // 反映
    pComp->set_MinVertexDistance(minDist);
    pComp->set_Width(width);
    pComp->set_DrawTime(time);
    pComp->set_PosRandVec(randPos);
}


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:LineRendererEditor Class --- */
//
// ***************************************************************************************
bool LineRendererEditor::Init(RendererEngine &renderer)
{
    return true;
}

void LineRendererEditor::OnEditorGUI(RendererEngine &renderer, GameObject &pObj)
{
    // コンポーネントの取得
    auto pComp = pObj.get_Component<LineRenderer>();

    if (pComp == nullptr)
    {
        return;
    }

    // beginはInspectorWindowで行っている
    VEC3 dir = pComp->get_Dir();
    VEC3 start = pComp->get_StartPos();
    float length = pComp->get_Length();
    float width = pComp->get_Width();

    VEC4 color = pComp->get_Color();
    float emissive = pComp->get_Emissive();

    // ノード
    if (Master::m_pDebugger->DG_TreeNode(U8ToChar(u8"ラインレンダラー")))
    {
        Master::m_pDebugger->DG_Separator();    // 区切り線

        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"カラー"));
        Master::m_pDebugger->DG_ColorEdit4("##Diffuse", &color);
        Master::m_pDebugger->DG_DragFloat("エミッシブ", 1, &emissive, 0.1f, 0.0f, 5000.0f);


        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"始点"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_DragVec3("##Start", &start, 0.1f, -10000.0f, 10000.0f);

        
        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"方向"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_DragVec3("##Dir", &dir, 0.1f, -10000.0f, 10000.0f);


        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"伸びる距離"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_DragFloat("##Length", 1, &length, 0.1f, 1.0f, 5000.0f);


        Master::m_pDebugger->DG_BulletText(U8ToChar(u8"幅"));
        Master::m_pDebugger->DG_SameLine();
        Master::m_pDebugger->DG_DragFloat("##Width", 1, &width, 0.01f, 0.01f, 5000.0f);

        Master::m_pDebugger->DG_TreePop();
    }

    // 反映
    pComp->set_Dir(dir);
    pComp->set_StartPos(start);
    pComp->set_Width(width);
    pComp->set_Length(length);
    pComp->set_Color(color);
    pComp->set_Emissive(emissive);
}

