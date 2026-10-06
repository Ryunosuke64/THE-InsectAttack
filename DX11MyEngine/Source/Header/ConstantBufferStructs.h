#pragma once
//==================================================================================================================================================================================
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
// 定数バッファ用 
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//==================================================================================================================================================================================
 

// ***************************************************************************************
//						定数バッファの送信先スロット
// ***************************************************************************************
constexpr int CB_SLOT_TRANSFORM = 0;	        // ワールド変換行列用
constexpr int CB_SLOT_VIEW = 1;		            // ビュー変換行列用
constexpr int CB_SLOT_PROJECTION = 2;	        // 投影変換行列用
constexpr int CB_SLOT_BONE = 3;	                // スキニング用ボーン行列用
constexpr int CB_SLOT_MATERIAL = 4;	            // マテリアル情報用
constexpr int CB_SLOT_LIGHTS = 5;	            // ライト用
//constexpr int CB_SLOT_DIRECTIONAL_LIGHT = 5;	// ディレクションライト用
//constexpr int CB_SLOT_POINT_LIGHT = 6;	    // ポイントライト用
constexpr int CB_SLOT_BLUR_WEIGHTS = 6;	        // ブラーの重み用
constexpr int CB_SLOT_POSTEFFECT = 7;	        // ポストエフェクト用
constexpr int CB_SLOT_SHADOW = 8;	            // シャドウマップ用
constexpr int CB_SLOT_SPRITE = 9;	            // スプライト用
constexpr int CB_SLOT_DECAL = 10;	            // デカール用
constexpr int CB_SLOT_WINDOW = 11;	            // ウインドウ情報用
constexpr int CB_SLOT_DISTORTION = 12;          // ディストーション用


//=========================================================================================
//
//						ワールド変換行列 - slot 0
// 
//=========================================================================================
struct CB_TRANSFORM {
    DirectX::XMFLOAT4X4 WorldMtx;  // ワールド変換行列
};

struct CB_TRANSFORM_SET {
    CB_TRANSFORM Data = {};
    ID3D11Buffer* pBuff = nullptr;
};

//=========================================================================================
//
//						ビュー変換行列 - slot 1
// 
//=========================================================================================
struct CB_VIEW
{
    DirectX::XMFLOAT4X4 View;

    // ビュープロジェクション行列の逆行列
    DirectX::XMFLOAT4X4 ViewProjInvMatrix;
};

struct CB_VIEW_SET
{
    CB_VIEW Data = {};
    ID3D11Buffer* pBuff = nullptr;
};

//=========================================================================================
//
//						投影変換行列 - slot 2
// 
//=========================================================================================
struct CB_PROJECTION
{
    DirectX::XMFLOAT4X4 Projection;	    // 透視投影行列
    DirectX::XMFLOAT4X4 InvProjection;  // 逆行列
    DirectX::XMFLOAT4X4 OrthographicProjection;  // 正射投影
};

struct CB_PROJECTION_SET
{
    CB_PROJECTION Data={};
    ID3D11Buffer* pBuff = nullptr;
};

//=========================================================================================
//
//						スキニング用ボーン行列 - slot 3
// 
//=========================================================================================
constexpr int MAX_BONES = 128; // ボーンの最大数（必要に応じて変更）
struct CB_BONES_DATA
{
    DirectX::XMFLOAT4X4 BonesMatrices[MAX_BONES];
};

struct CB_BONES_DATA_SET
{
    CB_BONES_DATA Data = {};
    ID3D11Buffer* pBuff = nullptr;
};

//=========================================================================================
//
//						マテリアル情報 - slot 4
// 
//=========================================================================================
struct CB_MATERIAL {
    DirectX::XMFLOAT4 Diffuse;              // ディフューズ反射光

    DirectX::XMFLOAT4 Specular;             // スペキュラ反射光

    float SpecularPower;                    // スペキュラの強さ
    float EmissivePower;                    // 自己発光の強さ（ブルーム強度）
    DirectX::XMFLOAT2 OffsetUV;             // UVオフセット（エネルギーの流れる感じとかの表現に使えそう ヘクトルみたいな）

    DirectX::XMFLOAT3 EmissiveColor;        // 発光カラー
	float EnvironmentReflectionStrength;    // 環境反射の強さ

     // パディング(16バイトアラインメント ※詳細はUtilityHeader.hlsli側を見て)
};

struct CB_MATERIAL_SET {
    CB_MATERIAL Data = {};
    ID3D11Buffer* pBuff = nullptr;
};


//=========================================================================================
//
//						ライト情報 - slot 5
// 
//=========================================================================================

/* =========================================================================
//* - @:ディレクションライト用構造体 - */
//* =========================================================================
struct CB_DirectionLightData
{
    DirectX::XMFLOAT3 Direction;    // 方向
    float Padding;
    DirectX::XMFLOAT3 DiffuseColor; // ディフューズ色
    float DiffuseIntensity;         // ディフューズ光度
    DirectX::XMFLOAT3 SpecularColor;// スペキュラ色
    float SpecularIntensity;        // スペキュラ強度

    // ライトから見た画面座標
    DirectX::XMFLOAT4X4 LightViewProj;

};

/* =========================================================================
//* - @:ポイントライト用構造体 - */
//* =========================================================================
struct CB_PointLightData
{
    DirectX::XMFLOAT3 Pos;          // 座標
    float Range;                    // 影響範囲

    DirectX::XMFLOAT3 DiffuseColor; // ディフューズ色
    float DiffuseIntensity;         // ディフューズ光度

    DirectX::XMFLOAT3 SpecularColor;// スペキュラ色
    float SpecularIntensity;        // スペキュラ強度
};

/* =========================================================================
//* - @:スポットライト用構造体 - */
//* =========================================================================
struct CB_SpotLightData
{
    DirectX::XMFLOAT3 Pos;                  // 座標   
    float Range;                            // 範囲

    DirectX::XMFLOAT3 DiffuseColor;         // ディフューズ
    float DiffuseIntensity;                 // ディフューズ光度

    DirectX::XMFLOAT3 SpecularColor;        // スペキュラ
    float SpecularIntensity;                // スペキュラ強度

    DirectX::XMFLOAT3 Direction;            // 放射する方向
    float Angle;                            // 射出角度
};

struct CB_LIGHTS
{
    /* ディレクションライト */
    CB_DirectionLightData DirLights[DIRECTIONLIGHT_MAX_NUM];    // 今のところは1個しか使わないけど、複数のライトに対応できるように配列にしている

    /* ポイントライト */
    CB_PointLightData PointLights[POINTLIGHT_MAX_NUM];          // 100個

    /* スポットライト */
    CB_SpotLightData SpotLights[SPOTLIGHT_MAX_NUM];             // 20個

    int32_t PointLightCount;                                    // 使用するポイントライトの個数
    int32_t SpotLightCount;                                     // 使用するスポットライトの個数
    float Padding1;
    float Padding2;
    DirectX::XMFLOAT3 EyePos;       // 視点の位置
    float Padding3;
};


//=========================================================================================
//<ユーザー拡張>
//						ブラー情報 - slot 7 
// 
//=========================================================================================
struct CB_BLUR
{
	float weights[8];  // ブラーの重み
};

//=========================================================================================
//<ユーザー拡張>
//					ポストエフェクト情報 - slot 8
// 
//=========================================================================================

/// <summary> 被写界深度情報 </summary>
struct CB_Dof
{
	float dof_MaxRange; // ぼかしの最大距離
	float dof_MinRange; // ぼかしの開始距離
	float pad[2];
};
/// <summary> フォグ情報 </summary>
struct CB_Fog
{
    DirectX::XMFLOAT3 Color;    // フォグカラー
    float Start;                // フォグの開始距離
    float End;                  // フォグの終了距離
    float padding[3];
};
/// <summary> カラーグレーディング情報 </summary>
struct CB_ColorGrading
{
    float Exposure;                 // 露出補正（1.0fで補正なし）
    float Contrast;                 // コントラスト（1.0fで補正なし）
    float Saturation;               // 彩度（1.0fで補正なし）
    float Gamma;                    // ガンマ補正（1.0fで補正なし）

    DirectX::XMFLOAT3 ColorTint;    // 色味（1.0fで補正なし）
    float padding;
};
 
struct CB_POSTEFFECT {
	CB_Dof Dof;                     // 被写界深度情報
	CB_ColorGrading ColorGrading;   // カラーグレーディング情報
    CB_Fog Fog;                     // フォグ
};

//=========================================================================================
//<ユーザー拡張>
//						シャドウ情報 - slot 9
// 
//=========================================================================================
struct CB_SHADOW
{
    float baseShadowBias;
    float slopeScaledBias;
    float depthBiasClamp;
    BOOL isEnable;              // ただのboolだと、1バイトになってしまうため、BOOL型にして4バイトにしている
};


//=========================================================================================
//
//						スプライト情報 - slot 10
// 
//=========================================================================================
struct CB_SPRITE
{
    DirectX::XMFLOAT2 OffsetUV;   // UVオフセット
    float pad[2];
};

struct CB_SPRITE_SET {
    CB_SPRITE Data = {};
    ID3D11Buffer* pBuff = nullptr;
};


//=========================================================================================
//
//						デカール情報 - slot 11
// 
//=========================================================================================
struct CB_DECAL {
    DirectX::XMFLOAT4X4 DecalWorldInvMtx;  // デカールボックスのワールド変換行列
    DirectX::XMFLOAT3 DecalColor;           // カラー
    float pad;
};

struct CB_DECAL_SET {
    CB_DECAL Data = {};
    ID3D11Buffer *pBuff = nullptr;
};


//=========================================================================================
//
//						ウインドウ情報 - slot 12
// 
//=========================================================================================
struct CB_WINDOW {
	float WindowWidth;          // ウインドウ幅
	float WindowHeight;         // ウインドウ高さ
    float pad[2];
};

struct CB_WINDOW_SET {
    CB_WINDOW Data = {};
    ID3D11Buffer* pBuff = nullptr;
};


//=========================================================================================
//
//						ディストーション情報 - slot 13
// 
//=========================================================================================
struct CB_DISTORTION {
	float Power;                 // ディストーションの強さ
	float Time;                  // ディストーションの時間（アニメーションさせるためのもの）
	DirectX::XMFLOAT2 UVScale;   // ディストーションのUVスケール
};

//=========================================================================================
//
//						ディストーション情報 - slot 13
// 
//=========================================================================================
struct CB_FRAME {
    float time = 0.0f;      // ゲーム開始時からの時間
    float deltaTime = 0.0f; // デルタタイム
    float padding[2];
};



// *****************************************************************************************
/*                           ユーザー拡張                                                 */
// *****************************************************************************************
struct CB_USER_EXPAND_SET{
    void * pData = nullptr;
    ID3D11Buffer* pBuff = nullptr;
    int Size = 0;
    int Slot = -1;
};











// *****************************************************************************************
/*                          ＣＰＵ用 ３Ｄ情報                                             */
// *****************************************************************************************
// ディフューズマップ
struct DIFFUSE_MAP_DATA
{
    std::weak_ptr <class Texture> Texture;
    int index = -1;
};

// スペキュラマップ
struct SPECULAR_MAP_DATA
{
    std::weak_ptr <class Texture> Texture;
    int index = -1;
};

// ノーマルマップ
struct NORMAL_MAP_DATA
{
    std::weak_ptr <class Texture> Texture;
    int index = -1;
};

//* =========================================================================
//* - @:BLEND_MODE列挙体 - */
//* 【?】ブレンドモード
//* =========================================================================
enum class BLEND_MODE
{
    NONE = 0,       // 何もしない
    ALPHA,          // 半透明
    ALPHA_TRANS,    // 半透明合成
    ADD,            // 加算
    SUB,            // 減算

    NUM,
};


// テクスチャマップ
enum TEXTURE_MAP
{
    TEXTURE_MAP_NONE,
    TEXTURE_MAP_DIFFUSE,	// ディフューズ 拡散光
    TEXTURE_MAP_NORMAL,		// ノーマル		凹凸（法線）
    TEXTURE_MAP_SPECULAR,	// スペキュラ	反射光
};


struct InstanceData
{
    DirectX::XMMATRIX World;
    VECTOR4::VEC4 Color;
    VECTOR2::VEC2 UV;
};

