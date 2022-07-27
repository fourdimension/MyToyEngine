
Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

float4 PSMain(VS_OUTPUT input) : SV_TARGET
{
    return g_texture.Sample(g_sampler, input.uv);
}
