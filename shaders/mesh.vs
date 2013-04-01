#version 420 core 

in vec3 position; // object space vertex coordinates.

void main(void) {
  gl_Position = vec4(position, 1.0);
}
