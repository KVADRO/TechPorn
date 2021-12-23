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
    float2 texCoord : TEXCOORD;
};

PSInput VSLMain(VSInput input)
{
    input.pos.w = 1.0f;
   
    PSInput result;
    result.pos = input.pos;
    result.texCoord = input.texCoord;
    
    return result;
}