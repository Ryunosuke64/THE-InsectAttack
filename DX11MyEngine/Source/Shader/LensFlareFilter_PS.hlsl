// \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
//
/* - @:ピクセルシェーダ -*/
//
//  【?】レンズフレア用
//
// \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
#pragma once
SamplerState g_sSampler : register(s0);
Texture2D g_tBrightTexture : register(t0);	// シーンの高輝度を抜き出したテクスチャ


/* =========================================================================
/* - @:入力構造体 - */
/* =========================================================================*/
struct PS_IN
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
    float2 UV : TEXCOORD;
};
static const float cb_GhostSpacing = 0.4; // 0.4
static const float cb_GhostIntensity = 0.6; // 0.6
static const float cb_HaloRadius = 0.5; // 0.5
static const float cb_HaloIntensity = 0.3; // 0.3
static const float cb_ChromaticOffset = 0.0031; // 0.003
static const float cb_Threshold = 1.0; // 1.0


static const int GHOST_COUNT = 6;
// 円形マスク
float CircleMask(float2 uv, float2 center, float radius, float softness)
{
    float dist = length(uv - center);

    return 1.0f - smoothstep(
        radius - softness,
        radius,
        dist
    );
}
float3 SampleBright(float2 uv)
{
    if (any(uv < 0.0f) || any(uv > 1.0f))
        return float3(0, 0, 0);

    return g_tBrightTexture.Sample(g_sSampler, uv).rgb;
}

float3 SampleChromatic(float2 uv, float2 direction)
{
    float2 offset = direction * cb_ChromaticOffset;

    return float3(
        SampleBright(uv + offset).r,
        SampleBright(uv).g,
        SampleBright(uv - offset).b
    );
}
float3 SampleCircularGhost(float2 centerUV, float radius)
{
    float3 color = float3(0, 0, 0);
    float totalWeight = 0.0f;

    const int SAMPLE_COUNT = 5;

    for (int y = -SAMPLE_COUNT; y <= SAMPLE_COUNT; y++)
    {
        for (int x = -SAMPLE_COUNT; x <= SAMPLE_COUNT; x++)
        {
            float2 offset = float2(x, y) / SAMPLE_COUNT;

            float dist = length(offset);

            // 円形の範囲内だけサンプリング
            float weight = 1.0f - smoothstep(0.7f, 1.0f, dist);

            float2 sampleUV = centerUV + offset * radius;

            color += SampleBright(sampleUV) * weight;
            totalWeight += weight;
        }
    }

    return color / max(totalWeight, 0.0001f);
}
float4 PSMain(PS_IN input) : SV_TARGET
{
    float2 uv = input.UV;

    float3 flareColor = float3(0, 0, 0);

    // =====================================
    // Ghost
    // =====================================
    float2 ghostVector = (0.5f - uv) * cb_GhostSpacing;

    float2 chromaticDir = normalize(
        ghostVector + float2(0.00001f, 0.00001f)
    );

    for (int i = 0; i < GHOST_COUNT; i++)
    {
        float2 sampleUV =
            1.0f - uv + ghostVector * i;

        float3 color = SampleCircularGhost(
            sampleUV,
            0.02
        );

        // Ghostごとの強度
        float weight = 1.0f - (float) i / GHOST_COUNT;

        flareColor += color * weight;
    }

    flareColor *= cb_GhostIntensity;

    // =====================================
    // Halo
    // =====================================
    float2 haloVector = normalize(
        0.5f - uv + float2(0.00001f, 0.00001f)
    );

    float2 haloUV =
        1.0f - uv + haloVector * cb_HaloRadius;

    float3 haloColor = SampleChromatic(
        haloUV,
        haloVector
    );

    flareColor += haloColor * cb_HaloIntensity;

    return float4(flareColor, 1.0f);
}