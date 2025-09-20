struct SkinningData
{
    float4 position;
    float4 weights;
    float4 indices;
};

RWTexture2D<float4> gOutput : register(u0, space8);
StructuredBuffer<SkinningData> g_skinningData : register(t1, space8);

[numthreads(8, 8, 1)]
void Main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int width;
    int height;
    gOutput.GetDimensions(width, height);
    
    float2 uv = dispatchThreadID.xy / float2(width, height);
    gOutput[dispatchThreadID.xy] = float4(uv.xy, 0.0f, 1.0f);
}