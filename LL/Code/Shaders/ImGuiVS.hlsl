#include "ImGuiCommon.hlsli"

cbuffer VSConstantBuffer : register(b0)
{
    float4x4 MVP;
};

struct VS_INPUT
{
    float2 Pos : POSITION;
    float4 Col : COLOR0;
    float2 UV  : TEXCOORD0;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float4 Col : COLOR0;
    float2 UV  : TEXCOORD0;
};

[RootSignature(ImGui_RootSig)]
PS_INPUT main(VS_INPUT Input)
{
    PS_INPUT Output;
    Output.Pos = mul(MVP, float4(Input.Pos.xy, 0.0f, 1.0f));
    Output.Col = Input.Col;
    Output.UV = Input.UV;
    return Output;
}