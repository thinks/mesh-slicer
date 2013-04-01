#version 420 core 

layout(std140) uniform Camera {
  mat4 viewMatrix;
  mat4 projectionMatrix;
};

in vec3 position; // NB: eye space vertex coordinates.

void 
main(void) {
  gl_Position = projectionMatrix*vec4(position, 1.0);
}
