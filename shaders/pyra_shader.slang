
static const float2 verticies[3] = { float2(1,1), float2(0,0), float2(1,0) };

struct VertexShaderOutput
{
    float4 vertexCoords : SV_Position; 
};

[shader("vertex")]
VertexShaderOutput VertexShaderMain(uint32_t vertexID : SV_VertexID)
{
    VertexShaderOutput vertex;
    vertex.vertexCoords = float4(verticies[vertexID], 0.0, 1.0f);
    return vertex;
};

[shader("fragment")]
float4 FragmentShaderMain(VertexShaderOutput vertIn) 
{
    return float4(1.0, 0.0, 0.0, 1.0);
};