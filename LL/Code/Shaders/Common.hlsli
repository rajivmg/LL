// Common data and methods

struct VS_INPUT
{
	float3 Position : VS_POSITION;
	float2 TexCoord : VS_TEXCOORD;
	float3 Normal	: VS_NORMAL;
};

struct PS_INPUT
{
	float4 Position : SV_POSITION;
	float2 TexCoord : PS_TEXCOORD;
	float3 Normal	: PS_NORMAL;
};

cbuffer cb0 : register(b0)
{
	float4x4 ModelToProj;
};