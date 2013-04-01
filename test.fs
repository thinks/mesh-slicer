#version 150

smooth        in vec4 exEyeVtx;
flat          in vec3 exEyeNml;
noperspective in vec3 exEdgeDist;
noperspective in vec3 exVtxDist;

out vec4 outFragColor;

void main(void)
{
    const float edgeWidth   = 1.0;
    const vec4  edgeColor   = vec4(1.0, 1.0, 1.0, 1.0);
    const float pointRadius = 15.0;
    const vec4  pointColor  = vec4(0.0, 1.0, 0.0, 1.0);

    const vec4  surfColor  = vec4(1.0, 0.0, 0.0, 1.0);
    const vec4  lightColor = vec4(1.0, 1.0, 1.0, 1.0);
    const vec3  lightPos   = vec3(0.0, 0.0, 0.0); // Eye space.


    vec4 surfDiffuse = clamp(
        (surfColor*lightColor)*
            max(dot(exEyeNml, normalize(lightPos - exEyeVtx.xyz)), 0.0), 
                0.0, 1.0);

    vec4 color = surfDiffuse;

    float minEdgeDist = clamp(
        min(min(exEdgeDist[0], exEdgeDist[1]), exEdgeDist[2]) - 0.5*edgeWidth, 
        0.0, 2.0);
    float edgeAlpha = exp2(-2.0*minEdgeDist*minEdgeDist);
    color = mix(color, edgeColor, edgeAlpha);

    float minVtxDist = clamp(
        min(min(exVtxDist[0], exVtxDist[1]), exVtxDist[2]) - pointRadius,
        0.0, 2.0);
    float pointAlpha = exp2(-2.0*minVtxDist*minVtxDist);
    color = mix(color, pointColor, pointAlpha);

    outFragColor = color;
}
