#include "pch.h"
#include "InspectorWindow.h"
#include "EditorBase.h"
#include "RendererEngine.h"
#include "InspectorEditors.h"
#include "GameObject.h"

//*---------------------------------------------------------------------------------------
//*【?】コンストラクタ
//*----------------------------------------------------------------------------------------
InspectorWindow::InspectorWindow(): EditWindowBase()
{
}

//*---------------------------------------------------------------------------------------
//*【?】デストラクタ
//*----------------------------------------------------------------------------------------
InspectorWindow::~InspectorWindow()
{
}

//*---------------------------------------------------------------------------------------
//*【?】初期化
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//*
//* [返値]
//* true : 成功
//* false : 失敗
//*----------------------------------------------------------------------------------------
bool InspectorWindow::Init(RendererEngine &renderer)
{
    // 各エディタの生成
    m_EditorMap["MyTransform"]          = std::make_unique<TransformEditor>();
    m_EditorMap["RectTransform"]        = std::make_unique<RectTransformEditor>();
    m_EditorMap["PhysicsEditor"]        = std::make_unique<PhysicsEditor>();
    m_EditorMap["RigidBody"]            = std::make_unique<RigidBodyEditor>();
    m_EditorMap["DirectionalLight"]     = std::make_unique<DirectionLightEditor>();
    m_EditorMap["PointLight"]           = std::make_unique<PointLightEditor>();
    m_EditorMap["SpotLight"]            = std::make_unique<SpotLightEditor>();
    m_EditorMap["PlayerController"]     = std::make_unique<PlayerControllerEditor>();
    m_EditorMap["Camera3D"]             = std::make_unique<Camera3DEditor>();
    m_EditorMap["SkinnedMeshAnimator"]  = std::make_unique<SkinnedMeshAnimatorEditor>();
    m_EditorMap["ModelMeshResource"]    = std::make_unique<ModelMeshResourceEditor>();
    m_EditorMap["MeshResource"]         = std::make_unique<MeshResourceEditor>();
    m_EditorMap["BoxCollider"]          = std::make_unique<BoxColliderEditor>();
    m_EditorMap["SphereCollider"]       = std::make_unique<SphereColliderEditor>();
    m_EditorMap["TrailRenderer"]        = std::make_unique<TrailRendererEditor>();
    m_EditorMap["LineRenderer"]         = std::make_unique<LineRendererEditor>();
    m_EditorMap["ModelMeshRenderer"]    = std::make_unique<ModelMeshRendererEditor>();

    return true;
}

//*---------------------------------------------------------------------------------------
//*【?】更新
//*
//* [引数]
//* &renderer : 描画エンジンの参照
//*
//* [返値] なし
//*----------------------------------------------------------------------------------------
void InspectorWindow::Update(RendererEngine &renderer)
{
    using namespace Tool;

    // 現在選択されているオブジェクトの取得
    m_pCrntSelectedObject = Master::m_pEditorManager->get_crntSelectedObject();

    const std::string windowName = std::string(U8ToChar(u8"インスペクター")) + "###Inspector";
    Master::m_pDebugger->BeginDebugWindow(windowName, 0);
    
    if (m_pCrntSelectedObject.expired() == false)
    {
        bool isActive = m_pCrntSelectedObject.lock()->get_IsStatusFlag(OBJECT_STATUS_BITFLAG::IS_ACTIVE);
        Master::m_pDebugger->DG_RadioButton(U8ToChar(u8"アクティブ状態"), isActive);
    }
    // 各エディタの更新
    for (auto &editor : m_EditorMap)
    {
        // 更新対象のオブジェクトが存在しているか
        if (auto obj = m_pCrntSelectedObject.lock())
        {
            editor.second->OnEditorGUI(renderer, *obj);
        }
    }
    Master::m_pDebugger->EndDebugWindow();
}


//*---------------------------------------------------------------------------------------
//*【?】現在選択されているオブジェクトの設定
//*
//* [引数]
//* pObj : 選択されたオブジェクトのポインタ
//*
//* [返値] なし
//*----------------------------------------------------------------------------------------
void InspectorWindow::set_CrntSelectedObject(std::shared_ptr<class GameObject> pObj)
{
    m_pCrntSelectedObject = pObj; 
}
