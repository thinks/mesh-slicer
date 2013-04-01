#version 420

uniform sampler2D texUnit;

smooth in vec2 uv;

out vec4 fragColor;

void main(void) {
  fragColor = texture(texUnit, uv);
}
