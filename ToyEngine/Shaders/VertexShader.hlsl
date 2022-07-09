
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

VS_OUTPUT VSMain(VS_INPUT input)
{
    VS_OUTPUT result;

    //result.pos = mul(input.pos, wvpMat);
    result.pos = input.pos;
    result.color = input.color;

    return result;
}

