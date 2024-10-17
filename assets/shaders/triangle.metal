#include <metal_stdlib>
using namespace metal;

struct VertexData {
    float4 position;
    float2 texCoords;
};

struct VertexOut {
    // The [[position]] attribute of this member indicates that this value
    // is the clip space position of the vertex when this structure is
    // returned from the vertex function.
    float4 position [[position]];

    // Since this member does not have a special attribute, the rasterizer
    // interpolates its value with the values of the other triangle vertices
    // and then passes the interpolated value to the fragment shader for each
    // fragment in the triangle.
    float2 textureCoordinate;
};

vertex VertexOut
vertexShader(uint vertexID [[vertex_id]],
             constant VertexData* vertexData)
{
    VertexOut out;
    out.position = vertexData[vertexID].position;
    out.textureCoordinate = vertexData[vertexID].texCoords;
    return out;
}

fragment float4 fragmentShader(VertexOut in [[stage_in]],
                               texture2d<float> colorTexture [[texture(0)]]) {
    
    constexpr sampler textureSampler (mag_filter::linear,
                                      min_filter::linear);
    // Sample the texture to obtain a color
    const float4 colorSample = colorTexture.sample(textureSampler, in.textureCoordinate);
    return colorSample;
}