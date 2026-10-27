#pragma once
#include "EditorBase.h"


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:TransformEditor Class --- */
//
//  ★継承：EditorBase ★
//
// 【?】トランスフォーム編集用エディタ
//
// ***************************************************************************************
class TransformEditor : public EditorBase
{
private:
    float m_SlideAccuRate; // 位置の入力精度

public:
    TransformEditor() 
        : EditorBase(),
        m_SlideAccuRate(0.001f) 
    {};
    ~TransformEditor()
    {
    };

    bool Init(RendererEngine& renderer) override;
    void OnEditorGUI(RendererEngine &renderer, class GameObject &pObj) override;

};

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:RectTransformEditor Class --- */
//
//  ★継承：EditorBase ★
//
// 【?】2D用トランスフォーム編集用エディタ
//
// ***************************************************************************************
class RectTransformEditor : public EditorBase
{
private:
    float m_SlideAccuRate; // 位置の入力精度

public:
    RectTransformEditor()
        : EditorBase(),
        m_SlideAccuRate(0.001f) 
    {};
    ~RectTransformEditor()
    {
    };

    bool Init(RendererEngine& renderer) override;
    void OnEditorGUI(RendererEngine &renderer, class GameObject &pObj) override;

};


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:PhysicsEditor Class --- */
//
//  ★継承：EditorBase ★
//
// 【?】物理コンポーネント編集用エディタ
//
// ***************************************************************************************
class PhysicsEditor : public EditorBase
{
private:

public:
    PhysicsEditor()
        : EditorBase()
    {
    };
    ~PhysicsEditor()
    {
    };

    bool Init(RendererEngine& renderer) override;
    void OnEditorGUI(RendererEngine& renderer, class GameObject& pObj) override;

};


// Bullet剛体の設定と、実行中の物理状態を表示するエディタ。
class RigidBodyEditor : public EditorBase
{
public:
    bool Init(RendererEngine& renderer) override;
    void OnEditorGUI(RendererEngine& renderer, class GameObject& pObj) override;
};


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:DirectionLightEditor Class --- */
//
//  ★継承：EditorBase ★
//
// 【?】ディレクションライト編集用エディタ
//
// ***************************************************************************************
class DirectionLightEditor : public EditorBase
{
private:
    float m_SlideAccuRate; // 位置の入力精度

public:
    DirectionLightEditor()
        : EditorBase(),
        m_SlideAccuRate(1.0f)
    {
    };
    ~DirectionLightEditor()
    {
    };
    bool Init(RendererEngine &renderer) override;
    void OnEditorGUI(RendererEngine &renderer, class GameObject &pObj) override;

};



// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:PointLightEditor Class --- */
//
//  ★継承：EditorBase ★
//
// 【?】ポイントライト編集用エディタ
//
// ***************************************************************************************
class PointLightEditor : public EditorBase
{
private:
    float m_SlideAccuRate; // 位置の入力精度

public:
    PointLightEditor()
        : EditorBase(),
        m_SlideAccuRate(1.0f)
    {
    };
    ~PointLightEditor()
    {
    };
    bool Init(RendererEngine &renderer) override;
    void OnEditorGUI(RendererEngine &renderer, class GameObject &pObj) override;
};


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:SpotLightEditor Class --- */
//
//  ★継承：EditorBase ★
//
// 【?】スポットライト編集用エディタ
//
// ***************************************************************************************
class SpotLightEditor : public EditorBase
{
private:
    float m_SlideAccuRate; // 位置の入力精度

public:
    SpotLightEditor()
        : EditorBase(),
        m_SlideAccuRate(1.0f)
    {
    };
    ~SpotLightEditor()
    {
    };
    bool Init(RendererEngine &renderer) override;
    void OnEditorGUI(RendererEngine &renderer, class GameObject &pObj) override;
};


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:PlayerControllerEditor Class --- */
//
//  ★継承：EditorBase ★
//
// 【?】プレイヤーコントローラー編集用エディタ
//
// ***************************************************************************************
class PlayerControllerEditor : public EditorBase
{
private:
    float m_SlideAccuRate; // 位置の入力精度

public:
    PlayerControllerEditor()
        : EditorBase(),
        m_SlideAccuRate(1.0f)
    {
    };
    ~PlayerControllerEditor()
    {
    };
    bool Init(RendererEngine &renderer) override;
    void OnEditorGUI(RendererEngine &renderer, class GameObject &pObj) override;

};

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:Camera3DEditor Class --- */
//
//  ★継承：EditorBase ★
//
// 【?】カメラ3D編集用エディタ
//
// ***************************************************************************************
class Camera3DEditor : public EditorBase
{
private:

public:
    Camera3DEditor()
        : EditorBase()
    {
    };
    ~Camera3DEditor()
    {
    };
    bool Init(RendererEngine &renderer) override;
    void OnEditorGUI(RendererEngine &renderer, class GameObject &pObj) override;

};

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:SkinnedMeshAnimator Class --- */
//
//  ★継承：EditorBase ★
//
// 【?】スキンメッシュアニメーション編集用エディタ
//
// ***************************************************************************************
class SkinnedMeshAnimatorEditor : public EditorBase
{
private:
    float m_BlendDuration = 0.2f;

public:
    SkinnedMeshAnimatorEditor()
        : EditorBase()
    {
    };
    ~SkinnedMeshAnimatorEditor()
    {
    };
    bool Init(RendererEngine &renderer) override;
    void OnEditorGUI(RendererEngine &renderer, class GameObject &pObj) override;

};


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:ModelMeshRenderer Class --- */
//
//  ★継承：EditorBase ★
//
// 【?】モデルレンダラーの確認用エディタ
//
// ***************************************************************************************
class ModelMeshRendererEditor : public EditorBase
{
private:

public:
    ModelMeshRendererEditor()
        : EditorBase()
    {
    };
    ~ModelMeshRendererEditor()
    {
    };
    bool Init(RendererEngine& renderer) override;
    void OnEditorGUI(RendererEngine& renderer, class GameObject& pObj) override;
};

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:ModelMeshResourceEditor Class --- */
//
//  ★継承：EditorBase ★
//
// 【?】モデルリソースの確認用エディタ
//
// ***************************************************************************************
class ModelMeshResourceEditor : public EditorBase
{
private:

public:
    ModelMeshResourceEditor()
        : EditorBase()
    {
    };
    ~ModelMeshResourceEditor()
    {
    };
    bool Init(RendererEngine &renderer) override;
    void OnEditorGUI(RendererEngine &renderer, class GameObject &pObj) override;

};

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:MeshResourceEditor Class --- */
//
//  ★継承：EditorBase ★
//
// 【?】ユーティリティメッシュリソースの確認・編集用エディタ
//
// ***************************************************************************************
class MeshResourceEditor : public EditorBase
{
private:

public:
    MeshResourceEditor()
        : EditorBase()
    {
    };
    ~MeshResourceEditor()
    {
    };
    bool Init(RendererEngine& renderer) override;
    void OnEditorGUI(RendererEngine& renderer, class GameObject& pObj) override;
};


// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:BoxColliderEditor Class --- */
//
//  ★継承：EditorBase ★
//
// 【?】ボックスコライダーの編集用エディタ
//
// ***************************************************************************************
class BoxColliderEditor : public EditorBase
{
private:

public:
    BoxColliderEditor()
        : EditorBase()
    {
    };
    ~BoxColliderEditor()
    {
    };
    bool Init(RendererEngine &renderer) override;
    void OnEditorGUI(RendererEngine &renderer, class GameObject &pObj) override;

private:
    bool DrawCollisionBitMaskEditor(
        const char* label,
        uint32_t& bitMask,
        const char* id);

};

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:SphereColliderEditor Class --- */
//
//  ★継承：EditorBase ★
//
// 【?】スフィアコライダー編集用エディタ
//
// ***************************************************************************************
class SphereColliderEditor : public EditorBase
{
private:
public:
    SphereColliderEditor() : EditorBase() {};
    ~SphereColliderEditor() {};
    bool Init(RendererEngine &renderer) override;
    void OnEditorGUI(RendererEngine &renderer, class GameObject &pObj) override;
};

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:TrailRendererEditor Class --- */
//
//  ★継承：EditorBase ★
//
// 【?】スフィアコライダー編集用エディタ
//
// ***************************************************************************************
class TrailRendererEditor : public EditorBase
{
private:
public:
    TrailRendererEditor() : EditorBase() {};
    ~TrailRendererEditor() {};
    bool Init(RendererEngine &renderer) override;
    void OnEditorGUI(RendererEngine &renderer, class GameObject &pObj) override;
};

// ***************************************************************************************
// ---------------------------------------------------------------------------------------
/* --- @:LineRendererEditor Class --- */
//
//  ★継承：EditorBase ★
//
// 【?】ラインレンダラー編集用エディタ
//
// ***************************************************************************************
class LineRendererEditor : public EditorBase
{
private:
public:
    LineRendererEditor() : EditorBase() {};
    ~LineRendererEditor() {};
    bool Init(RendererEngine &renderer) override;
    void OnEditorGUI(RendererEngine &renderer, class GameObject &pObj) override;
};

class DistortionEffectEditor : public EditorBase
{
public:
    bool Init(RendererEngine& renderer) override;
    void OnEditorGUI(RendererEngine& renderer, class GameObject& pObj) override;
};
