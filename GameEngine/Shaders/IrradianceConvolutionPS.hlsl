#include "Math_Helpers.hlsli"

struct PSInput
{
    float4 position : SV_POSITION;
    float3 worldDir : TEXCOORD0;
};


TextureCube cubeMapTexture : register(t4, space0);
SamplerState gSampler : register(s0);

float4 Main(PSInput input) : SV_TARGET
{
    float3 dir = normalize(input.worldDir);
    float3 irradiance = float3(0.0f, 0.0f, 0.0f);
    float3 up = abs(dir.y);
    float3 right = normalize(cross(up, dir));
    up = normalize(cross(dir, right));
    
    float sampleDelta = 0.01;
    float nrSamples = 0.0;
    
    for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
         // spherical to cartesian (in tangent space)
            float3 tangentSample = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
        // tangent space to world
            float3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * dir;

            irradiance += cubeMapTexture.Sample(gSampler, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    
    irradiance *= PI / max(nrSamples, 1.0f);
    return float4(irradiance, 1.0f);
}