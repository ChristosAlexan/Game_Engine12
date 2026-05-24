Texture2D<float4> gColorTexture : register(t5);
SamplerState gSampler : register(s0);

cbuffer FXAAConstants : register(b5)
{
    float2 invScreenSize;
    float contrastThreshold;
    float blendStrength;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

float Luma(float3 color)
{
    return dot(color, float3(0.299f, 0.587f, 0.114f));
}

float4 Main(PSInput input) : SV_TARGET
{
    float2 uv = input.uv;

    float2 uvN = saturate(uv + float2(0.0f, -invScreenSize.y));
    float2 uvS = saturate(uv + float2(0.0f, invScreenSize.y));
    float2 uvE = saturate(uv + float2(invScreenSize.x, 0.0f));
    float2 uvW = saturate(uv + float2(-invScreenSize.x, 0.0f));

    float2 uvNW = saturate(uv + float2(-invScreenSize.x, -invScreenSize.y));
    float2 uvNE = saturate(uv + float2(invScreenSize.x, -invScreenSize.y));
    float2 uvSW = saturate(uv + float2(-invScreenSize.x, invScreenSize.y));
    float2 uvSE = saturate(uv + float2(invScreenSize.x, invScreenSize.y));

    float3 center = gColorTexture.Sample(gSampler, uv).rgb;

    float3 north = gColorTexture.Sample(gSampler, uvN).rgb;
    float3 south = gColorTexture.Sample(gSampler, uvS).rgb;
    float3 east = gColorTexture.Sample(gSampler, uvE).rgb;
    float3 west = gColorTexture.Sample(gSampler, uvW).rgb;

    float3 nw = gColorTexture.Sample(gSampler, uvNW).rgb;
    float3 ne = gColorTexture.Sample(gSampler, uvNE).rgb;
    float3 sw = gColorTexture.Sample(gSampler, uvSW).rgb;
    float3 se = gColorTexture.Sample(gSampler, uvSE).rgb;

    float lumaCenter = Luma(center);
    float lumaNorth = Luma(north);
    float lumaSouth = Luma(south);
    float lumaEast = Luma(east);
    float lumaWest = Luma(west);

    float lumaNW = Luma(nw);
    float lumaNE = Luma(ne);
    float lumaSW = Luma(sw);
    float lumaSE = Luma(se);

    float lumaMin = min(lumaCenter,min(min(min(lumaNorth, lumaSouth), min(lumaEast, lumaWest)),min(min(lumaNW, lumaNE), min(lumaSW, lumaSE))));

    float lumaMax = max(lumaCenter,max(max(max(lumaNorth, lumaSouth), max(lumaEast, lumaWest)), max(max(lumaNW, lumaNE), max(lumaSW, lumaSE))));

    float contrast = lumaMax - lumaMin;

    if (contrast < contrastThreshold)
    {
        return float4(center, 1.0f);
    }

    float horizontalEdge = abs(lumaNW - lumaSW) + 2.0f * abs(lumaNorth - lumaSouth) + abs(lumaNE - lumaSE);

    float verticalEdge = abs(lumaNW - lumaNE) + 2.0f * abs(lumaWest - lumaEast) + abs(lumaSW - lumaSE);

    float3 blur;

    if (horizontalEdge >= verticalEdge)
    {
        blur = (west + center + east + 0.5f * (nw + ne + sw + se)) / 5.0f;
    }
    else
    {
        blur = (north + center + south + 0.5f * (nw + ne + sw + se)) / 5.0f;
    }

    float3 result = lerp(center, blur, saturate(blendStrength));

    return float4(result, 1.0f);
}