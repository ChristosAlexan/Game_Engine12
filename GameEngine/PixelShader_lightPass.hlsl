#include "General_PS.hlsli"
#include "Math_Helpers.hlsli"

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};


float3 PointLight(float3 worldPos, uint index);
float3 SpotLight(float3 worldPos, float3 normal, uint index);

Texture2D albedoTexture : register(t0, space0);
Texture2D normalTexture : register(t1, space0);
Texture2D metalRoughnessMaskTexture : register(t2, space0);
Texture2D worldPosDepthTexture : register(t3, space0);

// space2: Lights
StructuredBuffer<GPULight> g_Lights : register(t0, space2);
SamplerState gSampler : register(s0);

float4 Main(PSInput input) : SV_TARGET
{
    float4 color = albedoTexture.Sample(gSampler, input.uv).rgba;
    float mask = metalRoughnessMaskTexture.Sample(gSampler, input.uv).b;
    
    if(mask == 0.0f)
        return float4(color.rgb, 1.0f);
    
    float3 normal = normalTexture.Sample(gSampler, input.uv).xyz;
    float3 worldPos = worldPosDepthTexture.Sample(gSampler, input.uv).xyz;
    float metalness = metalRoughnessMaskTexture.Sample(gSampler, input.uv).r;
    float rougness = metalRoughnessMaskTexture.Sample(gSampler, input.uv).g;
   
    
    float3 ambient = float3(0.1f, 0.1f, 0.1f);
    normal = normalize(normal.xyz);
    
    float3 result = ambient;
    for (uint i = 0; i < 2; ++i)
    {
        switch (g_Lights[i].lighType)
        {
            case 0: // Directional
            {
                    break;
            }
            case 1: // Spot
            {
                  result += SpotLight(worldPos, normal, i);
                   break;
            }
            case 2: // Point
            {
                    result += PointLight(worldPos, i);
                    break;
            }
        }
        
    }
    
    return float4(result * color.rgb, 1.0f);
}

float3 PointLight(float3 worldPos, uint index)
{
    float3 result;
    
    float3 toLight = worldPos.xyz - g_Lights[index].position;
    float dist = length(toLight);
    float atten = saturate(1.0 - dist / g_Lights[index].radius);
    result = g_Lights[index].color * atten * g_Lights[index].strength;
    
    return result;
}

float3 SpotLight(float3 worldPos, float3 normal, uint index)
{
    float3 V = normalize(cameraPos.xyz - worldPos.xyz);
    
    float4 q = normalize(g_Lights[index].direction);
    float3 forward = RotateVectorByQuaternion(float3(0, 0, 1), q);
    float3 L = normalize(g_Lights[index].position - worldPos).xyz;
    float theta = dot(L, normalize(-forward));
    float outerCutOff = g_Lights[index].cutoff / 3.0f;
    float epsilon = g_Lights[index].cutoff - outerCutOff;
    float intensity = clamp((theta - outerCutOff) / epsilon, 0.0f, 1.0f);
    float3 H = normalize(V + L);
    float distance = length(g_Lights[index].position - worldPos.xyz);
    float attenuation = 1.0f / (distance * distance) * intensity;
    float3 radiance = g_Lights[index].color.xyz * attenuation;
    float NdotL = max(dot(normal, L), 0.0f);
    
    float3 result = radiance * g_Lights[index].strength * NdotL;
    return result;
}