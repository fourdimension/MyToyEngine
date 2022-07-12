
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

cbuffer RootConstants : register(b1)
{
    float x; // Half the width of the triangles.
    float y; // The z offset for the triangle vertices.
    float z; // The culling plane offset in homogenous space.
    float w; // The number of commands to be processed.
};

VS_OUTPUT VSMain(VS_INPUT input)
{
    VS_OUTPUT result;

    result.pos = mul(input.pos, wvpMat);
    result.pos *= x;
    //result.pos = input.pos;
    result.color = input.color;

    return result;
}

