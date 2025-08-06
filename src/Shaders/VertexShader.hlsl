////////////////////////////////
// File: VertexShader.hlsl
////////////////////////////////

//-------------------------------------------------------------------------------------
// Constant Buffer
//-------------------------------------------------------------------------------------
cbuffer VS_CONSTANTBUFFER : register(b0)
{
    float4x4 m_WVPMatrix : packoffset(c0);
};

//--------------------------------------------------------------------------------------
// Input structure
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float3 position : POSITION;
    float4 color : COLOR;
    float2 texcoord : TEXCOORD;
};

//--------------------------------------------------------------------------------------
// Output structure
//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR0;
    float2 texcoord : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT vs_main(VS_INPUT input)
{
    VS_OUTPUT output;

    output.position = float4(input.position, 1.0f);
    output.position = mul(output.position, m_WVPMatrix);
    
    output.color = input.color;
    output.texcoord = input.texcoord;

    return output;
}