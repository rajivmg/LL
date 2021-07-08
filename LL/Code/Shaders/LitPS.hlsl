#include "Common.hlsli"

SamplerState Sampler0 : register(s0);
Texture2D    AlbedoTexture : register(t0);

float4 main(PS_INPUT In) : SV_TARGET
{
    return float4(AlbedoTexture.Sample(Sampler0, In.TexCoord));
    //return float4(abs(normalize(In.Normal)), 1.0f);
}