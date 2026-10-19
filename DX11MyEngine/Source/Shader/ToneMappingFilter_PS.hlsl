// \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//
/* - @:ピクセルシェーダ -*/
//
//  【?】HDR対応のためのトーンマッピングを行う
//      一旦、ACES (Academy Color Encoding System)というのを使用してみる
//      映画風のシネマティックな感じらしい（ 見てみたらEDF4みたいな感じだった ）
//
//      TODO:エフェクトの見た目が大幅に変わってしまうので、ACESは一旦飛ばして、ガンマ補正のみ行う。
//
// \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
#pragma once
#include "ConstantBuffers_H.hlsli"
SamplerState g_sSampler : register(s0);
Texture2D g_tTexture0 : register(t0); 


/* =========================================================================
/* - @:入力構造体 - */
/* =========================================================================*/
struct PS_IN
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
    float2 UV : TEXCOORD;
};

// ACESトーンマッピングの近似式
// 参考： https://bravememo.hatenablog.com/entry/2020/06/05/143255
//        https://ny-program.hatenablog.com/entry/2021/07/10/153631
float3 ACESToneMapping(float3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

float3 HablePartial(float3 x)
{
    const float A = 0.15;
    const float B = 0.50;
    const float C = 0.10;
    const float D = 0.20;
    const float E = 0.02;
    const float F = 0.30;

    return ((x * (A * x + C * B) + D * E) /
            (x * (A * x + B) + D * F)) - E / F;
}

float3 HableToneMapping(float3 color)
{
    const float whitePoint = 20.0f;     // 
    const float exposureBias = 5.0f;    // マッピング前の明るさ調整

    float3 mapped = HablePartial(color * exposureBias);
    float3 white = HablePartial(whitePoint.xxx);

    return mapped / white;
}


// **************************************************************************
/* - @:エントリーポイント - */
// **************************************************************************
float4 PSMain(PS_IN input) : SV_TARGET
{
    float3 hdrColor = g_tTexture0.Sample(g_sSampler, input.UV).rgb;
    float3 finalColor = float3(0, 0, 0);
    // 少し赤みを足して世紀末感を出す（定数バッファで変更できるようにすれば演出も作りやすそう）
    //hdrColor.r *= 1.2f;
    //hdrColor.r *= 0.2f;
    //hdrColor.g *= 0.2f;
    //hdrColor.b *= 0.2f;
    
    // トーンマッピング処理
    // TODO : エフェクトの見た目が極端に変わってしまうので、一旦飛ばす
    finalColor = hdrColor;
    
    // 露出
    finalColor *= exp2(cb_ColorGrading.Exposure);

    finalColor = HableToneMapping(finalColor);
    
    // コントラスト
    finalColor = (finalColor - 0.5f) * cb_ColorGrading.Contrast + 0.5f;

    // 彩度
    float luminance = dot(
        finalColor,
        float3(0.2126f, 0.7152f, 0.0722f)
    );

    finalColor = lerp(
        luminance.xxx,
        finalColor,
       cb_ColorGrading.Saturation
    );

    // 色味
    finalColor *= cb_ColorGrading.ColorTint;
    
    
    // リニア空間：現実的な正しい世界
    // ガンマ空間：モニタが表示する少し暗い世界
    
    // ガンマ補正
    // ガンマ値 2.2というものがあり、モニタの特性として、
    // そのままだと本来よりも暗くなってしまう。
    // 1.0 / 2.2をすることで0.4545...ガンマの逆数となり、
    // この累乗を計算することで正しい明るさで表示される......らしい。
    
    // 2026/07/13追記
    // バックバッファ側をSRGBにしているので、ここでガンマ補正を行う必要はない。
    //finalColor = pow(finalColor, 1.0f / cb_ColorGrading.Gamma);
    
    return float4(finalColor, 1.0f);;
}