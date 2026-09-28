

struct VertexShaderInput
{
    float2 position; 
    float4 color;
};

struct VertexShaderOutput
{
    float4 position : SV_Position;
    float4 color;
};

[shader("vertex")]
VertexShaderOutput vsEntry(VertexShaderInput vsInput)
{
    VertexShaderOutput vsOutput;
    vsOutput.position = float4(vsInput.position, 0.0f, 1.f);
    vsOutput.color = vsInput.color;
    return vsOutput;
}

[shader("fragment")]
float4 fsEntry(VertexShaderOutput fsInput)
{
    return fsInput.color;
}