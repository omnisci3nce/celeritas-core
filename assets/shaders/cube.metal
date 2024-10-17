#include <metal_stdlib>
using namespace metal;

struct VertexData {
    float4 position;
    float4 normal;
    float2 texCoords;
};

struct VertexOut {
    float4 position [[position]];
    float2 textureCoordinate;
};

struct TransformationData {
    float4x4 modelMatrix;
    float4x4 viewMatrix;
    float4x4 perspectiveMatrix;
};

vertex VertexOut cubeVertexShader(uint vertexID [[vertex_id]],
             constant VertexData* vertexData,
             constant TransformationData* transformationData)
{
    VertexOut out;
    out.position = transformationData->perspectiveMatrix * transformationData->viewMatrix * transformationData->modelMatrix * vertexData[vertexID].position;
    out.textureCoordinate = vertexData[vertexID].texCoords;
    return out;
}