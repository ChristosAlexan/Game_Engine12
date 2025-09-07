struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 worldDir : TEXCOORD1;
};
TextureCube cubeMapTexture : register(t4, space0);
SamplerState gSampler : register(s0);

float4 Main(PSInput input) : SV_TARGET
{
    float3 dir = normalize(input.worldDir);
    //return float4(dir, 1.0f);
    return float4(cubeMapTexture.SampleLevel(gSampler, dir, 0).rgb, 1.0f);
}