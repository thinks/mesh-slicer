#version 420 core 

layout(std140) uniform Color {
  vec4 color;
};

out vec4 fragColor;

void main(void) {
  fragColor = color;
}