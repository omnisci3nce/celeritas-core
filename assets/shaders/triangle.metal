#include <metal_stdlib>

using namespace metal;

struct VertexIn {
  float2 position;
  float3 color;
};

struct VertexOut {
  float4 computedPosition [[position]];
  float3 fragColor;
};

// Vertex shader
vertex VertexOut basic_vertex(
  const device VertexIn* vertex_array [[ buffer(0) ]],
  unsigned int vid [[ vertex_id ]]
  ) {
  VertexIn v = vertex_array[vid];

  VertexOut outVertex = VertexOut();
  outVertex.computedPosition = float4(v.position.xy, 0.0, 1.0);
  outVertex.fragColor = v.color;
  return outVertex;
}

// Fragment shader
fragment float4 basic_fragment(
  VertexOut interpolated [[stage_in]]
) { 
  return float4(interpolated.fragColor, 1.0);
}