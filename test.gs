#version 150
//#extension GL_EXT_gpu_shader4 : enable
//#extension GL_EXT_geometry_shader4 : enable

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform mat4  uniModelXf;
uniform mat3  uniNmlXf;
uniform mat4  uniViewXf;
uniform mat4  uniProjXf;
uniform ivec4 uniViewport;
uniform float uniDepthNear;
uniform float uniDepthFar;
 
smooth		  out vec4 exEyeVtx;
flat		  out vec3 exEyeNml;
noperspective out vec3 exEdgeDist;
noperspective out vec3 exVtxDist;

void 
main(void) 
{
    // Per-face normal. Same for all vertices.
    exEyeNml = uniNmlXf*normalize(cross(
        gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz, 
        gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz)
	);

    // World space.
    vec4 wsx[3] = vec4[3](
        uniModelXf*gl_in[0].gl_Position,
        uniModelXf*gl_in[1].gl_Position,
        uniModelXf*gl_in[2].gl_Position
	);

    // Eye space.
    vec4 esx[3] = vec4[3](
        uniViewXf*wsx[0],
		uniViewXf*wsx[1],
		uniViewXf*wsx[2]
	);

	// Clip space.
    vec4 csx[3] = vec4[3](
		uniProjXf*esx[0],
		uniProjXf*esx[1],
		uniProjXf*esx[2]
	);

	// Normalized device coordinates.
	vec3 ndc[3] = vec3[3](
		csx[0].xyz/csx[0].w,
		csx[1].xyz/csx[1].w,
		csx[2].xyz/csx[2].w
	);

	// Viewport space.
    vec3 vpx[3] = vec3[3](
		vec3(uniViewport[0] + 0.5*uniViewport[2]*(ndc[0].x),
			 uniViewport[1] + 0.5*uniViewport[3]*(ndc[0].y),
			 uniDepthNear	+ 0.5*uniDepthFar*(
				ndc[0].z + uniDepthFar - uniDepthNear)),
		vec3(uniViewport[0] + 0.5*uniViewport[2]*(ndc[1].x),
			 uniViewport[1] + 0.5*uniViewport[3]*(ndc[1].y),
			 uniDepthNear   + 0.5*uniDepthFar*(
				ndc[1].z + uniDepthFar - uniDepthNear)),
		vec3(uniViewport[0] + 0.5*uniViewport[2]*(ndc[2].x),
			 uniViewport[1] + 0.5*uniViewport[3]*(ndc[2].y),
			 uniDepthNear   + 0.5*uniDepthFar*(
				ndc[2].z + uniDepthFar - uniDepthNear)));

	// TODO: Could be optimized for 2D!
	float h[3] = float[3](
		length(cross(vpx[1] - vpx[0], vpx[0] - vpx[2]))/length(vpx[1] - vpx[0]),
		length(cross(vpx[2] - vpx[1], vpx[1] - vpx[0]))/length(vpx[2] - vpx[1]),
		length(cross(vpx[2] - vpx[0], vpx[2] - vpx[1]))/length(vpx[0] - vpx[2])
	);

	float d[3] = float[3](
		length(vpx[1].xy - vpx[0].xy),
		length(vpx[2].xy - vpx[1].xy),
		length(vpx[2].xy - vpx[0].xy));

    exEdgeDist  = vec3(0.0, h[1], 0.0);
	exVtxDist   = vec3(0.0, d[0], d[2]);
    exEyeVtx    = esx[0]; // Eye space.
    gl_Position = csx[0]; // Clip space.
    EmitVertex();

    exEdgeDist  = vec3(0.0, 0.0, h[2]);
	exVtxDist   = vec3(d[0], 0.0, d[1]);
    exEyeVtx    = esx[1]; // Eye space.
    gl_Position = csx[1]; // Clip space.
    EmitVertex();

    exEdgeDist  = vec3(h[0], 0.0, 0.0);
	exVtxDist   = vec3(d[2], d[1], 0.0);
    exEyeVtx    = esx[2]; // Eye space.
    gl_Position = csx[2]; // Clip space.
    EmitVertex();

    EndPrimitive();
}
