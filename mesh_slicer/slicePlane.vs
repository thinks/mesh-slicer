#version 420 core 

layout(std140) uniform Camera {
  mat4 viewMatrix;
  mat4 projectionMatrix;
};

layout(std140) uniform Model {
  mat4 modelMatrix;
  mat4 normalMatrix;
};

in vec3 position; // object space vertex coordinates.

void main(void) {
  gl_Position = projectionMatrix*viewMatrix*modelMatrix*vec4(position, 1.0);
}
