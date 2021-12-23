struct PSInput
{
    float4 pos : SV_Position;
    float4 normal : NORMAL;
    float4 color : COLOR;
};

struct PSOutput
{
    float4 normal : SV_Target0;
    float4 color : SV_Target1;
};

PSOutput PSGMain(PSInput input)
{
    PSOutput result;
    result.normal = input.normal;
    result.color = input.color;

    return result;
}