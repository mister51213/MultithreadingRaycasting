Texture2D scrTex : register(t0);
Texture2D wallTex : register(t1);
SamplerState scrSmp : register(s0);
SamplerState wallSmp : register(s1);

float4 FramebufferPS(float4 position : SV_POSITION, float2 texCoord : TEXCOORD0) : SV_TARGET
{
	//return wallTex.Sample(scrSmp, texCoord);
	float4 smp = scrTex.Sample(scrSmp, texCoord);
	//float c = 1.0f / sqrt(smp.z);
	//return float4(c, c, c, 1);

	float4 clr = wallTex.Sample(wallSmp, smp.xy) / sqrt(smp.z);
	return clr;
}