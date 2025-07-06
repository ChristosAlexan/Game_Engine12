
struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};


float4 Main(PSInput input) : SV_TARGET
{
	return float4(0.4f, 0.4f, 0.4f, 1.0f);
}