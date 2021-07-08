#include "Common.hlsli"

cbuffer cb1 : register(b1)
{
}

PS_INPUT main(VS_INPUT In)
{
    PS_INPUT Out;
    Out.Position = mul(ModelToProj, float4(In.Position, 1.0));
    Out.TexCoord = In.TexCoord;
    Out.Normal = In.Normal;
    return Out;
}