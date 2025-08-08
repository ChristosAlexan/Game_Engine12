cbuffer CB_PS_Lights : register(b4)
{
    uint totalLights;
    float3 padding3;
};


struct GPULight
{
    float3 position;
    float strength;

    float3 color;
    float radius;
    
    float4 direction;
    
    uint lighType;
    float cutoff;
    float2 padding;
};