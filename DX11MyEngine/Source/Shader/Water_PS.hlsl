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
    // ===== 見た目の調整 =====
    const float2 flowDirection = float2(1.0f, 0.0f);
    const float flowSpeed = 0.06f;
    const float normalStrength = 0.5f;
    const float3 waterColor = float3(0.05f, 0.15f, 0.35f);

    // ① 時間に応じてテクスチャを流す
    float2 flow = flowDirection * flowSpeed * cb_Time;

    float2 uvA = input.UV - flow;

    // 2枚目は波の大きさ・速度・開始位置を変える
    float2 uvB = input.UV * 1.73f
               - flow * 0.8f
               + float2(0.17f, 0.09f);

    // ② ノーマルマップの値を 0～1 から -1～1 に変換
    float3 normalA =
        g_tNormalTextureA.Sample(g_sSampler, uvA).xyz
        * 2.0f - 1.0f;

    float3 normalB =
        g_tNormalTextureB.Sample(g_sSampler, uvB).xyz
        * 2.0f - 1.0f;

    // ③ 2枚のノーマルを合成し、水面の傾きを調整
    float3 normalTS = normalA + normalB;
    normalTS.xy *= normalStrength;
    normalTS.z = max(normalTS.z, 0.001f);
    normalTS = normalize(normalTS);

    // ④ 現在の水平な板ポリに合わせて向きを変換
    // テクスチャのU方向 = +X、V方向 = -Z、上方向 = +Y
    float3 normalWS =
        float3(normalTS.x, normalTS.z, -normalTS.y);

    // ⑤ 固定方向の光で簡単な明暗を付ける
    float3 lightDirection =
        normalize(float3(-0.3f, 1.0f, -0.4f));

    float lightAmount =
        saturate(dot(normalWS, lightDirection));

    float brightness = 0.6f + lightAmount * 0.4f;

    // 光に向いた部分を少し明るくする簡易表現
    float highlight = pow(lightAmount, 32.0f) * 0.15f;

    float3 color = waterColor * brightness;
    color += float3(0.8f, 0.95f, 1.0f) * highlight;

    // マテリアル色・頂点色を反映
    color *= cb_DiffuseColor.rgb * input.Color.rgb;

    // ⑥ マテリアルの透明度を使う
    float alpha = saturate(cb_DiffuseColor.a * input.Color.a);

    return float4(color, alpha);
}