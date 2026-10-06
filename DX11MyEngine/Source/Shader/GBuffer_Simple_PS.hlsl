// \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//
/* - @:Gパス -*/
//
//  【?】簡易GBuffer
//      接ベクトル、従ベクトルを使わないシンプルな実装
//
// \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
#pragma once
#include "LightFunctions_H.hlsli"           // ライトヘッダー
#include "UtilityFunctions_H.hlsli"         // 汎用関数群
SamplerState g_sSampler : register(s0);
Texture2D g_tDiffuseTex : register(t0);     // ディフューズ
Texture2D g_tNormalTex : register(t1);      // ノーマル
Texture2D g_tSpecularTex : register(t2);    // スペキュラ

/* =========================================================================
/* - @:入力構造体 - */
//* 【?】ライティング用
/* =========================================================================*/
struct PS_IN
{
    float4 Pos : SV_Position;
    float3 Normal : NORMAL0;
    float4 Color : COLOR0;
    float2 UV : TEXCOORD0;
};


//ピクセルシェーダーの出力構造体
struct PS_OUT
{
    float4 Albedo   : SV_Target0;
    float4 Normal   : SV_Target1;
    float4 Specular : SV_Target2;
    float4 Emissive : SV_Target3;
    //float4 Depth    : SV_Target4;
};


// **************************************************************************
/* - @:エントリーポイント - */
// **************************************************************************
PS_OUT PSMain(PS_IN input)
{
    float4 diffuseMap = g_tDiffuseTex.Sample(g_sSampler, input.UV);
    //float4 normalMap = g_tNormalTex.Sample(g_sSampler, input.UV);
    
    float4 finalCol = float4(1.0, 1.0, 1.0, 1.0);
    
    finalCol = diffuseMap * cb_DiffuseColor;
    
    clip(diffuseMap.a - 0.7f);
    
    //finalCol.xyz = input.Color.xyz;
    
    // こっちは法線マップなしver
    //float3 normal = GetNorm(normalMap, float3(1.0, 0.0, 0.0), float3(0.0, 0.0, -1.0), input.Normal);
    
    // ディザリング
    //DitheringClip(input.Pos.xy, 60);
    
    // 発光色を求める
    float3 emissiveColor = cb_EmissiveColor * cb_EmissivePower;

    // テスト出力
    PS_OUT output;
    output.Albedo       = finalCol;                         // ディフューズカラー格納
    output.Normal.rgb   = (input.Normal.rgb * 0.5f) + 0.5f; // 0～1に収める
    output.Normal.a     = 1.0f;                             // 空き
    output.Specular.rgb = cb_SpecularColor.rgb;             // スペキュラカラー格納
    output.Specular.a   = (cb_SpecularPower) / (255.0f);    // wに反射強度入れる（0～1に)
    output.Emissive.rgb = emissiveColor;                    // 発光カラー格納
    output.Emissive.a   = 1.0f; // エミッシブのaに環境反射の強さを入れる（0～1に収める）
    
    // 以下のように深度値を手動で入れてもライティングパス時には反映されないよ
    // 理由はDSVをパイプラインにバインドしているので、ハードウェア側が自動で深度値を入れてくれている。
    // output.Depth.x      = 0;
    
    return output;
}

