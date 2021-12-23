Texture2D normalTexture : register(t0);
Texture2D colorTexture : register(t1);

SamplerState deferredSampler : register(s0);

cbuffer LightBuffer
{
    float4 direction;
}

struct PSInput
{
    float4 pos : SV_Position;
    float2 texCoord : TEXCOORD;
};

float4 PSLMain(PSInput input) : SV_TARGET
{
    float4 normal = normalTexture.Sample(deferredSampler, input.texCoord);
    float4 color = colorTexture.Sample(deferredSampler, input.texCoord);
    
    float scalar = saturate(dot(normal, normalize(direction)));
    return color * scalar + (color * 0.1);
}