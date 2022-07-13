
struct VS_INPUT
{
    float4 pos : POSITION;
    float4 color : COLOR;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

cbuffer ConstantBuffer : register(b0)
{
    float4x4 wvpMat;
};

cbuffer Camera : register(b1)
{
    float4x4 camvpMat;
}

VS_OUTPUT VSMain(VS_INPUT input)
{
    VS_OUTPUT result;

    result.pos = mul(mul(input.pos, wvpMat), camvpMat);
    result.color = input.color;

    return result;
}

