#version 420 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

layout(std140) uniform Camera {
  mat4 viewMatrix;
  mat4 projectionMatrix;
};

layout(std140) uniform Model {
  mat4 modelMatrix;
  mat4 normalMatrix;
};

layout (std140) uniform Light {
  vec4 ambientColor;
  vec4 diffuseColor;
  vec4 specularColor;
  vec4 lightPosition; // world space.
};
 
smooth out vec3 viewDirection;
smooth out vec3 lightDirection;
flat out vec3 viewNormal;

void 
main(void) {
  // Per-face normal. Same for all vertices of the emitted triangle.
  vec3 normal = cross(
    gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz, 
    gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz);
  mat4 viewModelMatrix = viewMatrix*modelMatrix;


  vec4 lightViewPosition = viewMatrix*lightPosition;
  viewNormal = mat3(normalMatrix)*normal;

  int i;
  vec4 viewPosition;
  for (i = 0; i < 3; ++i) {
    viewPosition = viewModelMatrix*gl_in[i].gl_Position;
    lightDirection = lightViewPosition.xyz - viewPosition.xyz;
    viewDirection = -viewPosition.xyz;
    gl_Position = projectionMatrix*viewPosition; // Clip space.
    EmitVertex();	
  }
  EndPrimitive();
}
