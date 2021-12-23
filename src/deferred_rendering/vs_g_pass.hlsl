cbuffer MatrixBuffer
{
    matrix World;
    matrix View;
    matrix Projection;
}

struct VSInput
{
    float4 pos : POSITION;
    float4 normal : NORMAL;
    float2 texCoord : TEXCOORD;
    float4 color : COLOR;
};

struct PSInput
{
    float4 pos : SV_Position;
    float4 normal : NORMAL;
    float4 color : COLOR;
};

PSInput VSGMain(VSInput input)
{
    input.pos.w = 1.0;
    input.normal.w = 0.0;
    
    PSInput result;
    result.pos = mul(input.pos, World);
    result.pos = mul(result.pos, View);
    result.pos = mul(result.pos, Projection);

    result.normal = mul(input.normal, World);
    result.color = input.color;
    
    return result;
}