#include "General_PS.hlsli"
#include "Math_Helpers.hlsli"

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};
float3 fresnelSchlick(float cosTheta, float3 F0);
float3 fresnelSchlickRoughness(float cosTheta, float3 F0, float roughness);
float DistributionGGX(float3 N, float3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(float3 N, float3 V, float3 L, float roughness);

float3 PointLight(float3 albedo, float3 normal, float metallic, float roughness, float3 worldPos, float3 F0, uint index);
float3 SpotLight(float3 albedo, float3 normal, float metallic, float roughness, float3 worldPos, float3 F0, uint index);
float3 BlinnPhongPointLight(float3 albedo, float3 normal, float3 worldPos, uint index);

Texture2D albedoTexture : register(t0, space0);
Texture2D normalTexture : register(t1, space0);
Texture2D metalRoughnessMaskTexture : register(t2, space0);
Texture2D worldPosDepthTexture : register(t3, space0);

// space2: Lights
StructuredBuffer<GPULight> g_Lights : register(t0, space2);
SamplerState gSampler : register(s0);

float4 Main(PSInput input) : SV_TARGET
{
    float4 albedo = albedoTexture.Sample(gSampler, input.uv).rgba;
    float mask = metalRoughnessMaskTexture.Sample(gSampler, input.uv).b;

    if(mask == 0.0f)
        return float4(albedo.rgb, 1.0f);
    
    float3 normal = normalize(normalTexture.Sample(gSampler, input.uv)).xyz;
    float3 worldPos = worldPosDepthTexture.Sample(gSampler, input.uv).xyz;
    float metalness = metalRoughnessMaskTexture.Sample(gSampler, input.uv).g;
    float roughness = metalRoughnessMaskTexture.Sample(gSampler, input.uv).r;
    float depth = worldPosDepthTexture.Sample(gSampler, input.uv).w; // w is depth
   
    float3 V = normalize(cameraPos.xyz - worldPos.xyz);
    float3 ambient = float3(0.1f, 0.1f, 0.1f);
    normal = normalize(normal.xyz);
    float3 F0 = float3(0.08f, 0.08f, 0.08f);
    F0 = lerp(F0, albedo.rgb, metalness);
    float3 Lo = float3(0, 0, 0);
    
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
                    Lo += SpotLight(albedo.rgb, normal, metalness, roughness, worldPos, F0, i);
                   break; 
            }
            case 2: // Point
            {
                    Lo += PointLight(albedo.rgb, normal, metalness, roughness, worldPos, F0, i);
                    break;
            }
        }
        
    }
    float3 F = fresnelSchlickRoughness(max(dot(normal, V), 0.0f), F0, roughness);
    float3 kS = F;
    float3 kD = 1.0f - kS;
    
    return float4(Lo + albedo.rgb * ambient, 1.0f);
}

float3 fresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0f - F0) * pow(clamp(1.0f - cosTheta, 0.0, 1.0), 5.0f);
}

float3 fresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
    return F0 + (max(float3(1.0f - roughness, 1.0f - roughness, 1.0f - roughness), F0) - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
}

float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0f);
    float NdotH2 = NdotH * NdotH;
    
    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    
    denom = PI * denom * denom;
    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0f);
    float k = (r * r) / 8.0f;
    
    float nom = NdotV;
    float denom = NdotV * (1.0f - k) + k;
    
    return nom / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}

float3 PointLight(float3 albedo, float3 normal, float metallic, float roughness, float3 worldPos, float3 F0, uint index)
{
    float3 V = normalize(cameraPos.xyz - worldPos);
    float3 L = normalize(g_Lights[index].position - worldPos);
    float3 H = normalize(V + L);

    float distance = length(g_Lights[index].position - worldPos);
    float attenuation = 1.0f / (distance * distance);
    float3 radiance = g_Lights[index].color * attenuation * g_Lights[index].strength;

    float NDF = DistributionGGX(normal, H, roughness);
    float G = GeometrySmith(normal, V, L, roughness);
    float3 F = fresnelSchlick(max(dot(H, V), 0.0f), F0);

    float3 nominator = NDF * G * F;
    float denominator = 4.0f * max(dot(normal, V), 0.0f) * max(dot(normal, L), 0.0f) + 0.001;
    float3 specular = nominator / denominator;

    float3 kS = F;
    float3 kD = float3(1.0f, 1.0f, 1.0f) - kS;
    kD *= 1.0f - metallic;

    float NdotL = max(dot(normal, L), 0.0f);
    
    return (kD * albedo / PI + specular) * radiance * NdotL;

}

float3 SpotLight(float3 albedo, float3 normal, float metallic, float roughness, float3 worldPos,float3 F0, uint index)
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
    float3 radiance = g_Lights[index].color.xyz * attenuation * g_Lights[index].strength;
    
    float NDF = DistributionGGX(normal, H, roughness);
    float G = GeometrySmith(normal, V, L, roughness);
    float3 F = fresnelSchlick(max(dot(H, V), 0.0f), F0);
        
    float3 nominator = NDF * G * F;
    float denominator = 4 * max(dot(normal, V), 0.0f) * max(dot(normal, L), 0.0f) + 0.001;
    float3 specular = (nominator / denominator);
        
    float3 kS = F;
    float3 kD = float3(1.0f, 1.0f, 1.0f) - kS;
    kD *= 1.0f - metallic;
    
    float NdotL = max(dot(normal, L), 0.0f);
    
    return (kD * albedo / PI + specular) * radiance * NdotL;
}

float3 BlinnPhongPointLight(float3 albedo, float3 normal, float3 worldPos, uint index)
{
    float3 L = normalize(g_Lights[index].position - worldPos);
    float3 V = normalize(cameraPos - worldPos);
    float3 H = normalize(L + V);

    float NdotL = max(dot(normal, L), 0.0f);
    float NdotH = max(dot(normal, H), 0.0f);
    
    float specularPower = 32.0f;
    float specularStrength = 0.5f;

    float3 diffuse = albedo * NdotL;
    float3 specular = pow(NdotH, specularPower) * specularStrength;

    return (diffuse + specular) * g_Lights[index].color * g_Lights[index].strength;
}