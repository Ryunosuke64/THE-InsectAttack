// \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//
/* - @:ピクセルシェーダ -*/
//
//  【?】フォワードシェーディング
//       水表現用
//
// \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
#pragma once
#include "ConstantBuffers_H.hlsli"
SamplerState g_sSampler : register(s0);
Texture2D g_tNormalTextureA : register(t0); // 水用ノーマルマップA
Texture2D g_tNormalTextureB : register(t1); // 水用ノーマルマップB 

/* =========================================================================
/* - @:出力構造体 - */
/* =========================================================================*/
struct PS_SimpleIntput
{
    float4 Pos : SV_Position;
    float3 Normal : NORMAL0;
    float4 Color : COLOR0;
    float2 UV : TEXCOORD0;
};


// **************************************************************************
/* - @:エントリーポイント - */
// **************************************************************************
float4 PSMain(PS_SimpleIntput input) : SV_TARGET
{
    float2 uvA = input.UV;
    float2 uvB = input.UV;

    uvA += float2(0.10, 0.02) * cb_Time;
    uvB += float2(-0.04, 0.08) * cb_Time;
    
    float3 texA = g_tNormalTextureA.Sample(g_sSampler, uvA).xyz * 2.0f - 1.0f;
    float3 texB = g_tNormalTextureB.Sample(g_sSampler, uvB).xyz * 2.0f - 1.0f;
    float3 waterNormal = normalize(texA + texB);
    
    float4 finalColor = float4(0.0, 0.0, 0.0, 1.0);
    finalColor.xyz = waterNormal;
    finalColor *= cb_DiffuseColor;
    
    return finalColor;
}