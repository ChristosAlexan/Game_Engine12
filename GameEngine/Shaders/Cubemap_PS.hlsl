struct PSInput
{
    float4 position : SV_POSITION;
    float3 worldDir : TEXCOORD0;
};

Texture2D hdrEquirectMap : register(t0, space3);
SamplerState samplerState : register(s0);

float2 SampleEquirectangularMap(float3 dir)
{
    float2 invAtan = float2(0.1591, 0.3183);
    float2 uv = float2(atan2(dir.z, dir.x), asin(dir.y));
    uv *= invAtan;
    uv += 0.5;
    uv.y = 1 - uv.y; // flip it to render corectly
    return uv;
}

float4 Main(PSInput input) : SV_TARGET
{
    float3 dir = normalize(input.worldDir);
    float2 uv = SampleEquirectangularMap(dir);
    float3 color = hdrEquirectMap.Sample(samplerState, uv).rgb;
    return float4(color, 1.0);
}