struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 normal : NORMAL;
    float4 color : COLOR;
};

float4 ColorPixelShader(PixelInputType input) : SV_TARGET
{
    float scalar = saturate(dot(input.normal, normalize(float4(-1.0, 1.0, -1.0, 0.0))));
    return input.color * scalar + (input.color * 0.1);
}
