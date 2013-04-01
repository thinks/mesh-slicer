#version 420

layout(std140) uniform Camera {
  mat4 viewMatrix;
  mat4 projectionMatrix;
};

layout(std140) uniform Model {
  mat4 modelMatrix;
  mat4 normalMatrix;
};

in vec3 position; // object space vertex coordinates.
in vec2 texture;

out vec2 uv;

void main(void) {
  uv = texture;
  gl_Position = projectionMatrix*viewMatrix*modelMatrix*vec4(position, 1.0);
}
