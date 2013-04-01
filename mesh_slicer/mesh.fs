#version 420 core 

layout(std140) uniform Material {
  vec4 materialFrontAmbientColor;
  vec4 materialBackAmbientColor;
  vec4 materialFrontDiffuseColor;
  vec4 materialBackDiffuseColor;
  vec4 materialFrontSpecularColor;
  vec4 materialBackSpecularColor;
};

layout(std140) uniform Light {
  vec4 lightAmbientColor;
  vec4 lightDiffuseColor;
  vec4 lightSpecularColor;
  vec4 lightPosition;
};

smooth in vec3 viewDirection;
smooth in vec3 lightDirection;
flat in vec3 viewNormal;

out vec4 fragColor;

void main(void) {
  vec3 fragNormal = normalize(viewNormal);
  vec3 fragLightDirection = normalize(lightDirection);
  vec3 fragViewDirection = normalize(viewDirection);
  vec4 ambientColor;
  vec4 diffuseColor;
  vec4 specularColor;
  float shininess;
 
  if (gl_FrontFacing) {
    ambientColor = materialFrontAmbientColor;
    diffuseColor = materialFrontDiffuseColor;
    specularColor = materialFrontSpecularColor;
    shininess = 16.0;
  }
  else {
    ambientColor = materialBackAmbientColor;
    diffuseColor = materialBackDiffuseColor;
    specularColor = materialBackSpecularColor;
    shininess = 16.0;
    fragNormal = -fragNormal; // Flip normal!
  }


  float diffuse = max(dot(fragNormal, fragLightDirection), 0.0);

  vec3 reflectedNormal = reflect(-fragLightDirection, fragNormal);
  float specular = 
    pow(max(dot(reflectedNormal, fragViewDirection), 0.0), shininess);

  fragColor = 
    lightAmbientColor*ambientColor + 
    diffuse*(lightDiffuseColor*diffuseColor) + 
    specular*(lightSpecularColor*specularColor);
}