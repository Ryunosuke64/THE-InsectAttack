#pragma once
#include "Object.h"
#include "Component_IMeshResource.h"
#include "Component_SpriteRenderer.h"
#include "Component_BillboardResource.h"
#include "ModelData.h"


// 入力マテリアル情報
struct SetupMaterialInfo
{
    std::shared_ptr<Material> pMaterialData;  // マテリアル
    int Index = 0;              // マテリアル番号
};

/// <summary>
/// LODモデルのセットアップ情報
/// </summary>
struct SetupLODModelInfo
{
    std::string _path;
    float _distance = 0.0f;
};

/// <summary>
/// 拡張定数バッファ情報
/// </summary>
struct ExpandConstantBufferInfo
{
    int SetSlot = -1; 						    // 定数バッファ番号
    void* pUserExpandConstantBuffer = nullptr;  // ユーザー拡張用定数バッファ
    int UserExpandConstantBufferSize = 0;       // ユーザー拡張用定数バッファサイズ
};

/// <summary>
/// 生成に必要な基本の情報
/// </summary>
struct CreateMesh_Base
{
    RendererEngine* pRenderer;      // 描画
    std::string ObjTag;             // オブジェクトのタグ
    int ObjLayer;                   // オブジェクトの更新レイヤー
    bool IsActive;                  // 生成時にオブジェクトをアクティブにするか
    bool IsTransparent;             // 透明度があるか（ある場合はフォワード用のシェーダにしてね）
    SHADER_TYPE ShaderType;         // 使用するシェーダの種類
    SHADER_TYPE Shadow_ShaderType;  // シャドウマップ生成に使用するシェーダの種類

    CreateMesh_Base() :
        pRenderer(nullptr),
        IsActive(true),
        ShaderType(SHADER_TYPE::DEFERRED_STD_STATIC),
        Shadow_ShaderType(SHADER_TYPE::POST_SHADOWMAP),
        IsTransparent(false),
        ObjLayer(100),
        ObjTag("Unknown")
    {};
};

/// <summary>
/// モデルの生成情報
/// </summary>
struct CreateModelInfo : public CreateMesh_Base
{
    bool IsAnim;                                // アニメーションするかどうか
    int InitAnimIndex;                          // 最初に再生するアニメーション
    SetupMaterialInfo* SetupMaterial;           // マテリアル情報（MatNum分）
    UINT MatNum;                                // マテリアル数

    std::array<SetupLODModelInfo, MAX_REGISTER_LOD>  // LODデータ
        LODModels;  

    // コンストラクタ
    CreateModelInfo() :
        IsAnim(false),
        InitAnimIndex(0),
        SetupMaterial(nullptr),
        MatNum(0)
    {
    }
};


/// <summary>
/// 汎用メッシュ生成情報
/// </summary>
struct CreateUtilityMeshInfo : public CreateMesh_Base
{
    UTILITY_MESH_TYPE Type;      // メッシュタイプ
    bool IsNormalMap;            // 法線マップ使用するかどうか
    SetupMaterialInfo* MaterialData; // マテリアル情報
    UINT MatNum;                     // マテリアル数
	VECTOR2::VEC2 TilingScale;       // タイリングスケール（UVの繰り返し数） 

    // コンストラクタ
    CreateUtilityMeshInfo():
        MaterialData(nullptr),
        MatNum(0),
        Type(UTILITY_MESH_TYPE::NONE),
        IsNormalMap(false),
        TilingScale(VECTOR2::VEC2(1.0f,1.0f))
    {};
};


/// <summary>
/// スプライト生成情報
/// </summary>
struct CreateSpriteInfo : public CreateMesh_Base
{
    SPRITE_USAGE_TYPE Type;             // スプライトの使用方法
    BLEND_MODE Blend;
    ExpandConstantBufferInfo*pPSConstantBuffers;  // ピクセルシェーダ用定数バッファ
    ExpandConstantBufferInfo*pVSConstantBuffers;  // 頂点シェーダ用定数バッファ
    int PSConstBufferNum;        // ピクセルシェーダ用定数バッファ数
    int VSConstBufferNum;        // 頂点シェーダ用定数バッファ数

    std::map<int, std::weak_ptr<class Texture>> pTextureMap;    // テクスチャマップ

    // 幅
    float Width;
    float Height;

    CreateSpriteInfo():
        Width(0.0f),
        Height(0.0f),
        Type(SPRITE_USAGE_TYPE::NORMAL),
        pPSConstantBuffers(nullptr),
        pVSConstantBuffers(nullptr),
        PSConstBufferNum(0),
        VSConstBufferNum(0),
        Blend(BLEND_MODE::ALPHA)
    {};
};


/// <summary>
/// ビルボード生成情報
/// </summary>
struct CreateBillboradInfo : public CreateMesh_Base
{
    BILLBOARD_USAGE_TYPE Type;    // 使用方法
    FIXED_AXIS_BITFLAG FixedAxis; // 固定軸ビットフラグ（指定しない場合は固定軸なし）
    
    SetupMaterialInfo* MaterialData;  // マテリアル情報
    UINT MatNum;                  // マテリアル数

    // コンストラクタ
    CreateBillboradInfo() :
        MaterialData(nullptr),
        MatNum(0),
        Type(BILLBOARD_USAGE_TYPE::SIMPLE),
        FixedAxis()
    {};
};

/// <summary>
/// スカイボックス生成情報
/// </summary>
struct CreateSkyboxInfo : public CreateMesh_Base
{
    SetupMaterialInfo* MaterialData; // マテリアル情報
    UINT MatNum;                 // マテリアル数

    // コンストラクタ
    CreateSkyboxInfo() :
        MaterialData(nullptr),
        MatNum(0)
    {};
};


/// <summary>
/// デカール生成情報
/// </summary>
struct CreateDecalInfo : public CreateMesh_Base
{
    UTILITY_MESH_TYPE Type;      // メッシュタイプ
    bool IsNormalMap;            // 法線マップ使用するかどうか
    SetupMaterialInfo *MaterialData; // マテリアル情報
    UINT MatNum;                     // マテリアル数
    bool IsDynamic;                  // 動的変更を行うか

    // コンストラクタ
    CreateDecalInfo() :
        MaterialData(nullptr),
        MatNum(0),
        Type(UTILITY_MESH_TYPE::NONE),
        IsNormalMap(false),
        IsDynamic(false)
    {
    };
};




class MeshFactory
{
private:


public:
    static ModelLODSet LoadLODModels(const CreateModelInfo& info);  // LODモデルの読み込み
    static std::shared_ptr<class GameObject> CreateModel(const CreateModelInfo& info);
    static std::shared_ptr<class GameObject> CreateUtilityMesh(const CreateUtilityMeshInfo& info);
    static std::shared_ptr<class GameObject> CreateSprite(const CreateSpriteInfo &info);
    static std::shared_ptr<class GameObject> CreateBillboard(const CreateBillboradInfo& info);
    static std::shared_ptr<class GameObject> CreateSkybox(const CreateSkyboxInfo& info);
    static std::shared_ptr<class GameObject> CreateDecal(const CreateDecalInfo & info);
}; 

